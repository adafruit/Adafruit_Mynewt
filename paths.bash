#!/bin/bash

# Add Go and Newt Paths (Mynewt, etc.)
export GOPATH=$HOME/prog/go
export PATH="$GOPATH"/bin/:$PATH

# GCC paths
export PATH=$PATH:$HOME/prog/gcc-arm-none-eabi-4_9-2015q1/bin
export PATH=$PATH:/usr/local/Cellar/gcc/5.3.0/bin
