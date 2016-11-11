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

/* Init all tasks */
int init_tasks(void);

/* Task 1 */
#define BLINKY_TASK_PRIO     (1)
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

void
blinky_task_handler(void *arg)
{
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC * 1);

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
    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
            BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

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

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    os_init();

    rc = init_tasks();
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return rc;
}
