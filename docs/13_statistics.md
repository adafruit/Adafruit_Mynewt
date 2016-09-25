# Statistics

## Configuring Your App/Module for Statistics

Normally statistics are referenced by number, but if you wish to reference them
by name the following cflag should be added to your pkg.yml file:

```
pkg.cflags:
    - "-DSTATS_NAME_ENABLE"
```

The following dependency also needs to be added to the pkg.yml in your app or
module:

```
pkg.deps:
    - "@apache-mynewt-core/sys/stats"
```

## Adding the Stats Struct to your C File

Stats requires the following header to be present:

```
#include <stats/stats.h>
```

This will enable the use of the STATS* macros to define the stats layout,
following the example below:

```
STATS_SECT_START(my_stat_section)
    STATS_SECT_ENTRY(attempt_stat)
    STATS_SECT_ENTRY(error_stat)
STATS_SECT_END
```

At compile time, this will resolve to the following structure:

```
struct stats_my_stat_section {
    struct stats_hdr s_hdr;
    uint32_t sattempt_stat;
    uint32_t serror_stat;
};
```

You will also need to provide names for each field, regardless of WHETHER
you have enabled naming support via `STATS_NAME_ENABLE` or not:

> Note that the field names need to match between the `STATS_SECT_ENTRY` name
above and the `STATS_NAME` entry below!

```
/* define a few stats for querying */
STATS_NAME_START(my_stat_section)
    STATS_NAME(my_stat_section, attempt_stat)
    STATS_NAME(my_stat_section, error_stat)
STATS_NAME_END(my_stat_section)
```

At compile time, this will resolve to the following structure:

```
struct stats_name_map g_stats_map_my_stat_section[] = {
    { __builtin_offsetof (struct stats_my_stat_section, sattempt_stat), "attempt_stat" },
    { __builtin_offsetof (struct stats_my_stat_section, serror_stat), "error_stat" },
};
```

## Accessing the Stats

You will need to declare a global variable  somewhere to holds the stats data,
using the model below:

```
STATS_SECT_DECL(my_stat_section) g_mystat;
```

If the global definition is is another file and you are referencing it
elsewhere, you would declare this in the file where you will modify locally:

```
extern STATS_SECT_DECL(my_stat_section) g_mystat;
```

## Initialising the Stats

Before your stats can be used or accessed, they need to be initialised and
registered.

You can initialise your stats entry as follows:

```
rc = stats_init(
    STATS_HDR(g_mystat),
    STATS_SIZE_INIT_PARMS(g_mystat, STATS_SIZE_32),
    STATS_NAME_INIT_PARMS(my_stat_section));
assert(rc == 0);
```

For the stat size, you can use one of the following values:

- `STATS_SIZE_16` -- stats are 16 bits (wraps at 65536)
- `STATS_SIZE_32` -- stats are 32 bits (wraps at 4294967296)
- `STATS_SIZE_64` -- stats are 64-bits

You then need to register the stats entry so that you can access it, which
is done via the following function call:

> Note: This is the name that you will use when accessing the stats via the
console or via the `newtmgr stat` command

```
rc = stats_register("my_stats", STATS_HDR(g_mystat));
assert(rc == 0);
```

## Updating the Stats Values

### Incrementing

To increment the stats values, you can use the `STATS_INC` or `STATS_INCN`
macros, as shown below:

```
STATS_INC(g_mystat, attempt_stat);
rc = do_task();
if(rc == ERR) {
    STATS_INC(g_mystat, error_stat);        
}
```

## Accessing Stats with the Console or `newtmgr`

### Console Access

Assuming that you have enabled named access to stats via `STATS_NAME_ENABLE`
you can access your stats from the console via:

```
stat my_stats
```

This will give you something resembling the following output:

```
12274:attempt_stat: 3
12275:error_stat: 0
```

If you don't have names enabled via `STATS_NAME_ENABLE` you would see
something like this:

```
stat my_stats
29149:s0: 3
29150:s1: 0
```

### `newtmgr` Access

You can also access stats through newtmgr as follows:

```
$ newtmgr -c serial1 stat my_stats
Return Code = 0
Stats Name: my_stats
  attempt_stat: 0
  error_stat: 0
```
