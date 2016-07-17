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
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "bsp/bsp.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_cputime.h"
#include "console/console.h"

/* BLE */
#include "nimble/ble.h"
#include "host/host_hci.h"
#include "host/ble_hs.h"
#include "host/ble_hs_adv.h"
#include "host/ble_uuid.h"
#include "host/ble_att.h"
#include "host/ble_gap.h"
#include "host/ble_gatt.h"
#include "host/ble_l2cap.h"
#include "controller/ble_ll.h"

#include "keystore.h"
#include "bluefruit_gatts/bluefruit_gatts.h"

uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;

/** Mbuf settings. */
#define MBUF_NUM_MBUFS      (12)
#define MBUF_BUF_SIZE       OS_ALIGN(BLE_MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + BLE_MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static os_membuf_t bleprph_mbuf_mpool_data[MBUF_MEMPOOL_SIZE];
struct os_mbuf_pool bleprph_mbuf_pool;
struct os_mempool bleprph_mbuf_mpool;

/** Log data. */
static struct log_handler bleprph_log_console_handler;
struct log bleprph_log;

/** Priority of the nimble host and controller tasks. */
#define BLE_LL_TASK_PRI             (OS_TASK_PRI_HIGHEST)

/** bleprph task settings. */
#define BLEPRPH_TASK_PRIO           1
#define BLEPRPH_STACK_SIZE          (OS_STACK_ALIGN(336))

struct os_eventq bleprph_evq;
struct os_task bleprph_task;
bssnz_t os_stack_t bleprph_stack[BLEPRPH_STACK_SIZE];

/** Our global device address (public) */
uint8_t g_dev_addr[BLE_DEV_ADDR_LEN] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};

/** Our random address (in case we need it) */
uint8_t g_random_addr[BLE_DEV_ADDR_LEN];

/** Device name - included in advertisements and exposed by GAP service. */
const char *bleprph_device_name = "nimble-bleprph";
static int bleprph_gap_event(int event, int status,
                             struct ble_gap_conn_ctxt *ctxt, void *arg);

/**
 * Utility function to log an array of bytes.
 */
static void
bleprph_print_bytes(uint8_t *bytes, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        BLEPRPH_LOG(INFO, "%s0x%02x", i != 0 ? ":" : "", bytes[i]);
    }
}

/**
 * Logs information about a connection to the console.
 */
static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    BLEPRPH_LOG(INFO, "handle=%d peer_addr_type=%d peer_addr=",
                desc->conn_handle,
                desc->peer_addr_type);
    bleprph_print_bytes(desc->peer_addr, 6);
    BLEPRPH_LOG(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                      "encrypted=%d authenticated=%d",
                desc->conn_itvl,
                desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.enc_enabled,
                desc->sec_state.authenticated);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
bleprph_advertise(void)
{
    struct ble_hs_adv_fields fields;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    memset(&fields, 0, sizeof fields);

    fields.tx_pwr_lvl_is_present = 1;
    fields.uuids128 = &((uint8_t[])BLEUART_SERVICE_UUID ) ;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 0;

//    fields.name = (uint8_t *)bleprph_device_name;
//    fields.name_len = strlen(bleprph_device_name);
//    fields.name_is_complete = 1;
//
//    fields.uuids16 = (uint16_t[]){ GATT_SVR_SVC_ALERT_UUID };
//    fields.num_uuids16 = 1;
//    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        BLEPRPH_LOG(ERROR, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    struct ble_hs_adv_fields rsp_fields =
    {
        .name = (uint8_t*) CFG_GAP_DEVICE_NAME,
        .name_len = strlen(CFG_GAP_DEVICE_NAME),
        .name_is_complete = 1
    };
    ble_gap_adv_rsp_set_fields(&rsp_fields);

    /* Begin advertising. */
    rc = ble_gap_adv_start(BLE_GAP_DISC_MODE_GEN, BLE_GAP_CONN_MODE_UND,
                           NULL, 0, NULL, bleprph_gap_event, NULL);
    if (rc != 0) {
        BLEPRPH_LOG(ERROR, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param status                The error code associated with the event
 *                                  (0 = success).
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int
bleprph_gap_event(int event, int status, struct ble_gap_conn_ctxt *ctxt,
                  void *arg)
{
    int authenticated;
    int rc;

    switch (event) {
    case BLE_GAP_EVENT_CONN:
        /* A new connection has been established or an existing one has been
         * terminated.
         */
        BLEPRPH_LOG(INFO, "connection %s; status=%d ",
                    status == 0 ? "up" : "down", status);
        bleprph_print_conn_desc(ctxt->desc);
        conn_handle = ctxt->desc->conn_handle;
        BLEPRPH_LOG(INFO, "\n");

        if (status != 0) {
            /* Connection terminated; resume advertising. */
            bleprph_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATED:
        /* The central has updated the connection parameters. */
        BLEPRPH_LOG(INFO, "connection updated; status=%d ", status);
        bleprph_print_conn_desc(ctxt->desc);
        BLEPRPH_LOG(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_LTK_REQUEST:
        /* An encryption procedure (bonding) is being attempted.  The nimble
         * stack is asking us to look in our key database for a long-term key
         * corresponding to the specified ediv and random number.
         */
        BLEPRPH_LOG(INFO, "looking up ltk with ediv=0x%02x rand=0x%llx\n",
                    ctxt->ltk_params->ediv, ctxt->ltk_params->rand_num);

        /* Perform a key lookup and populate the context object with the
         * result.  The nimble stack will use this key if this function returns
         * success.
         */
        rc = keystore_lookup(ctxt->ltk_params->ediv,
                             ctxt->ltk_params->rand_num, ctxt->ltk_params->ltk,
                             &authenticated);
        if (rc == 0) {
            ctxt->ltk_params->authenticated = authenticated;
            BLEPRPH_LOG(INFO, "ltk=");
            bleprph_print_bytes(ctxt->ltk_params->ltk,
                                sizeof ctxt->ltk_params->ltk);
            BLEPRPH_LOG(INFO, " authenticated=%d\n", authenticated);
        } else {
            BLEPRPH_LOG(INFO, "no matching ltk\n");
        }

        /* Indicate whether we were able to find an appropriate key. */
        return rc;

    case BLE_GAP_EVENT_KEY_EXCHANGE:
        /* The central is sending us key information or vice-versa.  If the
         * central is doing the sending, save the long-term key in the in-RAM
         * database.  This permits bonding to occur on subsequent connections
         * with this peer (as long as bleprph isn't restarted!).
         */
        if (ctxt->key_params->is_ours   &&
            ctxt->key_params->ltk_valid &&
            ctxt->key_params->ediv_rand_valid) {

            rc = keystore_add(ctxt->key_params->ediv,
                              ctxt->key_params->rand_val,
                              ctxt->key_params->ltk,
                              ctxt->desc->sec_state.authenticated);
            if (rc != 0) {
                BLEPRPH_LOG(INFO, "error persisting LTK; status=%d\n", rc);
            }
        }
        return 0;

    case BLE_GAP_EVENT_SECURITY:
        /* Encryption has been enabled or disabled for this connection. */
        BLEPRPH_LOG(INFO, "security event; status=%d ", status);
        bleprph_print_conn_desc(ctxt->desc);
        BLEPRPH_LOG(INFO, "\n");
        return 0;
    }

    return 0;
}

static int
bletiny_on_notify(uint16_t conn_handle, uint16_t attr_handle,
                  uint8_t *attr_val, uint16_t attr_len, void *arg)
{
    console_printf("received notification from conn_handle=%d attr=%d "
                   "len=%d value=", conn_handle, attr_handle, attr_len);

    bleprph_print_bytes(attr_val, attr_len);
    console_printf("\n");

    return 0;
}


/**
 * Event loop for the main bleprph task.
 */
static void bleprph_task_handler(void *unused)
{
    struct os_event *ev;
    struct os_callout_func *cf;
    int rc;

    rc = ble_hs_start();
    assert(rc == 0);

    ble_att_set_notify_cb(bletiny_on_notify, NULL);

    /* Begin advertising. */
    bleprph_advertise();

    while (1) {
        ev = os_eventq_get(&bleprph_evq);
        switch (ev->ev_type) {
        case OS_EVENT_T_TIMER:
            cf = (struct os_callout_func *)ev;
            assert(cf->cf_func);
            cf->cf_func(CF_ARG(cf));
            break;
        default:
            assert(0);
            break;
        }
    }
}

/* Task Blinky */
#define BLINKY_TASK_PRIO     10
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

void blinky_task_handler(void* arg)
{
  hal_gpio_init_out(LED_BLINK_PIN, 1);

  while(1)
  {
    os_time_delay(1000);

    hal_gpio_toggle(LED_BLINK_PIN);
  }
}

// BLEUART to UART bridge task
#define BLEUART_BRIDGE_TASK_PRIO     5
#define BLEUART_BRIDGE_STACK_SIZE    OS_STACK_ALIGN(256)

struct os_task bleuart_bridge_task;
os_stack_t bleuart_bridge_stack[BLEUART_BRIDGE_STACK_SIZE];

void bleuart_bridge_task_handler(void* arg)
{
  while(1)
  {
    int ch;

    // Get data from bleuart to hwuart
    if ( (ch = bf_gatts_bleuart_getc()) != EOF )
    {
      console_write( (char*)&ch, 1);
    }

    // Get data from hwuart to bleuart
    if ( console_read((char*)&ch, 1) )
    {
      bf_gatts_bleuart_putc( (char) ch);
    }

    os_time_delay(1);
  }
}


/**
 * main
 *
 * The main function for the project. This function initializes the os, calls
 * init_tasks to initialize tasks (and possibly other objects), then starts the
 * OS. We should not return from os start.
 *
 * @return int NOTE: this function should never return!
 */
int
main(void)
{
    struct ble_hs_cfg cfg;
    uint32_t seed;
    int rc;
    int i;

    /* Initialize OS */
    os_init();

    /* Set cputime to count at 1 usec increments */
    rc = cputime_init(1000000);
    assert(rc == 0);

    /* Seed random number generator with least significant bytes of device
     * address.
     */
    seed = 0;
    for (i = 0; i < 4; ++i) {
        seed |= g_dev_addr[i];
        seed <<= 8;
    }
    srand(seed);

    /* Initialize msys mbufs. */
    rc = os_mempool_init(&bleprph_mbuf_mpool, MBUF_NUM_MBUFS,
                         MBUF_MEMBLOCK_SIZE, bleprph_mbuf_mpool_data,
                         "bleprph_mbuf_data");
    assert(rc == 0);

    rc = os_mbuf_pool_init(&bleprph_mbuf_pool, &bleprph_mbuf_mpool,
                           MBUF_MEMBLOCK_SIZE, MBUF_NUM_MBUFS);
    assert(rc == 0);

    rc = os_msys_register(&bleprph_mbuf_pool);
    assert(rc == 0);

    /* Initialize the logging system. */
    log_init();
    log_console_handler_init(&bleprph_log_console_handler);
    log_register("bleprph", &bleprph_log, &bleprph_log_console_handler);

    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
                 BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

    os_task_init(&bleuart_bridge_task, "bleuart_bridge", bleuart_bridge_task_handler, NULL,
                 BLEUART_BRIDGE_TASK_PRIO, OS_WAIT_FOREVER, bleuart_bridge_stack, BLEUART_BRIDGE_STACK_SIZE);

    os_task_init(&bleprph_task, "bleprph", bleprph_task_handler,
                 NULL, BLEPRPH_TASK_PRIO, OS_WAIT_FOREVER,
                 bleprph_stack, BLEPRPH_STACK_SIZE);

    /* Initialize the BLE LL */
    rc = ble_ll_init(BLE_LL_TASK_PRI, MBUF_NUM_MBUFS, BLE_MBUF_PAYLOAD_SIZE);
    assert(rc == 0);

    /* Initialize the BLE host. */
    cfg = ble_hs_cfg_dflt;
    cfg.max_hci_bufs = 3;
    cfg.max_connections = 1;
    cfg.max_attrs = 42;
    cfg.max_services = 5;
    cfg.max_client_configs = 6;
    cfg.max_gattc_procs = 2;
    cfg.max_l2cap_chans = 3;
    cfg.max_l2cap_sig_procs = 1;
    cfg.sm_bonding = 1;
    cfg.sm_our_key_dist = BLE_L2CAP_SM_PAIR_KEY_DIST_ENC;
    cfg.sm_their_key_dist = BLE_L2CAP_SM_PAIR_KEY_DIST_ENC;

    /* Initialize eventq */
    os_eventq_init(&bleprph_evq);

    rc = ble_hs_init(&bleprph_evq, &cfg);
    assert(rc == 0);

    /* Initialize the console (for log output). */
    rc = console_init(NULL);
    assert(rc == 0);

    /* Register GATT attributes (services, characteristics, and
     * descriptors).
     */
    bf_gatts_init();

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return 0;
}
