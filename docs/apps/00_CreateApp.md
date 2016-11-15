# Creating an App in Mynewt

> These notes are based on `1.0.0-develop` and may vary with the latest release

This document makes the following assumptions:

- You have already created a project (`$ newt new projname`)
- You have already installed mynewt dependencues (`$ newt install`)
- You are familiar with how to build and load apps via the `newt` tool
- You have already created a bootloader target
- You have already tested a standard demo with your BSP

If you are not familiar with these concepts, please see the rest of the
documentation in the parent folder of this guide.

## Create Your App

A Mynewt app requires at least a `main()` function and a `pkg.yml` file.

Create the new app structure as follows:

```
$ mkdir -p apps/mini/src
```

Then create the core `apps/mini/pkg.yml` file with the following text (for
example via `$ nano apps/mini/pkg.yml`, or using your favorite text editor):

```
pkg.name: apps/mini
pkg.type: app

pkg.deps:
    - "@apache-mynewt-core/libc/baselibc"
    - "@apache-mynewt-core/sys/console/full"
    - "@apache-mynewt-core/kernel/os"
```

Then create a `apps/mini/src/main.c` file with the following code:

```
#include <assert.h>
#include "os/os.h"

int
main(void)
{
    /* Initialize OS */
    os_init();

    /* Start the OS */
    os_start();

    /* os_start should never return. If it does, this should be an error */
    assert(0);
}
```

## Adding a New target

Newt, you need to create a new target that points to your app via the following
command:

```
$ newt target create mini
```

### Set the Target's `app` Field

Point the new target to the appropriate **app** via:

```
$ newt target set mini app=apps/mini
```

### Set the Target's `bsp` Field

Next set the **bsp** for the new target, which indicates the HW that the app
will be running on.

If you are using the **Adafruit Mynewt Feather** this might be:

```
$ newt target set mini bsp=hw/bsp/feather52
```

For the Nordic nRF52DK this would be:

```
$ newt target set mini bsp=@apache-mynewt-core/hw/bsp/nrf52dk
```

### Set the `build_profile` Field

Finally set the **build profile** for the new target (`debug` or `optimized`):

```
$ newt target set mini build_profile=debug
```

### Build the target

You can now attempt to build your project with the following command:

```
$ newt build mini
```

If everything is configured properly, you should see output similar to this:

```
Building target targets/mini
Compiling main.c
Archiving mini.a
Compiling bootutil_misc.c
Compiling image_ec.c
Compiling image_rsa.c
...
Compiling mfg.c
Archiving mfg.a
Compiling mem.c
Archiving mem.a
Linking /[local_path]/bin/targets/mini/app/apps/mini/mini.elf
Target successfully built: targets/mini
```
