# Newtmgr Image Management Tool

`newtmgr` can be used to interact with the bootloader and images on the device.

For details see: http://mynewt.apache.org/newtmgr/overview/

## Connection Profiles

The `newtmgr` tools works with connection profiles, such as serial or ble,
depending on how you wish to communicate with the device under test.

Before you can use `newtmgr` you will need to setup at least one connection
profile, as described below:

### Adding a Serial Connection Profile

```
$ newtmgr conn add serial1 type=serial connstring=/dev/tty.usbserial-DJ004OSX
```

### Listing Existing Profiles

You can get a list of all defined connection profiles via:

```
$ newtmgr conn show
```

### Connect Using a Profile

This command will connect using the specified profile and list application
images present in flash memory:

```
$ newtmgr -c serial image list
```

## Adding Image Management Support to apps

To enable image management support via `newtmgr` in your application, you must include the following dependency in your `pkg.yml` file:

```
- "@apache-mynewt-core/libs/imgmgr"
```

You also need to make sure that you initialise the image manager by calling `imgmgr_module_init();` in your startup code.

## Uploading Application Images with `newtmgr`

The `newtmgr` tool can be used to upload an application image to bank number
2 in flash memory, and then switch application images during the next reset.

First create your image with an appropriate version number:

```
$ newt create-image throughput 0.4.6
```

Then copy the image to bank 2 of flash memory using the serial connection:

```
$ newtmgr -c serial1 image upload bin/throughput/apps/throughput/throughput.img
```

You should be able to see both application images in the two flash banks now:

```
$ newtmgr -c serial1 image list
Images:
    0 : 0.2.2
    1 : 0.4.6
```

Check the current boot image (0.2.2 in this case):

```
$ newtmgr -c serial1 image boot
    Test image :
    Main image : 0.2.2
    Active img : 0.2.2
```

Switch the boot image to 0.4.6:

```
$ newtmgr -c serial1 image boot 0.4.6
```

Now **reset your device**. When the device start up there will be a delay while
the image from flash bank 1 is copied over to flash bank 0, and the old image
is removed. Once the move is complete, the application image will begin
execution normally.

You can verify the currently executing version as follows:

```
$ newtmgr -c serial1 image list
Images:
    0 : 0.4.6
```
