# Mynewt Installation Notes

This guide fills in some details from the
[Installing Native Mynewt Tools](http://mynewt.apache.org/os/get_started/native_tools/)
page in the official documentation. Consult that page to get started setting
Mynewt and related tools up on your system.

## Path Requirements

The following paths and values must be available from the command-line:

#### GCC Paths

An ARM cross compiler is required for firmware builds, as well as a native
GCC compiler when building tools, unit tests and simulating hardware. Both
of these toolchains can be added to the `$PATH` variable as shown below:

```
export PATH=$PATH:/Users/ktown/prog/gcc-arm-none-eabi-4_9-2015q1/bin
export PATH=$PATH:/usr/local/Cellar/gcc/5.3.0/bin
```

#### Newt and Go Paths

To build the native tools, and make `newt` available from the command
line the following values must also be define on your system:

```
# Add Go and Newt Paths (Mynewt, etc.)
export GOPATH=$HOME/prog/go
export PATH="$GOPATH"/bin/:$PATH
```

### Creating a `paths.bash` Script

To simplify the path issues described above, simply add a
[paths.bash](https://github.com/adafruit/Adafruit_Mynewt/blob/master/paths.bash)
file to the root folder of the project, with the following content:

```
#!/bin/bash

# Add Go and Newt Paths (Mynewt, etc.)
export GOPATH=$HOME/prog/go
export PATH="$GOPATH"/bin/:$PATH

# GCC paths
export PATH=$PATH:/Users/ktown/prog/gcc-arm-none-eabi-4_9-2015q1/bin
export PATH=$PATH:/usr/local/Cellar/gcc/5.3.0/bin
```

To enable the required paths, just run `source paths.bash` before you start
using `newt` and other tools.

If everything is setup correctly, you should be able to run `newt version`:

```
$ newt version
Apache Newt (incubating) version: 0.9.0
```

## Installation Issues

The following problems were encountered when trying to setup Mynewt on an OS X
10.11 based system. A local installation was used rather than the Docker VM
option:

### Error: /usr/local/bin/gcc-5: No such file or directory

Following the guide here: http://mynewt.apache.org/os/get_started/native_tools/
GCC sometimes fails (at least on OS X 10.11) when you try to use a command:

```
$ newt test @apache-mynewt-core/libs/os
Testing package @apache-mynewt-core/libs/os
sh: /usr/local/bin/gcc-5: No such file or directory
Error: Test failure(s):
Passed tests: []
Failed tests: [libs/os]
```

You have to install GCC via:

```
brew install gcc
```

And make sure that gcc-5 is available in the $PATH variable (see above).
