#!/bin/bash
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
# Called: $0 <bsp_directory_path> <binary> [features...]
#  - bsp_directory_path is absolute path to hw/bsp/bsp_name
#  - binary is the path to prefix to target binary, .elf appended to name is
#    the ELF file
#  - identities is the project identities string.
#
#
if [ $# -lt 2 ]; then
    echo "Need binary to download"
    exit 1
fi

MY_PATH=$1
BASENAME=$2
IS_BOOTLOADER=0

# Look for 'bootloader' from 3rd arg onwards
shift
shift
while [ $# -gt 0 ]; do
    if [ $1 == "bootloader" ]; then
	IS_BOOTLOADER=1
    fi
    shift
done

if [ $IS_BOOTLOADER -eq 1 ]; then
    FLASH_OFFSET=0x00000000
    FILE_NAME=$BASENAME.elf.bin
    # we will unprotect and reprotect our bootloader to ensure that its
    # safe. also Arduino Zero Pro boards looks like they come with the 
    # arduino bootloader protected via the NVM AUX 
    CMD=-c
    UNPROTECT_FLASH="at91samd bootloader 0"
    PROTECT_FLASH="at91samd bootloader 16384"
else
    # this number is to offset the bootloader size 
    FLASH_OFFSET=0x0000C000
    FILE_NAME=$BASENAME.img
fi
echo "Downloading" $FILE_NAME "to" $FLASH_OFFSET

openocd -f $MY_PATH/arduino_zero.cfg -c init -c "reset halt" $CMD "$UNPROTECT_FLASH" -c "reset halt" -c "flash write_image erase $FILE_NAME $FLASH_OFFSET" $CMD "$PROTECT_FLASH" -c "reset run" -c shutdown


