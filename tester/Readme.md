# Mynewt Setup (RPi 3)

## 1. Install golang 1.8.3 binaries
Mynewt provides a set of command line tools written in [golang](https://golang.org/). To compile the tools down to platform-specific binaries, you will need to install Go 1.6 or higher. 1.8.3 is used here. 
```
$ sudo wget https://storage.googleapis.com/golang/go1.8.3.linux-armv6l.tar.gz
$ sudo tar xzf go1.8.3.linux-armv6l.tar.gz -C /usr/local
$ sudo rm go1.8.3.linux-armv6l.tar.gz
$ sudo chgrp -R staff /usr/local/go
```

## 2. Add go 1.8.3 to the PATH variable
```
$ export PATH=/usr/local/go/bin:$PATH
```

## 3. Check the go version (should be 1.8.3 NOT 1.3.3)
```
$ go version
```
> This should give you `go version go1.8.3 linux/arm`

## 4. Setup a dir for local go workspace
In addition to the go toolchain, you will need to setup a local workspace where go libraries and projects can be stored. The suggested location for this is `$HOME/dev/go`:
```
$ cd $HOME
$ mkdir -p dev/go
$ cd dev/go
$ export GOPATH=`pwd`
```

## 5. Install Mynewt tools into the workspace
Finally, you can download and build the mynewt tools (`newt` and `newtmgr`) via the `go get` command:
```
$ cd $GOPATH
$ go get mynewt.apache.org/newt/newt
$ go get -ldflags -s mynewt.apache.org/newtmgr/newtmgr
```

## 6. Optional: Add Mynewt tools (newt, newtmgr) to the path
If you wish to use `newt` and `newtmgr` anywhere on the system, you can add them to the `$PATH` variable as follows:
```
$ export PATH=$GOPATH/bin:$PATH
```

## 7. Make a local copy of the newt/newtmgr binaries for later use
```
$ cd /boot/testing
$ sudo mkdir nrf52pro
$ cd nrf52pro
$ mkdir bin
$ sudo cp $HOME/dev/go/bin/newt bin/
$ sudo cp $HOME/dev/go/bin/newtmgr bin/
$ bin/newt version
```
> This should give you `Apache Newt (incubating) version: 1.0.0-dev`

## 8. Copy the firmware binary files to the local directory
You will need to take the two .bin files in this repository and copy them to the SD card on your RPi3. There are a variety of methods to do this, but `scp` is one option, and is described below from the perspective of the command-line on your development machine (Linux or OS X should have `scp` support out of the box):
```
$ scp feather52_boot_only.bin pi@192.168.0.165:~
$ scp feather52_boot_bleuart.bin pi@192.168.0.165:~
```
> Don't forget to Copy the files locally (from `$HOME`), or update the path variable in the scp command above.

## 9. Flash the bootloader and Mynewt test app to the device
Now that the firmware binary files are available on the RPi, you can flash them via adalink and a Segger JLink as follows:
```
$ adalink nrf52832 -p jlink -b feather52_boot_bleuart.bin 0x0
```
> You should see blinky at this point!

## 10. ONCE ONLY! Add a serial connection for the newtmgr tool
Before we can test the serial communication to the bootloader and the mynewt binary we just flashed, we will need to setup the communication channel between the `newtmgr` too and the development board. This will happen via the CP2104 TTY/Serial adapter on the nRF52 Feather Pro, so we setup a new serial connection once as follows (naming it `serial` in this case):
```
$ bin/newtmgr conn add serial type=serial connstring=/dev/ttyUSB0
$ bin/newtmgr conn show
```

## 11. Test the `newtmgr` serial connection
You can test the `serial` connection with `newtmgr` now as follows:
```
$ bin/newtmgr -c serial taskstat
```
Which should give you the following task statistics:
```
      task pri tid  runtime      csw    stksz   stkuse last_checkin next_checkin
    ble_ll   0   2       21     4381       80       58        0        0
   bleuart   5   3        0    12030      256       31        0        0
      idle 255   0    11980    16292       64       26        0        0
      main 127   1        0       27     1024      210        0        0
```
