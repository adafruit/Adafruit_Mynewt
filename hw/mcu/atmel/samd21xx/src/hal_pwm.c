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

#include "hal/hal_pwm.h"
#include "hal/hal_pwm_int.h"
#include <assert.h>
#include <compiler.h>
#include "port.h"
#include <mcu/hal_pwm.h>
#include <tc.h>
#include <mcu/samd21.h>

static int samd_tc_pwm_off(struct hal_pwm *ppwm);
static int samd_tc_pwm_get_bits(struct hal_pwm *ppwm);
static int samd_tc_pwm_get_clk(struct hal_pwm *ppwm);


static int samd_tc_pwm_enable_duty(struct hal_pwm *ppwm, uint16_t frac_duty);

const struct hal_pwm_funcs samd21_tc_pwm_driver  = {
    .hpwm_disable = &samd_tc_pwm_off,
    .hpwm_get_bits = &samd_tc_pwm_get_bits,
    .hpwm_get_clk = &samd_tc_pwm_get_clk,
    .hpwm_ena_duty = &samd_tc_pwm_enable_duty,
    
}; 

#define CHANS_PER_TC    (2)
#define TC_RESOLUTION_BITS  (16)
#define TC_RESOLUTION_MAX   ((1 << TC_RESOLUTION_BITS) - 1)

struct samd_tc_pwm_channel;

/* this is the state of the PWM TC device */
struct samd_tc_pwm_device {
    struct tc_module module;    
    Tc              *tc;        /* Hardware pointer */
    uint16_t         counter;
    struct samd_tc_pwm_channel *pchannel[CHANS_PER_TC];
    const struct samd21_pwm_tc_config *pconfig;
};

/* each device has a few channels that can be controlled 
 * somewhat independently */
struct samd_tc_pwm_channel {
    struct hal_pwm   parent;
    uint8_t          channel;
    uint8_t          status;
    uint8_t          device;
    uint8_t          pin;
    uint16_t         cc;
};

/* some static RAM to hold the 3 device pointers instead of linking them
 * Seems like this is the best choices. If we add a singly linked list 
 * to the devices, it will take 1 pointer (list head) and then one
 * pointer in each one allocated. It'd be slightly better to link 
 * list these, but that adds some complexity.  For now, we will just
 * use an array */
static struct samd_tc_pwm_device * pdevices[3];

static int all_devices_off(struct samd_tc_pwm_device *ptcdev) {
    int i;
    for (i = 0; i < CHANS_PER_TC; i++) 
    {
        if ((NULL != ptcdev->pchannel[i])
                && (ptcdev->pchannel[i]->status)) {
            return 0;
        }
    }    
    return 1;
}

static int samd21_pwm_valid_cfg(const struct samd21_pwm_tc_config *pconfig) 
{
    /* just error checking config values */
    switch (pconfig->prescalar) 
    {
	case SAMD_TC_CLOCK_PRESCALER_DIV1:
	case SAMD_TC_CLOCK_PRESCALER_DIV2:
	case SAMD_TC_CLOCK_PRESCALER_DIV4:
	case SAMD_TC_CLOCK_PRESCALER_DIV8:
	case SAMD_TC_CLOCK_PRESCALER_DIV16:
	case SAMD_TC_CLOCK_PRESCALER_DIV64:
	case SAMD_TC_CLOCK_PRESCALER_DIV256:
	case SAMD_TC_CLOCK_PRESCALER_DIV1024:
            break;
       default:
            return 0;
    }            
    return 1;
}



static int 
samd21_tc_valid_input(enum samd_tc_device_id device_id, int channel, int pin) {
    
    switch(pin) 
    {
        case PIN_PA18E_TC3_WO0:
        case PIN_PA14E_TC3_WO0:
            if((device_id == SAMD_TC_DEV_3) && (channel == 0)) {
                return 1;
            }
            break;
        case PIN_PA19E_TC3_WO1:
        case PIN_PA15E_TC3_WO1:
            if((device_id == SAMD_TC_DEV_3) && (channel == 1)) {
                return 1;
            }
            break;
        case PIN_PA22E_TC4_WO0:
        case PIN_PB08E_TC4_WO0:
            if((device_id == SAMD_TC_DEV_4) && (channel == 0)) {
                return 1;
            }
            break;
        case PIN_PA23E_TC4_WO1:
        case PIN_PB09E_TC4_WO1:
            if((device_id == SAMD_TC_DEV_4) && (channel == 1)) {
                return 1;
            }
            break;
        case PIN_PA24E_TC5_WO0:
        case PIN_PB10E_TC5_WO0:
            if((device_id == SAMD_TC_DEV_5) && (channel == 0)) {
                return 1;
            }
            break;
            
        case PIN_PA25E_TC5_WO1:
        case PIN_PB11E_TC5_WO1:
            if((device_id == SAMD_TC_DEV_5) && (channel == 1)) {
                return 1;
            }
            break;
        default:
            break;
    }
    return 0;
}

/* This creates a new PWM object based on the samd21 TC devices */
struct hal_pwm * 
samd21_pwm_tc_create(enum samd_tc_device_id device_id, int channel, int pin,
                     const struct samd21_pwm_tc_config *pconfig) 
{
    struct samd_tc_pwm_channel *ppwm;
    int new_device = 0;

    if(0 == samd21_tc_valid_input(device_id, channel, pin)) {
        return NULL;
    }
    
    if (0 ==samd21_pwm_valid_cfg(pconfig)) 
    {
        return NULL;
    }
            
    ppwm = calloc(1, sizeof(struct samd_tc_pwm_channel));    
    if (NULL == ppwm) {
        return NULL;
    }
    
    /* initialize the channel */
    ppwm->channel = channel;
    ppwm->device = device_id;
    ppwm->parent.driver_api = &samd21_tc_pwm_driver;
    ppwm->pin = pin;    
    ppwm->cc = 0x8000;    
    ppwm->status = 0;
    
    /* do we already have a driver structure for the 
     * channel?  If so, link it in. if not create it*/    
    if (NULL == pdevices[device_id]) {
        pdevices[device_id] = calloc(1, sizeof(struct samd_tc_pwm_device));
        new_device = 1;
    }
   
    /* check again since calloc could have failed */
    if (NULL == pdevices[device_id]) {
        free(ppwm);
        return NULL;
    } 
       
    /* now we know that we have valid TC */
    
    if(new_device) {
        switch (device_id) 
        {
            case SAMD_TC_DEV_3:
                pdevices[device_id]->tc = TC3;
                break;
            case SAMD_TC_DEV_4:
                pdevices[device_id]->tc = TC4;
                break;
            case SAMD_TC_DEV_5:
                pdevices[device_id]->tc = TC5;
                break;
            default:
                assert(0);
        }    
        pdevices[device_id]->pconfig = pconfig;   
        pdevices[device_id]->counter = 0;
    }
        
    pdevices[device_id]->pchannel[channel] = ppwm;
    return &ppwm->parent;
}

int 
samd_tc_pwm_get_bits(struct hal_pwm *ppwm) {
    struct samd_tc_pwm_channel *ptcpwm = (struct samd_tc_pwm_channel*) ppwm;
    
    if (ptcpwm && (ptcpwm->parent.driver_api == &samd21_tc_pwm_driver)) 
    {
        return TC_RESOLUTION_BITS;
    }
    return -1;
}

int 
samd_tc_pwm_get_clk(struct hal_pwm *ppwm) {
    struct samd_tc_pwm_channel *ptcpwm = (struct samd_tc_pwm_channel*) ppwm;
    
    if (ptcpwm && (ptcpwm->parent.driver_api == &samd21_tc_pwm_driver))
    {
        return pdevices[ptcpwm->channel]->pconfig->clock_freq;
    }
    return -1;
}

static int 
samd_apply_and_enable(struct samd_tc_pwm_device *ptcdev) {
    enum status_code rc;    
    struct tc_config cfg;    
    int i;
    tc_get_config_defaults(&cfg);

    cfg.wave_generation = TC_WAVE_GENERATION_NORMAL_PWM_MODE;
        
    switch (ptcdev->pconfig->prescalar) 
    {
	case SAMD_TC_CLOCK_PRESCALER_DIV1:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV2:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV2;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV4:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV4;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV8:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV8;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV16:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV16;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV64:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV64;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV256:
            cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV256;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV1024:
             cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV1024;
            break;
       default:
            return -1;
    }    
    
    /* set to zero which is like its off */    
    for(i = 0; i < CHANS_PER_TC; i++) 
    {
        struct samd_tc_pwm_channel *pch = ptcdev->pchannel[i];
        
        if(pch) {
            cfg.pwm_channel[i].enabled = pch->status;
                cfg.counter_16_bit.compare_capture_channel[i] = pch->cc;
            
            /* all TCS are on MUX position ER */
            cfg.pwm_channel[i].pin_mux = PORT_PMUX_PMUXE_E_Val;
            cfg.pwm_channel[i].pin_out = pch->pin;            
        } else {
            cfg.pwm_channel[i].enabled = 0;
        }
    }     
    
    cfg.counter_16_bit.value = ptcdev->counter;
    
    /* for now, we will glitch one channel when we assign
     * the other. Hopefully its not noticeable since we only
     * do this when we turn it on */
    tc_reset(&ptcdev->module);
    rc = tc_init(&ptcdev->module, ptcdev->tc, &cfg);
    if(rc) {
        return -2;
    }
    
    tc_enable(&ptcdev->module);        
    return 0;
}

int 
samd_tc_pwm_off(struct hal_pwm *ppwm) {
    struct samd_tc_pwm_channel *ptcpwm = (struct samd_tc_pwm_channel*) ppwm;
    
    if (ptcpwm && (ptcpwm->parent.driver_api == &samd21_tc_pwm_driver))     
    {
        enum status_code rc;
        int chan = ptcpwm->channel;        
        struct samd_tc_pwm_device *ptcdev;

        ptcdev = pdevices[ptcpwm->device];
        assert(ptcdev);
                               
        ptcdev->pchannel[chan]->status = 0;
        ptcdev->pchannel[chan]->cc = TC_RESOLUTION_MAX/2;
                
        if(all_devices_off(ptcdev)) {
            rc = tc_reset(&ptcdev->module);
            if(rc) {
                return -2;
            }            
        }   
        return 0;
    }    
    return -1;
}

int 
samd_tc_pwm_enable_wave (struct hal_pwm *ppwm, uint32_t on) 
{
    struct samd_tc_pwm_channel *ptcpwm = (struct samd_tc_pwm_channel*) ppwm;
    int rc;
    
    if (ptcpwm && (ptcpwm->parent.driver_api == &samd21_tc_pwm_driver)) 
    {
        struct samd_tc_pwm_device *ptcdev;
        int chan = ptcpwm->channel;

        ptcdev = pdevices[ptcpwm->device];
        assert(ptcdev);

        /* set the on for this channel */
        ptcdev->pchannel[chan]->cc = on;
        
        /* turn this device on */
        if(ptcdev->pchannel[chan]->status) {
            ptcdev->tc->COUNT16.CC[chan].reg = on;            
        } else {
            ptcdev->pchannel[chan]->status = 1;                
            rc = samd_apply_and_enable(ptcdev);
            if(rc) {
                ptcdev->pchannel[chan]->status = 0;
                return -2;
            }            
        }   
        return 0;
    }
    return -1;
}

/* this only uses 16-bit timers so we are good */
int 
samd_tc_pwm_enable_duty(struct hal_pwm *ppwm, uint16_t frac_duty) 
{
    return samd_tc_pwm_enable_wave (ppwm, (uint32_t) frac_duty);
}

