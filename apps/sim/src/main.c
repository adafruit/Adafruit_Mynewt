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
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include <assert.h>
#include <string.h>
#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif
#include "sysinit/sysinit.h"
#include "console/console.h"
#include "shell/shell.h"

#include <sensor/sensor.h>
#include <sim/sim_accel.h>

struct sim_accel sim_accel_sensor;

static int
accel_init(struct os_dev *dev, void *arg)
{
    struct sim_accel_cfg cfg;
    int rc;

    rc = sim_accel_init(dev, arg);
    if (rc != 0) {
        goto err;
    }

    cfg.sac_nr_samples = 10;
    cfg.sac_nr_axises = 3;
    /* read once per sec.  API should take this value in ms. */
    cfg.sac_sample_itvl = OS_TICKS_PER_SEC;

    rc = sim_accel_config((struct sim_accel *) dev, &cfg);
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
#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    os_dev_create((struct os_dev *) &sim_accel_sensor, "accel",
            OS_DEV_INIT_KERNEL, OS_DEV_INIT_PRIMARY, accel_init, NULL);

    while (1) {
      os_eventq_run(os_eventq_dflt_get());
    }

    return 0;
}
