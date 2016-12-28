# Task Delays

There are various ways that a task can be interrupted, such as delaying
execution for a specific amount of time, waiting for an event on an event
queue, waiting for a semaphore, etc.

Delaying your tasks is important, because as long as your task is active, no
tasks of lower priority will execute. As such, it's important to manage your
tasks as efficiently as possible to ensure that clock cycles are available for
other tasks in the system.

### `os_time_delay`

The `os_time_delay` function is the easiest way to cause a delay in execution
in your task. Simply specify a specific number of ticks, and the scheduler will
mark this task as inactive for the indicated delay.

> Please note that `os_time_delay` uses system ticks, which may vary from one
  system to the next, so any delays should be based on the `OS_TICKS_PER_SECOND`
  macro to remain portable.

```
static void
my_task_func(void *arg)
{
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC * 1);

        /* Toggle the LED */
        hal_gpio_toggle(LED_BLINK_PIN);
    }
}
```

### Semaphores

ToDo!

### Events

See [03_EventQueues.md](03_EventQueues.md) for details on using events and
event queues to handle asynchronous events inside a task handler.
