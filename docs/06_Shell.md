# Shell Command Handler

## Adding Shell Support

To add shell support to your app make sure the following `pkg.deps` are
defined in your pkg.yml file:

```
"@apache-mynewt-core/libs/console/full"
"@apache-mynewt-core/libs/shell"
```

In your main.c file add the following code snippets:

```
#include "console/console.h"
#include "shell/shell.h"

...

/* Shell task */
#define SHELL_TASK_PRIO       (1)
#define SHELL_TASK_STACK_SIZE (OS_STACK_ALIGN(256))
os_stack_t shell_stack[SHELL_TASK_STACK_SIZE];

/* Shell maximum input line length */
#define SHELL_MAX_INPUT_LEN     (256)

...

void
init_cli(void)
{
    int rc;

    /* Init the console */
    rc = console_init(shell_console_rx_cb);
    assert(rc == 0);

    rc = shell_task_init(SHELL_TASK_PRIO, shell_stack, SHELL_TASK_STACK_SIZE,
                         SHELL_MAX_INPUT_LEN);
    assert(rc == 0);
}

...

// Call this before os_start()
init_cli();
```

## Adding Command Handlers

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

// Call this before shell_task_init to register the command
shell_cmd_register(&shell_test_cmd_struct);
```
