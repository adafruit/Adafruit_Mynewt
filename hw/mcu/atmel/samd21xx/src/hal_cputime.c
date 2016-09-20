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

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "bsp/cmsis_nvic.h"
#include "hal/hal_cputime.h"

#include "sam0/drivers/tc/tc.h"
#include "sam0/utils/status_codes.h"
#include "common/utils/interrupt/interrupt_sam_nvic.h"


/* XXX:
 *  - Must determine how to set priority of cpu timer interrupt
 *  - Determine if we should use a mutex as opposed to disabling interrupts
 *  - Should I use macro for compare channel?
 *  - Sync to OSTIME.
 */

/* CPUTIME data */
struct cputime_data
{
    uint32_t ticks_per_usec;    /* number of ticks per usec */
    uint32_t cputime_high;      /* high word of 64-bit cpu time */
    struct tc_module timer;
};


struct cputime_data g_cputime;

/**
 * cputime init 
 *  
 * Initialize the cputime module. This must be called after os_init is called 
 * and before any other timer API are used. This should be called only once 
 * and should be called before the hardware timer is used. 
 * 
 * @param clock_freq The desired cputime frequency, in hertz (Hz).
 * 
 * @return int 0 on success; -1 on error.
 */
int
cputime_init(uint32_t clock_freq)
{
    enum status_code rc;
    struct system_gclk_gen_config gcfg;
    
    struct tc_config cfg;
    tc_get_config_defaults(&cfg);  
    
    if(clock_freq > 8000000) {
        return -1;
    }
    
    /* set up gclk generator 1 to source this timer */
    gcfg.division_factor = 1;
    gcfg.high_when_disabled = false;
    gcfg.output_enable = true;
    gcfg.run_in_standby = true;
    gcfg.source_clock = GCLK_SOURCE_OSC8M;    
    system_gclk_gen_set_config(GCLK_GENERATOR_1, &gcfg);
    
    cfg.counter_size = TC_COUNTER_SIZE_32BIT;
    cfg.clock_source = GCLK_GENERATOR_1;       
    cfg.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
    
    g_cputime.ticks_per_usec = 8;
    
    rc = tc_init(&g_cputime.timer, TC4, &cfg);
    
    tc_enable(&g_cputime.timer);   
    
    return rc;
}

/**
 * cputime get64
 *  
 * Returns cputime as a 64-bit number. 
 * 
 * @return uint64_t The 64-bit representation of cputime.
 */
uint64_t 
cputime_get64(void)
{
    uint32_t high;
    uint32_t low;
    uint64_t cpu_time;

    cpu_irq_enter_critical();
    high = g_cputime.cputime_high;
    low = cputime_get32();
    if (/* TODO overflow */ 0 ) {
        ++high;
        low = cputime_get32();
    }
    cpu_irq_leave_critical();

    cpu_time = ((uint64_t)high << 32) | low;

    return cpu_time;
}

/**
 * cputime get32 
 *  
 * Returns the low 32 bits of cputime. 
 * 
 * @return uint32_t The lower 32 bits of cputime
 */
uint32_t
cputime_get32(void)
{
    uint32_t cpu_time;

    cpu_time = tc_get_count_value(&g_cputime.timer);    

    return cpu_time;
}

/**
 * cputime nsecs to ticks 
 *  
 * Converts the given number of nanoseconds into cputime ticks. 
 * 
 * @param usecs The number of nanoseconds to convert to ticks
 * 
 * @return uint32_t The number of ticks corresponding to 'nsecs'
 */
uint32_t 
cputime_nsecs_to_ticks(uint32_t nsecs)
{
    uint32_t ticks;

    ticks = ((nsecs * g_cputime.ticks_per_usec) + 999) / 1000;
    return ticks;
}

/**
 * cputime ticks to nsecs
 *  
 * Convert the given number of ticks into nanoseconds. 
 * 
 * @param ticks The number of ticks to convert to nanoseconds.
 * 
 * @return uint32_t The number of nanoseconds corresponding to 'ticks'
 */
uint32_t 
cputime_ticks_to_nsecs(uint32_t ticks)
{
    uint32_t nsecs;

    nsecs = ((ticks * 1000) + (g_cputime.ticks_per_usec - 1)) / 
            g_cputime.ticks_per_usec;

    return nsecs;
}

/**
 * cputime usecs to ticks 
 *  
 * Converts the given number of microseconds into cputime ticks. 
 * 
 * @param usecs The number of microseconds to convert to ticks
 * 
 * @return uint32_t The number of ticks corresponding to 'usecs'
 */
uint32_t 
cputime_usecs_to_ticks(uint32_t usecs)
{
    uint32_t ticks;

    ticks = (usecs * g_cputime.ticks_per_usec);
    return ticks;
}

/**
 * cputime ticks to usecs
 *  
 * Convert the given number of ticks into microseconds. 
 * 
 * @param ticks The number of ticks to convert to microseconds.
 * 
 * @return uint32_t The number of microseconds corresponding to 'ticks'
 */
uint32_t 
cputime_ticks_to_usecs(uint32_t ticks)
{
    uint32_t us;

    us =  (ticks + (g_cputime.ticks_per_usec - 1)) / g_cputime.ticks_per_usec;
    return us;
}

/**
 * cputime delay ticks
 *  
 * Wait until the number of ticks has elapsed. This is a blocking delay. 
 * 
 * @param ticks The number of ticks to wait.
 */
void 
cputime_delay_ticks(uint32_t ticks)
{
    uint32_t until;

    until = cputime_get32() + ticks;
    while ((int32_t)(cputime_get32() - until) < 0) {
        /* Loop here till finished */
    }
}

/**
 * cputime delay nsecs 
 *  
 * Wait until 'nsecs' nanoseconds has elapsed. This is a blocking delay. 
 *  
 * @param nsecs The number of nanoseconds to wait.
 */
void 
cputime_delay_nsecs(uint32_t nsecs)
{
    uint32_t ticks;

    ticks = cputime_nsecs_to_ticks(nsecs);
    cputime_delay_ticks(ticks);
}

/**
 * cputime delay usecs 
 *  
 * Wait until 'usecs' microseconds has elapsed. This is a blocking delay. 
 *  
 * @param usecs The number of usecs to wait.
 */
void 
cputime_delay_usecs(uint32_t usecs)
{
    uint32_t ticks;

    ticks = cputime_usecs_to_ticks(usecs);
    cputime_delay_ticks(ticks);
}
