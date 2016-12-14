# Shell Command Handler

## Adding shell support

To add shell support to your app make sure the following `pkg.deps` are
defined in your pkg.yml file:

```
pkg.deps:
    - "@apache-mynewt-core/sys/console/full"
    - "@apache-mynewt-core/sys/shell"
    - "@apache-mynewt-core/sys/sysinit"
```

In the `syscfg.vals` section of syscfg.yml (>= 0.10.0) add the following:

```
syscfg.vals:
    # Enable the shell task.
    SHELL_TASK: 1
```

## Adding a default system event queue

In your main.c file add the following code snippets:

```
#include "sysinit/sysinit.h"
#include "console/console.h"
#include "shell/shell.h"

/* System event queue task handler */
#define SYSEVQ_PRIO (10)
#define SYSEVQ_STACK_SIZE    OS_STACK_ALIGN(512)
static struct os_task task_sysevq;

/* Event queue for events handled by the system (shell, etc.) */
static struct os_eventq sys_evq;

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
```

In the `init_tasks` function (or in main if you prefer) add:

```
/**
 * init_tasks
 *
 * Called by main.c after sysinit(). This function performs initializations
 * that are required before tasks are running.
 *
 * @return int 0 success; error otherwise.
 */
static void
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

    /* Init shell support */
    shell_init();
}
```

Between `sysinit` and `os_start` in main add:

```
init_tasks();
```

## Adding a custom command handler

To add a new command handler use the following code snippets:

```
// Command handler prototype declaration
static int shell_test_cmd(int argc, char **argv);

// Shell command struct
static struct shell_cmd shell_test_cmd_struct = {
    .sc_cmd = "test",
    .sc_cmd_func = shell_test_cmd
};

...

// Implement your command handler
static int
shell_test_cmd(int argc, char **argv)
{
    console_printf("Test!\n");
    return 0;
}

...

// Call this before os_init to register the command
#if MYNEWT_VAL(SHELL_TASK)
    shell_cmd_register(&shell_test_cmd_struct);
#endif
```
