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

#include "adafruit/tsl2561.h"

/* Init all tasks */
int init_tasks(void);

/* Task 1 */
#define BLINKY_TASK_PRIO     (10)
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)
struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

/* System event queue task handler */
#define SYSEVQ_PRIO          (1)
#define SYSEVQ_STACK_SIZE    OS_STACK_ALIGN(512)
static struct os_task task_sysevq;

/* Event queue for events handled by the system (shell, etc.) */
static struct os_eventq sys_evq;

// Command handler prototype declaration
static int shell_i2cscan_cmd(int argc, char **argv);

// Shell command struct
static struct shell_cmd shell_i2cscan_cmd_struct = {
    .sc_cmd = "i2cscan",
    .sc_cmd_func = shell_i2cscan_cmd
};

/**
 * This task serves as a container for the shell and newtmgr packages.  These
 * packages enqueue timer events when they need this task to do work.
 */
static void
sysevq_handler(void *arg)
{
    while (1) {
        os_eventq_run(&sys_evq);
    }
}

// i2cscan command handler
static int
shell_i2cscan_cmd(int argc, char **argv)
{
    uint8_t addr;
    int32_t timeout = OS_TICKS_PER_SEC / 10;
    uint8_t dev_count = 0;

    console_printf("Scanning I2C bus 0\n"
                   "     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n"
                   "00:          ");

    /* Scan all valid I2C addresses (0x03..0x77) */
    for (addr = 0x03; addr < 0x78; addr++) {
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

/**
 * init_tasks
 *
 * Called by main.c after os_init(). This function performs initializations
 * that are required before tasks are running.
 *
 * @return int 0 success; error otherwise.
 */
int
init_tasks(void)
{
    os_stack_t *pstack;

    /* Initialize eventq and designate it as the default.  Packages that need
     * to schedule work items will piggyback on this eventq.  Example packages
     * which do this are sys/shell and mgmt/newtmgr.
     */
    os_eventq_init(&sys_evq);

    pstack = malloc(sizeof(os_stack_t)*SYSEVQ_STACK_SIZE);
    assert(pstack);

    os_task_init(&task_sysevq, "sysevq", sysevq_handler, NULL,
            SYSEVQ_PRIO, OS_WAIT_FOREVER, pstack, SYSEVQ_STACK_SIZE);

    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
            BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

    /* Set the default eventq for packages that lack a dedicated task. */
    os_eventq_dflt_set(&sys_evq);

    return 0;
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
    int rc;

    /* Initialize OS */
    sysinit();

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

#if MYNEWT_VAL(SHELL_TASK)
    shell_cmd_register(&shell_i2cscan_cmd_struct);
#endif

    rc = init_tasks();

    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return rc;
}
