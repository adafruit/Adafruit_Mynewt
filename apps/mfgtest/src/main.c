/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_i2c.h"
#include <assert.h>
#include <string.h>
#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif

#include "console/console.h"
#include "shell/shell.h"

#include "tsl2561/tsl2561.h"

#include "sensor/sensor.h"
#include "lsm303dlhc/lsm303dlhc.h"
#include "orientation/orientation.h"

struct lsm303dlhc lsm303dlhc_sensor;

/* Task 1 */
#define BLINKY_TASK_PRIO     (10)
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)
struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

/* Command handler prototype declaration */
static int shell_i2cscan_cmd(int argc, char **argv);
static int shell_orientation_cmd(int argc, char **argv);

/* i2cscan shell command struct */
static struct shell_cmd shell_i2cscan_cmd_struct = {
    .sc_cmd = "i2cscan",
    .sc_cmd_func = shell_i2cscan_cmd
};

/* orientation command shell struct */
static struct shell_cmd shell_orientation_cmd_struct = {
    .sc_cmd = "orientation",
    .sc_cmd_func = shell_orientation_cmd
};

// i2cscan command handler
static int
shell_i2cscan_cmd(int argc, char **argv)
{
    uint8_t addr;
    int32_t timeout = OS_TICKS_PER_SEC / 10;
    uint8_t dev_count = 0;

    console_printf("Scanning I2C bus 0\n"
                   "     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n"
                   "00:                         ");

    /* Scan all valid I2C addresses (0x08..0x77) */
    for (addr = 0x08; addr < 0x78; addr++) {
        int rc = hal_i2c_master_probe(0, addr, timeout);
        /* Print addr header every 16 bytes */
        if (!(addr % 16)) {
          console_printf("\n%02x: ", addr);
        }
        /* Display the addr if a response was received */
        if (!rc) {
            console_printf("%02x ", addr);
            dev_count++;
        } else {
            console_printf("-- ");
        }
    }
    console_printf("\nFound %u devices on I2C bus 0\n", dev_count);

    return 0;
}

static int
orientation_listener(struct sensor *sensor, void *arg, void *data, sensor_type_t type)
{
    (void) type;

    int rc;
    struct sensor_accel_data *sad;
    struct or_orientation_vec orv;

    memset(&orv, 0, sizeof(orv));

    sad = (struct sensor_accel_data *) data;

    rc = or_from_accel(sad, &orv);

    console_printf("r = %i, ", (int)orv.roll);
    console_printf("p = %i ", (int)orv.pitch);
    console_printf("\n");

    return rc;
}

/* orientation command handler */
static int
shell_orientation_cmd(int argc, char **argv)
{
    struct sensor *sensor;
    struct sensor_listener listener;
    int rc;

    /* Look up sensor by name */
    sensor = sensor_mgr_find_next_bydevname("lsm303dlhc", NULL);
    if (!sensor) {
        console_printf("Sensor %s not found!\n", "lsm303dlhc");
    }

    listener.sl_sensor_type = SENSOR_TYPE_ACCELEROMETER;
    listener.sl_func = orientation_listener;
    listener.sl_arg = NULL;
    //listener.sl_arg = &ctx;

    rc = sensor_register_listener(sensor, &listener);
    if (rc != 0) {
        goto err;
    }

    /* Get a fresh data sample (will be returned in the listener) */
    rc = sensor_read(sensor, SENSOR_TYPE_ACCELEROMETER,
        NULL, NULL, OS_TIMEOUT_NEVER);

    sensor_unregister_listener(sensor, &listener);

err:
    return 0;
}

void
blinky_task_handler(void *arg)
{
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC*1);

        /* Toggle the LED */
        hal_gpio_toggle(LED_BLINK_PIN);
    }
}

static int
lsm303dlhc_drvr_init(struct os_dev *dev, void *arg)
{
    struct lsm303dlhc_cfg cfg;
    int rc;

    rc = lsm303dlhc_init(dev, arg);
    if (rc != 0) {
        goto err;
    }

    cfg.accel_range = LSM303DLHC_ACCEL_RANGE_2;
    cfg.accel_rate = LSM303DLHC_ACCEL_RATE_400;
    cfg.mag_gain = LSM303DLHC_MAG_GAIN_1_3;
    cfg.mag_rate = LSM303DLHC_MAG_RATE_220;

    rc = lsm303dlhc_config((struct lsm303dlhc *) dev, &cfg);
    if (rc != 0) {
        goto err;
    }

    return (0);
err:
    return (rc);

}

/**
 * main
 *
 * The main function for the project. This function initializes the os, calls
 * init_tasks to initialize tasks (and possibly other objects), then starts the
 * OS. We should not return from os start.
 *
 * @return int NOTE: this function should never return!
 */
int
main(int argc, char **argv)
{
    /* Initialize OS */
    sysinit();

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

#if MYNEWT_VAL(SHELL_TASK)
    shell_cmd_register(&shell_i2cscan_cmd_struct);
    shell_cmd_register(&shell_orientation_cmd_struct);
#endif

    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
            BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

    os_dev_create((struct os_dev *) &lsm303dlhc_sensor, "lsm303dlhc",
            OS_DEV_INIT_KERNEL, OS_DEV_INIT_PRIMARY, lsm303dlhc_drvr_init, NULL);

    while (1) {
      os_eventq_run(os_eventq_dflt_get());
    }

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return 0;
}
