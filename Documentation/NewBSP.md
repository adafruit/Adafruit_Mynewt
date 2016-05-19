# Creating a BSP in Mynewt

Based on http://mynewt.apache.org/os/core_os/porting/port_bsp/

### Create the hw/bsp Directory

```
$ mkdir -p hw/bsp
```

### Copy an Existing BSP

Copy an existing BSP from `repos/apache-core-newt/hw/bsp` into the root `hw/bsp`
folder of your project and rename it to something appropriate.

In this example we'll assume `hw/bsp/boardname`.

### Rename Files

You probably want to rename files to include references to `boardname`,
including in the `pkg.yml` file.

### Edit the BSP Package Details

Edit the `pkg.yml` file to make sure that pkg.name matches the folder name
where the BSP has been placed

```
    pkg.name: "hw/bsp/boardname"
```

Next update the relative paths for the dependencies to point them to the
`@apache-mynewt-core` repo since they have been moved a few levels higher.

Take note in particular of `pkg.compiler` and `pkg.deps*`. The updated file
should resemble something like this:

```
pkg.arch: cortex_m0
pkg.compiler: "@apache-mynewt-core/compiler/arm-none-eabi-m0"
pkg.linkerscript: "boardname.ld"
pkg.linkerscript.bootloader.OVERWRITE: "boot-boardname.ld"
pkg.downloadscript: boardname_download.sh
pkg.debugscript: boardname_debug.sh
pkg.cflags: -DNRF51
pkg.deps:
    - "@apache-mynewt-core/hw/mcu/nordic/nrf51xxx"
    - "@apache-mynewt-core/libs/baselibc"
pkg.deps.BLE_DEVICE:
    - "@apache-mynewt-core/net/nimble/drivers/nrf51"
```

You may need to modify other values as well if you rename any files, etc.

### Edit BSP Content Where Required

You can update any source code in the BSP, such as changing `LED_BLINK_PIN`
defined in `hw/bsp/boardname/include/bsp/bsp.h`.

### Set the Target's BSP

Set the `bsp` for your target to `boardname`:

```
$ newt target set target_name bsp=hw/bsp/boardname
```
