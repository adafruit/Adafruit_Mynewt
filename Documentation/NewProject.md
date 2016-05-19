# Creating a New Project with Newt

To create a new project with the `newt` tool perform the following steps. The
nRF52 DK is used as a sample platform in this example.

### Create the skeleton project

```
$ newt new projectname
$ cd projectname
```

### Download project dependencies

Next download the project dependencies from the associated repos.

Since this is a new project, only `apache-mynewt-core` will be downloaded:

```
$ newt install -v
```

### Setup the bootloader target

Create a new `nrf52_boot` target for the bootloader image:

```
$ newt target create nrf52_boot
```

Set the target to point to `/apps/boot` in the `apache-mynewt-core`
dependency imported earlier (`repo/apache-mynewt-core/apps/boot`):

```
$ newt target set nrf52_boot app=@apache-mynewt-core/apps/boot
```

Import the BSP target for your specific board:

```
$ newt target set nrf52_boot bsp=@apache-mynewt-core/hw/bsp/nrf52dk
```

Set the `build_profile` flag to `optimized` for the bootloader target:

```
$ newt target set nrf52_boot build_profile=optimized
```

### Option 1: Create an entirely new application as a target

> **Note:** When the project is created via `newt new projectname` a bare bones
> blinky app will be created in `apps/blinky`. You can also use this as a
> starting point for your project instead of creating a new one, as described
> in 'Option 2' further down.

A mynewt app requires at least a `main()` function and a `pkg.yml` file.

Create the new app structure as follows:

```
$ mkdir -p apps/ble_app/src
```

Then create the core `apps/ble_app/pkg.yml` file with the following text (BLE
dependencies listed below for convenience sake):

```
pkg.name: apps/ble_app
pkg.type: app

pkg.deps:
    - "@apache-mynewt-core/libs/baselibc"
    - "@apache-mynewt-core/libs/console/full"
    - "@apache-mynewt-core/libs/os"
    - "@apache-mynewt-core/net/nimble/controller"
    - "@apache-mynewt-core/net/nimble/host"
```

Paste the following code into a new `apps/ble_app/src/main.c` file:

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

Create the new target with the following command:

```
$ newt target create ble_app
```

Point the new target to the appropriate `app`:

```
$ newt target set ble_app app=apps/ble_app
```

Set the `bsp` and `build profile` for the new target:

```
$ newt target set ble_app bsp=@apache-mynewt-core/hw/bsp/nrf52dk
$ newt target set ble_app build_profile=debug
```

### Option 2: Copy an existing demo app as a target

Import the standard blinky demo as a working example:

```
$ newt target create blink_nordic
$ newt target set blink_nordic app=apps/blinky
$ newt target set blink_nordic bsp=@apache-mynewt-core/hw/bsp/nrf52dk
$ newt target set blink_nordic build_profile=debug
```

### Review the target(s)

```
$ newt target show
```

This should give you a list resembling the following (assuming option 2 above):

```
targets/blink_nordic
    app=apps/blinky
    bsp=@apache-mynewt-core/hw/bsp/nrf52dk
    build_profile=debug
targets/nrf52_boot
    app=@apache-mynewt-core/apps/boot
    bsp=@apache-mynewt-core/hw/bsp/nrf52dk
    build_profile=optimized
```

# Building the Project

To build the targets created above, run the `newt build` command once for
each target required:

```
$ newt build nrf52_boot
$ newt build blink_nordic
```

# Sign the Build

You then need to **sign the build** so that we have some basic version
information and so that the bootloader will accept the firmware image(s), which
is done via the `newt create-image` command:

```
$ newt create-image blink_nordic 1.0.0
```

# Flashing the Device

You can now flash the device via newt with the following command(s):

```
$ newt -v load nrf52_boot
$ newt -v load blink_nordic
```
You may need to reset of power cycle the target device for the changes to take
effect.
