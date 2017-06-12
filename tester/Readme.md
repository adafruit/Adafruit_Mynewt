Mynewt Setup (RPi 3)
--------------------

# 1. Install golang 1.8.3 binaries
$ sudo wget https://storage.googleapis.com/golang/go1.8.3.linux-armv6l.tar.gz
$ sudo tar xzf go1.8.3.linux-armv6l.tar.gz -C /usr/local
$ sudo rm go1.8.3.linux-armv6l.tar.gz
$ sudo chgrp -R staff /usr/local/go

# 2. Add go 1.8.3 to the PATH variable
$ export PATH=/usr/local/go/bin:$PATH

# 3. Check the go version (should be 1.8.3 NOT 1.3.3)
$ go version
```
go version go1.8.3 linux/arm
```

# 4. Setup a dir for local go workspace
$ cd $HOME
$ mkdir -p dev/go
$ cd dev/go
$ export GOPATH=`pwd`

# 5. Install Mynewt tools into the workspace
$ cd $GOPATH
$ go get mynewt.apache.org/newt/newt
$ go get -ldflags -s mynewt.apache.org/newtmgr/newtmgr

# 6. Add Mynewt tools (newt, newtmgr) to the path
$ export PATH=$GOPATH/bin:$PATH

# 7. Make a local copy of the newt/newtmgr binaries for later use
$ cd /boot/testing
$ sudo mkdir nrf52pro
$ cd nrf52pro
$ mkdir bin
$ sudo cp $HOME/dev/go/bin/newt bin/
$ sudo cp $HOME/dev/go/bin/newtmgr bin/
$ bin/newt version
```
Apache Newt (incubating) version: 1.0.0-dev
```

# 8. Copy the firmware binary files to the local directory
$ scp feather52_boot_only.bin pi@192.168.0.165:~
$ scp feather52_boot_bleuart.bin pi@192.168.0.165:~
> Don't forget to Copy the files locally (from $HOME)

# 9. Flash the bootloader and Mynewt test app to the device
$ adalink nrf52832 -p jlink -b feather52_boot_bleuart.bin 0x0
> You should see blinky at this point! ***

# 10. ONCE ONLY! Add a serial connection for the newtmgr tool
$ bin/newtmgr conn add serial type=serial connstring=/dev/ttyUSB0
$ bin/newtmgr conn show

# 11. Test the serial connection
$ bin/newtmgr -c serial taskstat
```
      task pri tid  runtime      csw    stksz   stkuse last_checkin next_checkin
    ble_ll   0   2       21     4381       80       58        0        0
   bleuart   5   3        0    12030      256       31        0        0
      idle 255   0    11980    16292       64       26        0        0
      main 127   1        0       27     1024      210        0        0
```
