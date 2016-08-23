# Mynewt

This guide fills in some details from the
[Installing Native Mynewt Tools](http://mynewt.apache.org/os/get_started/native_tools/)
page in the official documentation. Consult that page to get started setting
Mynewt and related tools up on your system.

## Installation Notes

The following problems were encountered when trying to setup Mynewt on an OS X
10.11 based system. A local installation was used rather than the Docker VM
option.

### GCC Paths

The following paths must be available from the command-line:

```
export PATH=$PATH:/Users/ktown/prog/gcc-arm-none-eabi-4_9-2015q1/bin
export PATH=$PATH:/usr/local/Cellar/gcc/5.3.0/bin
```

### Newt and Go Paths

To make `newt` available from the command line the following was added to
`~/.bash_profile` (you'll need to restart terminal to take effect):

```
# Add Go and Newt Paths (Mynewt, etc.)
export GOPATH=$HOME/prog/go
export PATH="$GOPATH"/bin/:$PATH
```
**UPDATE:** To simplify the path issues,
[paths.bash](https://github.com/adafruit/Adafruit_Mynewt/blob/master/paths.bash)
was added to the root folder of the project, with the following contents:

```
#!/bin/bash

# Add Go and Newt Paths (Mynewt, etc.)
export GOPATH=$HOME/prog/go
export PATH="$GOPATH"/bin/:$PATH

# GCC paths
export PATH=$PATH:/Users/ktown/prog/gcc-arm-none-eabi-4_9-2015q1/bin
export PATH=$PATH:/usr/local/Cellar/gcc/5.3.0/bin
```

To enable the required paths, simply run `source paths.bash` before you start
using `newt` and other tools.

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
