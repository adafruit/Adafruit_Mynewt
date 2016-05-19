# Logging with `log/log.h`

Mynewt includes a [logging module](http://mynewt.apache.org/os/modules/logs/logs/)
that can be used at run time to log various levels of system information.

The level of information included in the binary image is set at compiled time
via the `LOG_LEVEL` compiler flag.

### Add Log Dependency

To enable logging support in your app add the following package dependencies to
you `pkg.yml` file:

```
pkg.deps:
    - "@apache-mynewt-core/sys/log"
```

### Set the Logging Level for the compiler

Log levels are defined in `apache-mynewt-core/sys/log/include/log/log.h`:

```
#define LOG_LEVEL_DEBUG    (0x01)
#define LOG_LEVEL_INFO     (0x02)
#define LOG_LEVEL_WARN     (0x04)
#define LOG_LEVEL_ERROR    (0x08)
#define LOG_LEVEL_CRITICAL (0x10)
/* Up to 7 custom log levels. */
#define LOG_LEVEL_PERUSER  (0x12)
```

The appropriate log level must be referenced as numeric values for the CFLAGs,
as shown below:

```
$ newt target set my_target cflags=-DLOG_LEVEL=8
```

This will add a `pkg.cflags -DLOG_LEVEL=8` flag which resolves to
`LOG_LEVEL_ERROR`, meaning that anything lower level that ERROR will not be
included in the firmware, keeping the code size smaller than a full debug
image might be.

### Add Logging Support at the App/Module Level

To use logging, you must create a log handler object in your app. You can uses
one of the following canned handlers:

- `console` - Streams log events to the console port
- `cbmem` - Commits log events to a circular buffer, and can be accessed via
  the `newtmgr` tool.

In an appropriate **header file** that will be included in your module add the
following code:

```
#include "log/log.h"

extern struct log my_log;
```

The in the **implementation c file** include the following:

```
struct log_handler log_console_handler;
struct log my_log;

void
init_log(void)
{
    log_init();
    log_console_handler_init(&log_console_handler);
    log_register("log", &my_log, &log_console_handler);
}
```

### Logging a Message

To log a message use an appropriate logging macro, as shown below:

```
LOG_DEBUG(&my_log, LOG_MODULE_DEFAULT, "bla");
LOG_DEBUG(&my_log, LOG_MODULE_DEFAULT, "bab");
```

Other variants of `LOG_*` exist based on the logging level:

- `LOG_DEBUG` - The lowest level, 0x01
- `LOG_INFO` - Level 0x02
- `LOG_WARN` - Level 0x04
- `LOG_ERROR` - Level 0x08
- `LOG_CRITICAL` - Level 0x10

If you wish to log to one of the **user log levels** you can defined a custom
LOG macro as follows:

```
#include "log/log.h"

extern struct log bleprph_log;

/* bleprph uses the first "peruser" log module. */
#define BLEPRPH_LOG_MODULE  (LOG_MODULE_PERUSER + 0)

/* Convenience macro for logging to the bleprph module. */
#define BLEPRPH_LOG(lvl, ...) \
    LOG_ ## lvl(&bleprph_log, BLEPRPH_LOG_MODULE, __VA_ARGS__)
```
