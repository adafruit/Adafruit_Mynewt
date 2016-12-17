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

/* Init all tasks */
int init_tasks(void);

/* System event queue task handler */
#define SYSEVQ_PRIO (10)
#define SYSEVQ_STACK_SIZE    OS_STACK_ALIGN(512)
static struct os_task task_sysevq;

/* Event queue for events handled by the system (shell, etc.) */
static struct os_eventq sys_evq;

struct sim_accel sim_accel_sensor;

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

    /* Set the default eventq for packages that lack a dedicated task. */
    os_eventq_dflt_set(&sys_evq);

    return 0;
}

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
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();

    rc = init_tasks();

    sensor_pkg_init();

    os_dev_create((struct os_dev *) &sim_accel_sensor, "accel",
            OS_DEV_INIT_KERNEL, OS_DEV_INIT_PRIMARY, accel_init, NULL);

    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return rc;
}
