/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef _SAMD21_HAL_SPI_H__
#define _SAMD21_HAL_SPI_H__

/* this is where the pin definitions are */
#include "../../src/sam0/utils/header_files/io.h"

enum samd21_spi_device 
{
    SAMD21_SPI_SERCOM0,
    SAMD21_SPI_SERCOM1,
    SAMD21_SPI_SERCOM2,
    SAMD21_SPI_SERCOM3,
    SAMD21_SPI_SERCOM4,
    SAMD21_SPI_SERCOM5,
};

enum samd21_spi_mux_setting 
{
    SAMD21_SPI_MUX_SETTING_A,
    SAMD21_SPI_MUX_SETTING_B,
    SAMD21_SPI_MUX_SETTING_C,
    SAMD21_SPI_MUX_SETTING_D,
    SAMD21_SPI_MUX_SETTING_E,
    SAMD21_SPI_MUX_SETTING_F,
    SAMD21_SPI_MUX_SETTING_G,
    SAMD21_SPI_MUX_SETTING_H,
    SAMD21_SPI_MUX_SETTING_I,
    SAMD21_SPI_MUX_SETTING_J,
    SAMD21_SPI_MUX_SETTING_K,
    SAMD21_SPI_MUX_SETTING_L,
    SAMD21_SPI_MUX_SETTING_M,
    SAMD21_SPI_MUX_SETTING_N,
    SAMD21_SPI_MUX_SETTING_O,
    SAMD21_SPI_MUX_SETTING_P,
};


/* These have to be set appropriately to be valid combination */
struct samd21_spi_config {
    uint8_t                     dipo; 
    uint8_t                     dopo;
    uint32_t                    pad0_pinmux;
    uint32_t                    pad1_pinmux;
    uint32_t                    pad2_pinmux;
    uint32_t                    pad3_pinmux;
};

/* This creates a new SPI object based on the samd21 TC devices */
struct hal_spi * 
samd21_spi_create(enum samd21_spi_device dev_id, 
                  const struct samd21_spi_config *pconfig);


#endif /* _SAMD21_HAL_SPI_H__ */