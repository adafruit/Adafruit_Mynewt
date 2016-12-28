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

#define IRQ_PIN             (20)
#define EVENT_TASK_PRIO     (1)
#define EVENT_STACK_SIZE    OS_STACK_ALIGN(128)

os_stack_t event_stack[EVENT_STACK_SIZE];

/* Advance function prototypes */
static int init_tasks(void);
static void gpio_task_irq_deferred_handler(struct os_event *);

/* Default event queue */
static struct os_eventq irq_evq;

/* Callout example */
static struct os_callout blinky_timer;

/* Event queue example */
struct os_task event_task;
static struct os_event gpio_irq_handle_event = {
    .ev_cb = gpio_task_irq_deferred_handler,
};

static void
blinky_timer_cb(struct os_event *ev)
{
    int32_t timeout = OS_TICKS_PER_SEC;

    /* Toggle the blinky LED */
    hal_gpio_toggle(LED_BLINK_PIN);

    /* Reset the timeout so that it fires again */
    os_callout_reset(&blinky_timer, timeout);
}

/**
 * This function will be called when the gpio_irq_handle_event is pulled
 * from the message queue.
 */
static void
gpio_task_irq_deferred_handler(struct os_event *ev)
{
    hal_gpio_toggle(LED_2);
}

/**
 * This task serves as a container for the shell and newtmgr packages.  These
 * packages enqueue timer events when they need this task to do work.
 */
static void
event_task_handler(void *arg)
{
    while (1) {
        os_eventq_run(&irq_evq);
    }
}

/**
 * This function handles the HW GPIO interrupt and registers an event in
 * the event queue to defer taking action here in the ISR context.
 */
static void
gpio_irq_handler(void *arg)
{
    /* Add item to event queue for processing later, etc. ... */
    os_eventq_put(&irq_evq, &gpio_irq_handle_event);
}

/**
 * init_tasks
 *
 * Called by main.c after os_init(). This function performs initializations
 * that are required before tasks are running.
 *
 * @return int 0 success; error otherwise.
 */
static int
init_tasks(void)
{
    /* Init IRQ pin: falling edge, pullup enabled */
    hal_gpio_irq_init(IRQ_PIN,
                      gpio_irq_handler,
                      NULL,
                      HAL_GPIO_TRIG_FALLING,
                      HAL_GPIO_PULL_UP);

    /* Enable the IRQ */
    hal_gpio_irq_enable(IRQ_PIN);

    /* Setup the LED pins */
    hal_gpio_init_out(LED_BLINK_PIN, 1);
    hal_gpio_init_out(LED_2, 0);

    os_task_init(&event_task, "irq", event_task_handler, NULL,
            EVENT_TASK_PRIO, OS_WAIT_FOREVER, event_stack, EVENT_STACK_SIZE);

    os_eventq_init(&irq_evq);
    os_eventq_dflt_set(&irq_evq);

    /* Create a callout (timer).  This callout is used to generate blinky */
    os_callout_init(&blinky_timer, &irq_evq, blinky_timer_cb, NULL);
    os_callout_reset(&blinky_timer, OS_TICKS_PER_SEC);

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
