# Adding a Blinky Task to Your App

Tasks form the basis of any RTOS, and work in combination with the scheduler.
The scheduler decides which task should be running at any given time and
allocates clock cycles accordingly.

Each task is made up of two components:

- Some **Task stack memory**, which is used to store local variables, and
maintain the system stack state at a given moment in time
- And a **task function**, which is the function that is called when the scheduler
activates the specific task, restoring the system stack to the previous state.

## Creating a Custom Task

To add a new task to your application, and let the scheduler know that the task
exists, you can add the following code to your main.c file:

### Task Parameters

```
/* Define task stack and task object */
#define MY_TASK_PRI         (OS_TASK_PRI_HIGHEST)
#define MY_STACK_SIZE       (64)
struct os_task my_task;
os_stack_t my_task_stack[MY_STACK_SIZE];
```

`MY_TASK_PRI` sets the priority of the task, where priority determines which
task should be executed first by the scheduler when multiple tasks are present.
Priority can be a value from 0x00 to 0xFF, where the lower the number the
higher the priority assigned to the task is (5 will execute before 15).

> You generally should ensure that tasks have unique priority levels so that
there is always a clear understanding of code flow and execution in your
application.

`MY_STACK_SIZE` is the amount of stack memory to assign to the task, which is
used to hold local variables and the system stack state.

> **NOTE**: The stack size is create in units of `os_stack_t`, **NOT** bytes,
and is usually 32-bits wide. In the example above we are most likely setting
the stack size for our task to 256 bytes, them (64 * 4), which is appropriate
for a relatively simple task.

`struct os_task my_task` is the variable name that you will use when passing
the task into the scheduler, and this variable will be filled with the functions
name and any properties about the task.

`os_stack_t my_task_stack[MY_STACK_SIZE]` allocates the memory that will be
used as stack space for your task.

### Task Handler Function

Next, you need to implement your task, as shown in the sample code below:

> **NOTE**: This task requires the `bsp/bsp.h` and `hal/hal_gpio.h` headers.
This task also assumes that an LED is available on your development
board and that the `LED_BLINK_PIN` macro is defined in the BSP.

```
/* This is the task function */
void my_task_func(void *arg) {
    /* Set the led pin as an output */
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    /* The task is a forever loop that does not return */
    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC*1);

        /* Toggle the LED */
        hal_gpio_toggle(LED_BLINK_PIN);
    }
}
```

### Register the Task

In your main loop, you then need to register your task with the scheduler,
which happens via the `os_task_init` function (called after `os_init` and
before `os_start`):

```
/* This is the main function for the project */
int main(void) {
    /* Initialize OS */
    os_init();

    /* Initialize the task */
    os_task_init(&my_task, "my_task", my_task_func, NULL, MY_TASK_PRI,
                 OS_WAIT_FOREVER, my_task_stack, MY_STACK_SIZE);

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);
}
```

> Note: Try to keep the public task names ("my_task" above) to **7 characters
or less** so that they show up concisely in newtmgr and reporting systems.

# Complete Source

You should end up with the following code in your `main.c` file:

```
#include <assert.h>
#include "bsp/bsp.h"
#include "os/os.h"
#include "hal/hal_gpio.h"

/* Define task stack and task object */
#define MY_TASK_PRI         (OS_TASK_PRI_HIGHEST)
#define MY_STACK_SIZE       (64)
struct os_task my_task;
os_stack_t my_task_stack[MY_STACK_SIZE];

/* This is the task function */
void my_task_func(void *arg) {
    /* Set the led pin as an output */
    hal_gpio_init_out(LED_BLINK_PIN, 1);

    /* The task is a forever loop that does not return */
    while (1) {
        /* Wait one second */
        os_time_delay(OS_TICKS_PER_SEC*1);

        /* Toggle the LED */
        hal_gpio_toggle(LED_BLINK_PIN);
    }
}

int main(void) {
    /* Initialize OS */
    os_init();

    /* Initialize the task */
    os_task_init(&my_task, "my_task", my_task_func, NULL, MY_TASK_PRI,
                 OS_WAIT_FOREVER, my_task_stack, MY_STACK_SIZE);

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);
}
```

# Testing the Blinky Task

You can build the code via the following command:

```
$ newt build mini
```

If everything built correctly, you should end up with the following message:

```
Building target targets/mini
Compiling main.c
Archiving mini.a
...
Linking /[local_path]/bin/targets/mini/app/apps/mini/mini.elf
Target successfully built: targets/mini
```

Assuming you have a bootloader installed, you can run your project via the
following commands (make sure your debugger is connected first!):

```
$ newt create-image mini 0.1.0
$ newt load mini
```
