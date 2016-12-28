# Tasks

A task (`os_task`) in Mynewt is made up of a task handler function, a task
'stack' which provide the block of memory that will be used when executing the
task, and a priority level.

Since Mynewt is a multi-tasking environment, tasks are also assigned a priority
level, and at any given time the highest priority task will run. When the
highest priority task stops (waiting for an event, or when delayed in code) the
next highest priority task will fire, and so on until the scheduler gets down
to the lowest priority task, usually called 'idle' (which will be set by the
kernel when the OS starts up).

### Declaring a task, priority and stack size

In order to declare a task, you need to set it's priority, stack size, and the
name of the task handler that will be run when the task is active.

The task's **priority** can be from 1..255, where the higher the number the
lower the priority.

The **stack size** is in units of `os_stack_t`, which is usually a 32-bits,
meaning a stack size of 64 (as shown in the example below) is 256 bytes wide.

The **task handler** has the following signature: `void my_task_func(void *arg)`

```
/* Define task stack and task object */
#define MY_TASK_PRI         (OS_TASK_PRI_HIGHEST)
#define MY_STACK_SIZE       OS_STACK_ALIGN(64)
struct os_task my_task;
os_stack_t my_task_stack[MY_STACK_SIZE];
```

### Initializing a task

To initialize the task, you need to call the `os_init()` function then add
your task to the os via: `os_task_init`. This normally takes plain in the
main loop, or in a dedicated function called inside main like `init_tasks()`.

`os_task_init` has the following signature and parameters:

```
os_task_init(struct os_task *t, const char *name, os_task_func_t func,
        void *arg, uint8_t prio, os_time_t sanity_itvl,
        os_stack_t *stack_bottom, uint16_t stack_size)
```

- `struct os_task *t`: A pointer to the `os_task` to initialize
- `const char *name`: The public name to associate with this task, which will
  be visible in the shell, newtmgr, and other reporting systems.
- `os_task_funct_t func`: The function to execute when this task is active,
  which will have the following signature: `void my_task_handler(void *arg)`
- `void *arg`: Optional arguments to pass into the task handler
- `uint8_t prio`: The priority level for the task, lower = higher priority
- `os_time_t sanity_itvl`: The time at which this task should check in with the
   sanity task. `OS_WAIT_FOREVER` means never check in.
- `os_stack_t *stack_bottom`: A pointer to the bottom of a task's stack.
- `uint16_t stack_size`: The size of the task's stack (in `os_stack_t` units),
  which are usually 32-bits.

The following examples initialises a task matching the values declared earlier
in this document:

```
/* This is the main function for the project */
int main(void) {
    int rc;

    /* Initialize OS */
    os_init();

    /* Initialize the task */
    os_task_init(&my_task, "my_task", my_task_func, NULL, MY_TASK_PRIO,
                 OS_WAIT_FOREVER, my_task_stack, MY_STACK_SIZE);

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return rc;
}
```

### Implementing the task handler

The last part of the system is the task handler, which will be called every
time that the task is active (as determined by the scheduler).

Task handlers are infinite loops that have an initial setup face, and then
normally a `while(1)` loop that runs forever as long as the task is active.

The following example initialises a GPIO pin as an output, setting the pin
high. It then starts an infinite loop and toggles the LED every second,
sleeping between 1s intervals so that other tasks can run:

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
