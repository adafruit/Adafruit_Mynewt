# Creating a New Project with Newt

To create a new project with the `newt` tool perform the following steps. The
nRF52 DK is used as a sample platform in this example.

## Create a project skeleton

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

## Setup a bootloader target

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

## Create an application

### Option 1: Copy an existing demo app as a target

> **Note:** When the project is created via `newt new projectname` a bare bones
> blinky app will be created in `apps/blinky`. You can use this as a starting
> point for your project instead of manually creating a new one, which is
> described 'Option 2' below.

Import the standard blinky demo as a working example:

```
$ newt target create blink_nordic
$ newt target set blink_nordic app=apps/blinky
$ newt target set blink_nordic bsp=@apache-mynewt-core/hw/bsp/nrf52dk
$ newt target set blink_nordic build_profile=debug
```

### Option 2: Create an entirely new application as a target

For details on how to create a custom app from scratch see apps/00_CreateApp.md

## Review the target(s)

At this point you should have two targets setup, one for the bootloader and one for the app you created.

To display a list of targets in your project enter:

```
$ newt target show
```

This should give you a list resembling the following (assuming option 1 above):

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

## Adding optional flags

You can add option flags and values to the target at this point as well, such
as the following command which will enable stand-alone serial support for
the bootloader:

```
# clear cflags
$ newt target set nrf52_boot cflags=

# set feature
$ newt target set nrf52_boot features=BOOT_SERIAL
```

This adds the following entry to the pkg.yml file for the nrf52_boot target:

```
pkg.cflags:
    - "-DBOOT_SERIAL"
```

## Building the Project

To build the targets created above, run the `newt build` command once for
each target required:

```
$ newt build nrf52_boot
$ newt build blink_nordic
```

You can optionally speed the build process up by using multiple threads via
the `-j` argument, as shown below:

```
$ newt -j 5 build nrf52_boot
$ newt -j 5 build blink_nordic
```

### Sign the Build

You then need to **sign the build** for any app(s) so that we have some basic
version information and so that the bootloader will accept the firmware image(s),
which is done via the `newt create-image` command:

```
$ newt create-image blink_nordic 1.0.0
```

## Flashing the Device

You can now flash the device via newt with the following command(s):

```
$ newt -v load nrf52_boot
$ newt -v load blink_nordic
```

You may need to reset of power cycle the target device for the changes to take
effect, depending on the HW platform used.
