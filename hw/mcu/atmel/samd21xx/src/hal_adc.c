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

#include "hal/hal_adc.h"
#include "hal/hal_adc_int.h"
#include <assert.h>
#include <compiler.h>
#include "port.h"
#include "adc.h"
#include "adc_feature.h"
#include <mcu/hal_adc.h>

/* ugh do we really need to store all this even if the user
 * only uses 1 ADC */
typedef struct samd21_adc_state {
    struct hal_adc                  parent;
    struct adc_module               module;    
    int                             pin;
    const struct samd21_adc_config  *pcfg;
} samd21_adc_state_t;



static int samd21_adc_get_bits(struct hal_adc *padc);
static int samd21_adc_get_refmv(struct hal_adc *padc);
static int samd21_adc_read(struct hal_adc *padc);

static const struct hal_adc_funcs samd21_adc_funcs = {
    .hadc_get_bits = &samd21_adc_get_bits,
    .hadc_get_ref_mv = &samd21_adc_get_refmv,
    .hadc_read = &samd21_adc_read,
};

const int chan_to_pin[SAMD21_ANALOG_MAX] = 
{
    ADC_INPUTCTRL_MUXPOS_PIN0,
    ADC_INPUTCTRL_MUXPOS_PIN1,
    ADC_INPUTCTRL_MUXPOS_PIN2,
    ADC_INPUTCTRL_MUXPOS_PIN3,
    ADC_INPUTCTRL_MUXPOS_PIN4,
    ADC_INPUTCTRL_MUXPOS_PIN5,
    ADC_INPUTCTRL_MUXPOS_PIN6,
    ADC_INPUTCTRL_MUXPOS_PIN7,
    ADC_INPUTCTRL_MUXPOS_PIN8,
    ADC_INPUTCTRL_MUXPOS_PIN9,
    ADC_INPUTCTRL_MUXPOS_PIN10,
    ADC_INPUTCTRL_MUXPOS_PIN11,
    ADC_INPUTCTRL_MUXPOS_PIN12,
    ADC_INPUTCTRL_MUXPOS_PIN13,
    ADC_INPUTCTRL_MUXPOS_PIN14,
    ADC_INPUTCTRL_MUXPOS_PIN15,
    ADC_INPUTCTRL_MUXPOS_PIN16,
    ADC_INPUTCTRL_MUXPOS_PIN17,
    ADC_INPUTCTRL_MUXPOS_PIN18,
    ADC_INPUTCTRL_MUXPOS_PIN19,
    ADC_INPUTCTRL_MUXPOS_BANDGAP,
    ADC_INPUTCTRL_MUXPOS_SCALEDCOREVCC,
    ADC_INPUTCTRL_MUXPOS_SCALEDIOVCC,
    ADC_INPUTCTRL_MUXPOS_DAC
};

static int samd21_channel_to_pin(int chan) {
    if(chan >= SAMD21_ANALOG_MAX) {
        return -1;
    }
    
    return chan_to_pin[chan];
}

struct hal_adc * 
samd21_adc_create(enum samd_adc_analog_channel chan, 
                  const struct samd21_adc_config *pconfig)
{   
    int pin;
    struct samd21_adc_state *padc;
    
    pin = samd21_channel_to_pin(chan);
    
    if(pin < 0) {
        return NULL;
    }
    
    padc = malloc(sizeof(struct samd21_adc_state)) ;
    
    if(NULL == padc) {
        return NULL;
    }

    padc->parent.driver_api = &samd21_adc_funcs;
    padc->pcfg = pconfig;
    padc->pin = pin;
    
    return &padc->parent;
}

static int
samd21_adc_build_and_apply(samd21_adc_state_t *pstate) {
    int rc = -1;
    struct adc_config cfg;
        
    /* get config defaults */    
    adc_get_config_defaults(&cfg);   
    
    switch(pstate->pcfg->volt) {
	case SAMD21_ADC_REFERENCE_INT1V:
            cfg.reference = ADC_REFCTRL_REFSEL_INT1V;
            break;
	case SAMD21_ADC_REFERENCE_INTVCC0:
            cfg.reference = ADC_REFCTRL_REFSEL_INTVCC0;
            break;
	case SAMD21_ADC_REFERENCE_INTVCC1:
            cfg.reference = ADC_REFCTRL_REFSEL_INTVCC1;
            break;
	case SAMD21_ADC_REFERENCE_AREFA:
            cfg.reference = ADC_REFCTRL_REFSEL_AREFA;
            break;
	case SAMD21_ADC_REFERENCE_AREFB:        
            cfg.reference = ADC_REFCTRL_REFSEL_AREFB;
            break;
        default:
            return rc;
    }
    
    switch(pstate->pcfg->resolution_bits) {
        case SAMD21_RESOLUTION_8_BITS:
            cfg.resolution = ADC_RESOLUTION_8BIT;
            break;
        case SAMD21_RESOLUTION_10_BITS:
            cfg.resolution = ADC_RESOLUTION_10BIT;
            break;
        case SAMD21_RESOLUTION_12_BITS:
            cfg.resolution = ADC_RESOLUTION_12BIT;
            break;
    }
    
    switch(pstate->pcfg->gain) {
        case SAMD21_GAIN_DIV2:
            cfg.gain_factor = ADC_GAIN_FACTOR_DIV2;
            break;
        case SAMD21_GAIN_1X:
            cfg.gain_factor = ADC_GAIN_FACTOR_1X;
            break;
        case SAMD21_GAIN_2X:
            cfg.gain_factor = ADC_GAIN_FACTOR_2X;
            break;
        case SAMD21_GAIN_4X:
            cfg.gain_factor = ADC_GAIN_FACTOR_4X;
            break;
        case SAMD21_GAIN_8X:
            cfg.gain_factor = ADC_GAIN_FACTOR_8X;
            break;
        case SAMD21_GAIN_16X:
            cfg.gain_factor = ADC_GAIN_FACTOR_16X;
            break;
    }
    
    cfg.positive_input = pstate->pin;
    
    rc = adc_init(&pstate->module, ADC, &cfg);
    
    return rc;
}

int 
samd21_adc_get_bits(struct hal_adc *padc) 
{
    samd21_adc_state_t *psadc = (samd21_adc_state_t *) padc;

    if(psadc && psadc->parent.driver_api == &samd21_adc_funcs) {
        switch(psadc->pcfg->resolution_bits) {
        case SAMD21_RESOLUTION_8_BITS:
            return 8;
        case SAMD21_RESOLUTION_10_BITS:
            return 10;
        case SAMD21_RESOLUTION_12_BITS:            
            return 12;
        }
    }
    return -1;
}

int 
samd21_adc_get_refmv(struct hal_adc *padc) 
{
    samd21_adc_state_t *psadc = (samd21_adc_state_t *) padc;

    if(psadc && psadc->parent.driver_api == &samd21_adc_funcs) {
        return psadc->pcfg->voltage_mvolts;
    }
    return -1;
}

int 
samd21_adc_read(struct hal_adc *padc) 
{
    samd21_adc_state_t *psadc = (samd21_adc_state_t *) padc;
    
    int rc;
    uint16_t result;
     
     /* TODO should take a lock since this can't have multiple conversions
      * at once */
     
    rc = samd21_adc_build_and_apply(psadc);
    if(rc != STATUS_OK) {
        return -1;
    }      
    
    rc = adc_enable(&psadc->module);
    if(rc != STATUS_OK) {
        return -2;
    }     
    
    adc_start_conversion(&psadc->module);
    
    while(adc_read(&psadc->module,&result) == STATUS_BUSY) {
        /* loop here bot not forever */
        /* TODO replace with interrupt and semaphore so we can block */
    }
    
    adc_disable(&psadc->module);     
    
     return result;
}


