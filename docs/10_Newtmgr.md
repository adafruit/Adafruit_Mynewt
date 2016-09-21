# Newtmgr Image Management Tool

`newtmgr` can be used to interact with the bootloader and images on the device.

For details see: http://mynewt.apache.org/newtmgr/overview/

### Adding a Serial Connection Profile

```
$ newtmgr conn add serial1 type=serial connstring=/dev/tty.usbserial-DJ004OSX
```

### Listing Existing Profiles

```
$ newtmgr conn show
```

### Connect Using a Serial Profile

```
$ newtmgr -c serial image list
```
