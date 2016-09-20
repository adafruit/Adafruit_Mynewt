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
#include <inttypes.h>
#include <hal/hal_dac_int.h>
#include <mcu/hal_dac.h>
#include <dac.h>
#include <stdlib.h>
#include <assert.h>

#define DAC_BITS    (10)
#define MAX_DAC_VAL ((1 <<DAC_BITS) - 1)

/* These functions make up the driver API for DAC devices.  All 
 * DAC devices with Mynewt-support implement this interface */
static int samd21_dac_write(struct hal_dac *pdac, int val);
static int samd21_dac_get_current(struct hal_dac *pdac);
static int samd21_dac_get_bits(struct hal_dac *pdac);
static int samd21_dac_disable(struct hal_dac *pdac);
static int samd21_dac_get_ref_mv(struct hal_dac *pdac);    

/* driver binding ti mynewt hal_dac_int.h */
const struct hal_dac_funcs samd21_dac_func = 
{
    .hdac_write = &samd21_dac_write,
    .hdac_current = &samd21_dac_get_current,
    .hdac_get_bits = &samd21_dac_get_bits,
    .hdac_get_ref_mv = &samd21_dac_get_ref_mv,
    .hdac_disable = &samd21_dac_disable,
};

/* internal state for a DAC driver */
struct samd21_dac 
{
    struct hal_dac                  parent;
    struct dac_module               dev_inst;    
    const struct samd21_dac_config* pconfig;    
    uint16_t                        value;
    uint8_t                         init;
    uint8_t                         enabled;
};

/* write the DAC value to the DAC.  For first time calls, this will
 * also enable the DAC */
static int
build_and_apply_config(struct samd21_dac *pdac) 
{           
    if (! pdac->init) {
        struct dac_config cfg;

        dac_get_config_defaults(&cfg);    
        
        /* override default */        
        switch (pdac->pconfig->reference) {
            case SAMD_DAC_REFERENCE_INT1V:
                cfg.reference = DAC_REFERENCE_INT1V;
                break;
            case SAMD_DAC_REFERENCE_AVCC:
                cfg.reference = DAC_REFERENCE_AVCC;
                break;
            case SAMD_DAC_REFERENCE_AREF:
                cfg.reference = DAC_REFERENCE_AREF;
                break;
            default:
                assert(0);
        }        

        /* there is only one DAC instance on this part */
        if (dac_init(&pdac->dev_inst, DAC, &cfg) != STATUS_OK) {
            return -1;
        }       

        pdac->init = 1;
    }
    
    /* write the DAC value */
    if (dac_chan_write(&pdac->dev_inst, 
                       DAC_CHANNEL_0, 
                       pdac->value) != STATUS_OK) {
        return -2;
    }
    
    /* if we are not enabled, enable it now */
    if (! pdac->enabled) {
        dac_enable(&pdac->dev_inst);
        pdac->enabled = 1;
    }
    
    return 0;
}

/* This creates a new DAC object for this DAC source */
struct hal_dac * 
samd21_dac_create(const struct samd21_dac_config *pconfig) 
{
    struct samd21_dac *psdac;
    
    /* just check for valid config values */
    switch(pconfig->reference) {
        case SAMD_DAC_REFERENCE_INT1V:
        case SAMD_DAC_REFERENCE_AVCC:
        case SAMD_DAC_REFERENCE_AREF:
            break;
        default:
            return NULL;
    }
    
    /* get zerod memory for this */
    psdac = calloc(1, sizeof(struct samd21_dac));
    
    /* bind to the driver and keep a pointer to config */
    if (NULL != psdac) {
        psdac->parent.driver_api = &samd21_dac_func;
        psdac->pconfig = pconfig;
        /* all other values should be zero */
    }
    return &psdac->parent;        
}

/* write an output value to the DAC specified in pdac.  */
int 
samd21_dac_write(struct hal_dac *pdac, int val)
{
    struct samd21_dac *psdac = (struct samd21_dac*) pdac;
    
    /* make sure we have a value hal_dac for us */
    if ((NULL == psdac) || (psdac->parent.driver_api != &samd21_dac_func)) {
        return -1;
    }
 
    if (val > MAX_DAC_VAL) {
        val = MAX_DAC_VAL;
    }
    
    psdac->value = val;    
    return build_and_apply_config(psdac);
}

/* return the current value written to the DAC */
int 
samd21_dac_get_current(struct hal_dac *pdac)
{
    struct samd21_dac *psdac = (struct samd21_dac*) pdac;
    
    /* make sure we have a value hal_dac for us */
    if ((NULL == psdac) || (psdac->parent.driver_api != &samd21_dac_func)) {
        return -1;
    }
    
    /* return the current value set in the DAC */
    return psdac->value; 
}

/* return the resolution of this DAC in bits */
int 
samd21_dac_get_bits(struct hal_dac *pdac)
{
    struct samd21_dac *psdac = (struct samd21_dac*) pdac;
    
    /* make sure we have a value hal_dac for us */
    if ((NULL == psdac) || (psdac->parent.driver_api != &samd21_dac_func)) {
        return -1;
    }
    
    return DAC_BITS; 
}

/* returns the value of the reference voltage for this dac in mv */
int 
samd21_dac_get_ref_mv(struct hal_dac *pdac)
{
    struct samd21_dac *psdac = (struct samd21_dac*) pdac;
    
    /* make sure we have a value hal_dac for us */
    if((NULL == psdac) || (psdac->parent.driver_api != &samd21_dac_func)) {
        return -1;
    }
    return psdac->pconfig->dac_reference_voltage_mvolts;     
}

/* disables the DAC channel */
int 
samd21_dac_disable(struct hal_dac *pdac) 
{
    struct samd21_dac *psdac = (struct samd21_dac*) pdac;
    
    /* make sure we have a value hal_dac for us */
    if ((NULL == psdac) || (psdac->parent.driver_api != &samd21_dac_func)) {
        return -1;
    }
    
    dac_disable(&psdac->dev_inst);
    return 0;
}
