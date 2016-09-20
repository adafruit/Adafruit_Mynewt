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

#include "hal/hal_i2c.h"
#include "hal/hal_i2c_int.h"
#include <assert.h>
#include <compiler.h>
#include "port.h"
#include <mcu/hal_i2c.h>
#include <i2c_master.h>
#include <mcu/samd21.h>

static int samd21_i2c_write_data(struct hal_i2c *pi2c, struct hal_i2c_master_data *ppkt);
static int samd21_i2c_read_data(struct hal_i2c *pi2c, struct hal_i2c_master_data *ppkt);
static int samd21_i2c_probe(struct hal_i2c *pi2c, uint8_t address);
static int samd21_i2c_stop(struct hal_i2c *pi2c);
static int samd21_i2c_start(struct hal_i2c *pi2c);

struct samd21_i2c_state
{
    struct hal_i2c                  parent;
    struct i2c_master_module        module;
    const struct samd21_i2c_config *pconfig;
};
const struct hal_i2c_funcs
samd21_i2c_funcs =
{
    .hi2cm_probe = &samd21_i2c_probe,
    .hi2cm_stop = &samd21_i2c_stop,
    .hi2cm_start = &samd21_i2c_start,
    .hi2cm_read_data = &samd21_i2c_read_data,
    .hi2cm_write_data = &samd21_i2c_write_data,
};

static int
build_and_apply(struct samd21_i2c_state *pi2c, Sercom *hw)
{
    enum status_code status;
    struct i2c_master_config cfg;

    i2c_master_get_config_defaults(&cfg);

    cfg.pinmux_pad0 = pi2c->pconfig->pad0_pinmux;
    cfg.pinmux_pad1 = pi2c->pconfig->pad1_pinmux;

    status = i2c_master_init(&pi2c->module,
                             hw,
                             &cfg);

    if(STATUS_OK == status) {
        i2c_master_enable(&pi2c->module);
        return 0;
    }

    return -1;
}

/* This creates a new I2C object based on the samd21 TC devices */
struct hal_i2c *
samd21_i2c_create(enum samd21_i2c_device dev_id,
                  const struct samd21_i2c_config *pconfig) {
    Sercom *hw;
    struct samd21_i2c_state *pi2c = NULL;

    /* validate device */
    switch(dev_id) {
        case SAMD21_I2C_SERCOM0:
        case SAMD21_I2C_SERCOM1:
        case SAMD21_I2C_SERCOM2:
        case SAMD21_I2C_SERCOM3:
        case SAMD21_I2C_SERCOM4:
        case SAMD21_I2C_SERCOM5:
            break;
        default:
            return NULL;
    }

    /* create the device */
    pi2c = calloc(1, sizeof(struct samd21_i2c_state));

    if(pi2c) {
        pi2c->parent.driver_api = &samd21_i2c_funcs;
        pi2c->pconfig = pconfig;

        switch(dev_id) {
            case SAMD21_I2C_SERCOM0:
                hw = SERCOM0;
                break;
            case SAMD21_I2C_SERCOM1:
                hw = SERCOM1;
                break;
            case SAMD21_I2C_SERCOM2:
                hw = SERCOM2;
                break;
            case SAMD21_I2C_SERCOM3:
                hw = SERCOM3;
                break;
            case SAMD21_I2C_SERCOM4:
                hw = SERCOM4;
                break;
            case SAMD21_I2C_SERCOM5:
                hw = SERCOM5;
                break;
            default:
                assert(0);
        }

        if(build_and_apply(pi2c, hw)) {
            free(pi2c);
            pi2c = NULL;
        }
    }

    return &pi2c->parent;
}

static int
samd21_i2c_write_data(struct hal_i2c *pi2c, struct hal_i2c_master_data *ppkt)
{
    int rc = -1;
    enum status_code status;

    struct samd21_i2c_state *psi2c = (struct samd21_i2c_state*) pi2c;
    if ((NULL == pi2c) || pi2c->driver_api != &samd21_i2c_funcs)
    {
        return rc;
    }

    struct i2c_master_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.address = ppkt->address;
    pkt.data_length = ppkt->len;
    pkt.data = ppkt->buffer;
    status = i2c_master_write_packet_wait_no_stop(&psi2c->module, &pkt);

    if (STATUS_OK == status) {
        rc = 0;
    }
    return rc;
}

static int
samd21_i2c_read_data(struct hal_i2c *pi2c, struct hal_i2c_master_data *ppkt)
{
    int rc = -1;
    enum status_code status;

    struct samd21_i2c_state *psi2c = (struct samd21_i2c_state*) pi2c;
    if ((NULL == pi2c) || pi2c->driver_api != &samd21_i2c_funcs)
    {
        return rc;
    }

    struct i2c_master_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.address = ppkt->address;
    pkt.data_length = ppkt->len;
    pkt.data = ppkt->buffer;
    status = i2c_master_read_packet_wait_no_stop(&psi2c->module, &pkt);

    if (STATUS_OK == status) {
        rc = 0;
    }
    return rc;
}

static int
samd21_i2c_stop(struct hal_i2c *pi2c)
{
    int rc = -1;

    struct samd21_i2c_state *psi2c = (struct samd21_i2c_state*) pi2c;
    if ((NULL == pi2c) || pi2c->driver_api != &samd21_i2c_funcs)
    {
        return rc;
    }
    
    i2c_master_send_stop(&psi2c->module);
    i2c_master_unlock(&psi2c->module);
    return 0;
}

static int
samd21_i2c_start(struct hal_i2c *pi2c)
{
    enum status_code status;

    struct samd21_i2c_state *psi2c = (struct samd21_i2c_state*) pi2c;
    if ((NULL == pi2c) || pi2c->driver_api != &samd21_i2c_funcs)
    {
        return -1;
    }
    
    status = i2c_master_lock(&psi2c->module);
    if ( STATUS_OK == status ) {
        return 0;
    } else {
        return -2;
    }
}

static int
samd21_i2c_probe(struct hal_i2c *pi2c, uint8_t address)
{
    int rc = -1;
    enum status_code status;

    struct samd21_i2c_state *psi2c = (struct samd21_i2c_state*) pi2c;
    if ((NULL == pi2c) || pi2c->driver_api != &samd21_i2c_funcs)
    {
        return rc;
    }

    status = i2c_master_lock(&psi2c->module);

    if ( STATUS_OK == status ) {
        uint8_t buf;
        struct i2c_master_packet pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.address = address;
        pkt.data_length = 0;
        pkt.data = &buf;
        status = i2c_master_read_packet_wait(&psi2c->module, &pkt);
    }
    i2c_master_unlock(&psi2c->module);

    if (STATUS_OK == status) {
        rc = 0;
    }

    return rc;
}