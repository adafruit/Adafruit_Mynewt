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
#include <tcc.h>
#include <mcu/samd21.h>

static int samd_tcc_pwm_off(struct hal_pwm *ppwm);
static int samd_tcc_pwm_get_bits(struct hal_pwm *ppwm);
static int samd_tcc_pwm_get_clk(struct hal_pwm *ppwm);
static int samd_tcc_pwm_set_freq(struct hal_pwm *ppwm, uint32_t freq_hz);
static int samd_tcc_pwm_enable_duty(struct hal_pwm *ppwm, uint16_t frac_duty);

const struct hal_pwm_funcs 
samd21_tcc_pwm_driver  = {
    .hpwm_disable = &samd_tcc_pwm_off,
    .hpwm_get_bits = &samd_tcc_pwm_get_bits,
    .hpwm_get_clk = &samd_tcc_pwm_get_clk,
    .hpwm_ena_duty = &samd_tcc_pwm_enable_duty,
    .hpwm_set_freq = &samd_tcc_pwm_set_freq,    
}; 

/* a second type of capture controller */
/* not all devices have 8 channels */
#define CHANS_PER_TCC    (8)

struct samd_tcc_pwm_channel;

/* this is the state of the PWM TC device */
struct samd_tcc_pwm_device 
{
    struct tcc_module                   module;    
    Tcc                                *tcc;        /* Hardware pointer */
    uint32_t                            period;
    struct samd_tcc_pwm_channel        *pchannel[CHANS_PER_TCC];
    const struct samd21_pwm_tcc_config *pconfig;
};

typedef struct tcc_port_entry_s 
{
    uint8_t device;
    uint8_t channel;
    uint8_t pin;
    uint8_t mux;
} tcc_port_entry_t;

/* each device has a few channels that can be controlled 
 * somewhat independently */
struct samd_tcc_pwm_channel 
{
    struct hal_pwm   parent;
    tcc_port_entry_t port;
    uint8_t          status;
    uint8_t          _reserved;  
    uint16_t         duty;
};

struct samd_tcc_pwm_device *pdevices[3];

static int all_devices_off(struct samd_tcc_pwm_device *ptccdev) 
{
    int i;
    for (i = 0; i < CHANS_PER_TCC; i++) {
        if ((NULL != ptccdev->pchannel[i]) && (ptccdev->pchannel[i]->status)) {
            return 0;
        }
    }    
    return 1;
}

const static tcc_port_entry_t tcc_valid[] = 
{
    {SAMD_TCC_DEV_2,0,PIN_PA00E_TCC2_WO0,PORT_PMUX_PMUXE_E_Val},    
    {SAMD_TCC_DEV_2,1,PIN_PA01E_TCC2_WO1,PORT_PMUX_PMUXE_E_Val},  
    {SAMD_TCC_DEV_0,0,PIN_PA04E_TCC0_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,1,PIN_PA05E_TCC0_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_1,0,PIN_PA06E_TCC1_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_1,1,PIN_PA07E_TCC1_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,0,PIN_PA08E_TCC0_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_1,2,PIN_PA08F_TCC1_WO2,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,1,PIN_PA09E_TCC0_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_1,3,PIN_PA09F_TCC1_WO3,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_1,0,PIN_PA10E_TCC1_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,2,PIN_PA10F_TCC0_WO2,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_1,1,PIN_PA11E_TCC1_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,3,PIN_PA11F_TCC0_WO3,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_2,0,PIN_PA12E_TCC2_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,6,PIN_PA12F_TCC0_WO6,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_2,1,PIN_PA13E_TCC2_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,7,PIN_PA13F_TCC0_WO7,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,4,PIN_PA14F_TCC0_WO4,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,5,PIN_PA15F_TCC0_WO5,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_2,0,PIN_PA16E_TCC2_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,6,PIN_PA16F_TCC0_WO6,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_2,1,PIN_PA17E_TCC2_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,7,PIN_PA17F_TCC0_WO7,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,2,PIN_PA18F_TCC0_WO2,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,3,PIN_PA19F_TCC0_WO3,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,6,PIN_PA20F_TCC0_WO6,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,7,PIN_PA21F_TCC0_WO7,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,4,PIN_PA22F_TCC0_WO4,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,5,PIN_PA23F_TCC0_WO5,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_1,2,PIN_PA24F_TCC1_WO2,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_1,3,PIN_PA25F_TCC1_WO3,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_1,0,PIN_PA30E_TCC1_WO0,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_1,1,PIN_PA31E_TCC1_WO1,PORT_PMUX_PMUXE_E_Val},
    {SAMD_TCC_DEV_0,5,PIN_PB11F_TCC0_WO5,PORT_PMUX_PMUXE_F_Val},
    {SAMD_TCC_DEV_0,4,PIN_PB10F_TCC0_WO4,PORT_PMUX_PMUXE_F_Val},
};

#define TCC_RESOLUTION_BITS     (14)

/* what combinations are valid on the SamD21*/
static int 
samd21_tcc_valid_input(enum samd_tcc_device_id device_id, int channel, int pin) 
{
    int i;
    for (i = 0; i < sizeof(tcc_valid)/sizeof(tcc_port_entry_t); i++) {
        if ((tcc_valid[i].channel == channel) 
              && (tcc_valid[i].device == device_id)
              && (tcc_valid[i].pin == pin)) {
            return 1;
        }                
    }
    return 0;
}

static int mux_value(enum samd_tcc_device_id device_id, int channel, int pin) 
{
    int i;
    for (i = 0; i < sizeof(tcc_valid)/sizeof(tcc_port_entry_t); i++) {
        if ((tcc_valid[i].channel == channel) 
              && (tcc_valid[i].device == device_id)
              && (tcc_valid[i].pin == pin)) {
            return tcc_valid[i].mux;
        }                
    }
    assert(0);     
}

static int samd21_tcc_valid_config(const struct samd21_pwm_tcc_config *pconfig) 
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
            return 1;
       default:
            return 0;
    }        
}

static uint32_t 
duty_to_waveform(struct samd_tcc_pwm_device *ptccdev, uint16_t duty) 
{    
    uint64_t val = ptccdev->period;    
    val *= duty;
    val >>= 16;    
    
    if(val > ptccdev->period - 1) {
        val = ptccdev->period - 1;
    }
    return val;
}

static int 
samd_apply_and_enable(struct samd_tcc_pwm_device *ptccdev) 
{
    struct tcc_config cfg;    
    int i;
    int rc;
    tcc_get_config_defaults(&cfg, ptccdev->tcc);
    
    switch (ptccdev->pconfig->prescalar) 
    {
	case SAMD_TC_CLOCK_PRESCALER_DIV1:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV2:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV2;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV4:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV4;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV8:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV8;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV16:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV16;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV64:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV64;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV256:
            cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV256;
            break;
	case SAMD_TC_CLOCK_PRESCALER_DIV1024:
             cfg.counter.clock_prescaler = TCC_CLOCK_PRESCALER_DIV1024;
            break;
       default:
            return -1;
    }       
        
    /* set to zero which is like its off */    
    for (i = 0; i < CHANS_PER_TCC; i++) 
    {
        struct samd_tcc_pwm_channel *pch = ptccdev->pchannel[i];
        if (pch) {
            cfg.pins.enable_wave_out_pin[i] = pch->status;    
            cfg.compare.match[i] = duty_to_waveform(ptccdev, pch->duty);
            cfg.pins.wave_out_pin_mux[i] = pch->port.mux;
            cfg.pins.wave_out_pin[i] = pch->port.pin;            
        } else {
            cfg.pins.enable_wave_out_pin[i] = 0;
        }
    }    
    
    cfg.compare.wave_generation = TCC_WAVE_GENERATION_SINGLE_SLOPE_PWM;
    cfg.counter.period = ptccdev->period - 1;    

    /* for now, we may glitch one channel when we assign
     * the other. Hopefully its not noticeable since we only
     * do this when we turn it on, not when we change its duty cycle  */
    tcc_reset(&ptccdev->module);
    rc = tcc_init(&ptccdev->module, ptccdev->tcc, &cfg);
    if (rc) {
        return -2;
    }
    
    tcc_enable(&ptccdev->module);        
    return 0;
    
}

static int
get_tcc_max_resolution_bits(int device_id) {
    switch(device_id) {
        case SAMD_TCC_DEV_0:
            return 24;
        case SAMD_TCC_DEV_1:
            return 24;
        case SAMD_TCC_DEV_2:
            return 16;
    }
    assert(0);
}

static Tcc*
get_tcc_hardware(int device_id) {
    switch (device_id) 
    {
        case SAMD_TCC_DEV_0:
            return TCC0;
        case SAMD_TCC_DEV_1:
            return TCC1;
        case SAMD_TCC_DEV_2:
            return TCC2;
    }        
    assert(0);
}


struct hal_pwm * 
samd21_pwm_tcc_create(enum samd_tcc_device_id device_id, int channel, int pin,
                        const struct samd21_pwm_tcc_config *pconfig) 
{
    struct samd_tcc_pwm_channel *ppwm;
    int new_device;
    
    if (0 == samd21_tcc_valid_input(device_id, channel, pin)) {
        return NULL;        
    }
    
    if (0 == samd21_tcc_valid_config(pconfig)) {
        return NULL;
    }
    
    ppwm = calloc(1, sizeof(struct samd_tcc_pwm_channel));    
    if (NULL == ppwm) {
        return NULL;
    }
    
    /* initialize the port */
    ppwm->port.channel = channel;
    ppwm->port.device = device_id;
    ppwm->port.pin = pin;    
    ppwm->port.mux =  mux_value(device_id, channel, pin);  
    
    /* do we already have a driver structure for the 
     * channel?  If so, link it in. if not create it*/    
    if (NULL == pdevices[device_id]) {
        pdevices[device_id] = calloc(1, sizeof(struct samd_tcc_pwm_device));
        new_device = 1;
    }
   
    /* check again since calloc could have failed */
    if (NULL == pdevices[device_id]) {
        free(ppwm);
        return NULL;
    } 
       
    /* now we know that we have valid TCC. First time setup */    
    if (new_device) {
        pdevices[device_id]->tcc = get_tcc_hardware(device_id);
        pdevices[device_id]->period = 1 << get_tcc_max_resolution_bits(device_id);
        pdevices[device_id]->pconfig = pconfig;  
    }
        
    /* initialize the channel */
    ppwm->parent.driver_api = &samd21_tcc_pwm_driver;     
    ppwm->duty = 0x8000;
    ppwm->status = 0;      
    
    pdevices[device_id]->pchannel[channel] = ppwm;
    return &ppwm->parent;    
}

int 
samd_tcc_pwm_get_bits(struct hal_pwm *ppwm)
{
    struct samd_tcc_pwm_channel *ptccpwm = (struct samd_tcc_pwm_channel*) ppwm;
    
    if (ptccpwm && (ptccpwm->parent.driver_api == &samd21_tcc_pwm_driver)) {
        return get_tcc_max_resolution_bits(ptccpwm->port.device);
    }
    return -1;
}

int 
samd_tcc_pwm_get_clk(struct hal_pwm *ppwm)
{
    struct samd_tcc_pwm_channel *ptccpwm = (struct samd_tcc_pwm_channel*) ppwm;
    
    if (ptccpwm && (ptccpwm->parent.driver_api == &samd21_tcc_pwm_driver)) {
        return pdevices[ptccpwm->port.device]->pconfig->clock_freq;
    }
    return -1;}

int 
samd_tcc_pwm_off(struct hal_pwm *ppwm) 
{
    struct samd_tcc_pwm_channel *ptccpwm = (struct samd_tcc_pwm_channel*) ppwm;
    
    if (ptccpwm && (ptccpwm->parent.driver_api == &samd21_tcc_pwm_driver)) {
        int chan = ptccpwm->port.channel;        
        struct samd_tcc_pwm_device *ptccdev;

        ptccdev = pdevices[ptccpwm->port.device];
        assert(ptccdev);
                               
        ptccdev->pchannel[chan]->status = 0;
        ptccdev->pchannel[chan]->duty = 0;
        if (all_devices_off(ptccdev)) {
            tcc_reset(&ptccdev->module);
        }                        
        return 0;
    }    
    return -1;
}

static int samd_tcc_pwm_set_freq(struct hal_pwm *ppwm, uint32_t freq_hz) {
    /* we know the clock frequency in Hz */
    struct samd_tcc_pwm_channel *ptccpwm = (struct samd_tcc_pwm_channel*) ppwm;
    
    if (ptccpwm && (ptccpwm->parent.driver_api == &samd21_tcc_pwm_driver)) {
        struct samd_tcc_pwm_device *ptccdev;
        uint32_t period;
        int resolution_bits = get_tcc_max_resolution_bits(ptccpwm->port.device);
        uint32_t clk_freq;
        
        ptccdev = pdevices[ptccpwm->port.device];    
        
        clk_freq = samd_tcc_pwm_get_clk(ppwm);
        assert(clk_freq);
        
        /* no point working on something larger than we can do. In
         * order to get a "wave" we nee to have two bits per period */
        if(freq_hz > clk_freq/2 ) {
            return -2;
        }
        
        /* is the frequency too low */
        if(freq_hz <= (clk_freq >> resolution_bits)) {
            return -3;
        }
        
        /* convert from a frequency to a period value. Rounding */
        period = (clk_freq + freq_hz/2)/freq_hz;        
        ptccdev->period = period;
        
        if (!all_devices_off(ptccdev)) {
            int rc;
            rc = samd_apply_and_enable(ptccdev);
            if (rc) {
                return -2;
            }
        }
        return 0;
    }    
    
    return -1;
}

int 
samd_tcc_pwm_enable_duty (struct hal_pwm *ppwm, uint16_t duty) 
{
    struct samd_tcc_pwm_channel *ptccpwm = (struct samd_tcc_pwm_channel*) ppwm;
    int rc;
    
    if (ptccpwm && (ptccpwm->parent.driver_api == &samd21_tcc_pwm_driver)) {
        struct samd_tcc_pwm_device *ptccdev;
        int chan = ptccpwm->port.channel;

        ptccdev = pdevices[ptccpwm->port.device];
        assert(ptccdev);

        /* set the on for this channel */
        ptccdev->pchannel[chan]->duty = duty;
        
        /* turn this device on */
        if (ptccdev->pchannel[chan]->status) {
            uint32_t on = duty_to_waveform(ptccdev, duty);
            rc = tcc_set_compare_value(&ptccdev->module, chan, on);
            if (rc) {
                ptccdev->pchannel[chan]->status = 0;
                return -3;
            }
        } else {
            ptccdev->pchannel[chan]->status = 1;                
            rc = samd_apply_and_enable(ptccdev);
            if (rc) {
                ptccdev->pchannel[chan]->status = 0;
                return -2;
            }            
        }   
        return 0;
    }
    return -1;
}
