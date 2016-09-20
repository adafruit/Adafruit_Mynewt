/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hal/hal_spi.h>
#include <hal/hal_spi_int.h>
#include <assert.h>
#include <compiler.h>
#include "port.h"
#include <mcu/hal_spi.h>
#include <sercom.h>
#include <spi.h>
#include <mcu/samd21.h>

struct samd21_spi_state 
{
    struct hal_spi                  parent;
    struct spi_module               module;
    Sercom *                        hw;
    const struct samd21_spi_config *pconfig;
    uint8_t                         flags;
    
#define SAMD21_SPI_FLAG_ENABLED     (0x1)
#define SAMD21_SPI_FLAG_9BIT        (0x2)
};

static int samd21_spi_config(struct hal_spi *pspi, struct hal_spi_settings *psettings);
static int samd21_spi_transfer(struct hal_spi *psdi, uint16_t tx);  

const struct hal_spi_funcs 
samd21_spi_funcs = 
{
    .hspi_config = &samd21_spi_config,
    .hspi_master_transfer = &samd21_spi_transfer,
};

int
build_and_apply(struct samd21_spi_state *pspi, struct hal_spi_settings *psettings) 
{
    enum status_code rc;    
    struct spi_config cfg;
    uint32_t ctrla;
    spi_get_config_defaults(&cfg);
    
    if(pspi->flags & SAMD21_SPI_FLAG_ENABLED) {
        spi_disable(&pspi->module);    
        pspi->flags &= ~SAMD21_SPI_FLAG_ENABLED;
    }
    
    ctrla = (pspi->pconfig->dopo << SERCOM_SPI_CTRLA_DOPO_Pos) |
            (pspi->pconfig->dipo << SERCOM_SPI_CTRLA_DIPO_Pos);
    
    cfg.pinmux_pad0 = pspi->pconfig->pad0_pinmux;
    cfg.pinmux_pad1 = pspi->pconfig->pad1_pinmux;
    cfg.pinmux_pad2 = pspi->pconfig->pad2_pinmux;
    cfg.pinmux_pad3 = pspi->pconfig->pad3_pinmux;
    cfg.mux_setting = ctrla;

    /* apply the hal_settings */
    switch(psettings->word_size) 
    {
        case HAL_SPI_WORD_SIZE_8BIT:
            cfg.character_size = SPI_CHARACTER_SIZE_8BIT;
            pspi->flags &= ~SAMD21_SPI_FLAG_9BIT;
            break;
        case HAL_SPI_WORD_SIZE_9BIT:
            cfg.character_size = SPI_CHARACTER_SIZE_9BIT;
            pspi->flags |= SAMD21_SPI_FLAG_9BIT;
            break;
        default:
            return -1;
    }
    
    switch(psettings->data_order) 
    {
        case HAL_SPI_LSB_FIRST:
            cfg.data_order = SPI_DATA_ORDER_LSB;
            break;
        case HAL_SPI_MSB_FIRST:
            cfg.data_order = SPI_DATA_ORDER_MSB;
            break;
        default:
            return -2;
    }
    
    switch(psettings->data_mode) 
    {
        case HAL_SPI_MODE0:
            cfg.transfer_mode = SPI_TRANSFER_MODE_0;
            break;
        case HAL_SPI_MODE1:
            cfg.transfer_mode = SPI_TRANSFER_MODE_1;
            break;
        case HAL_SPI_MODE2:
            cfg.transfer_mode = SPI_TRANSFER_MODE_2;
            break;
        case HAL_SPI_MODE3:
            cfg.transfer_mode = SPI_TRANSFER_MODE_3;
            break;
        default:
            return -3;
    }
        
    cfg.mode_specific.master.baudrate = psettings->baudrate;   
    
    rc = spi_init(&pspi->module, pspi->hw, &cfg);
    if (STATUS_OK != rc) {
        return -4;
    }
    
    spi_enable(&pspi->module);
    pspi->flags |= SAMD21_SPI_FLAG_ENABLED;
    
    return 0;
}

struct hal_spi * 
samd21_spi_create(enum samd21_spi_device dev_id, const struct samd21_spi_config *pconfig) {
    struct samd21_spi_state *pspi = NULL;
        
    /* error check bsp config */
    switch(dev_id) {
        case SAMD21_SPI_SERCOM0:
        case SAMD21_SPI_SERCOM1:
        case SAMD21_SPI_SERCOM2:
        case SAMD21_SPI_SERCOM3:
        case SAMD21_SPI_SERCOM4:
        case SAMD21_SPI_SERCOM5:
            break;
        default:
            return NULL;
    }    
    
    switch(pconfig->dipo) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
        default:
            return NULL;
    }
    
    switch(pconfig->dopo) {
        case 0:
        case 1:
        case 2:
        case 3:
            break;
        default:
            return NULL;
    }
    
    /* NOTE only 16 different combinations of DIPO and DOPO are valid */
    {
        uint32_t val = (pconfig->dopo << SERCOM_SPI_CTRLA_DOPO_Pos) |
                        (pconfig->dipo << SERCOM_SPI_CTRLA_DIPO_Pos); 
        
        switch(val) {
            case SPI_SIGNAL_MUX_SETTING_A:
            case SPI_SIGNAL_MUX_SETTING_B:
            case SPI_SIGNAL_MUX_SETTING_C:
            case SPI_SIGNAL_MUX_SETTING_D:
            case SPI_SIGNAL_MUX_SETTING_E:
            case SPI_SIGNAL_MUX_SETTING_F:
            case SPI_SIGNAL_MUX_SETTING_G:
            case SPI_SIGNAL_MUX_SETTING_H:
            case SPI_SIGNAL_MUX_SETTING_I:
            case SPI_SIGNAL_MUX_SETTING_J:
            case SPI_SIGNAL_MUX_SETTING_K:
            case SPI_SIGNAL_MUX_SETTING_L:
            case SPI_SIGNAL_MUX_SETTING_M:
            case SPI_SIGNAL_MUX_SETTING_N:
            case SPI_SIGNAL_MUX_SETTING_O:
            case SPI_SIGNAL_MUX_SETTING_P:
                break;
            default:
                return NULL;                
        }
    }    
    
    pspi = calloc(1,sizeof(struct samd21_spi_state));
    
    if (pspi) {
        pspi->parent.driver_api = &samd21_spi_funcs;
        pspi->pconfig = pconfig;
        
        switch(dev_id) {
            case SAMD21_SPI_SERCOM0:
                pspi->hw = SERCOM0;
                break;
            case SAMD21_SPI_SERCOM1:
                pspi->hw = SERCOM1;
                break;
            case SAMD21_SPI_SERCOM2:
                pspi->hw = SERCOM2;
                break;
            case SAMD21_SPI_SERCOM3:
                pspi->hw = SERCOM3;
                break;
            case SAMD21_SPI_SERCOM4:
                pspi->hw = SERCOM4;
                break;
            case SAMD21_SPI_SERCOM5:
                pspi->hw = SERCOM5;
                break;
            default:
                assert(0); /* we just checked above */
        }    
    }
    
    return &pspi->parent;
}

int 
samd21_spi_config(struct hal_spi *pspi, struct hal_spi_settings *psettings)
{
    struct samd21_spi_state *psspi = (struct samd21_spi_state*) pspi;    
    if( (NULL == pspi) || pspi->driver_api != &samd21_spi_funcs) 
    {
        return -1;
    }
    
    return build_and_apply(psspi, psettings); 
}

int 
samd21_spi_transfer(struct hal_spi *pspi, uint16_t tx) 
{

    enum status_code rc;
    int read_val;
    
    struct samd21_spi_state *psspi = (struct samd21_spi_state*) pspi;
    
    if( (NULL == pspi) || pspi->driver_api != &samd21_spi_funcs) {
        return -1;
    }
    
    if(psspi->flags & SAMD21_SPI_FLAG_9BIT) {
        uint8_t tx_buf[2];
        uint8_t rx_buf[2];        
        tx_buf[1] = tx >> 8;
        tx_buf[0] = tx & 0xff;
        rc = spi_transceive_buffer_wait(&psspi->module, tx_buf, rx_buf, 2);
        read_val =  (int) rx_buf[0] << 8;
        read_val |= (int) rx_buf[1];
    } else {
        uint8_t tx_buf = tx;
        uint8_t rx_buf;
        rc = spi_transceive_buffer_wait(&psspi->module, &tx_buf, &rx_buf, 1);        
        read_val =  (int) rx_buf;
    }
        
    if(rc != STATUS_OK) {
        return -2;
    }
    return read_val;
}
