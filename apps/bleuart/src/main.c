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
#include <console/console.h>
#include <shell/shell.h>
#include <log/log.h>
#include <imgmgr/imgmgr.h>

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
#include "host/ble_sm.h"
#include "controller/ble_ll.h"

/* Newtmgr include */
#include "newtmgr/newtmgr.h"
#include "nmgrble/newtmgr_ble.h"

/* RAM persistence layer. */
#include "store/ram/ble_store_ram.h"

/* Mandatory services. */
#include "services/mandatory/ble_svc_gap.h"
#include "services/mandatory/ble_svc_gatt.h"
#include "bluefruit_gatts/bluefruit_gatts.h"

uint16_t conn_handle = BLE_HS_CONN_HANDLE_NONE;

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
/** Mbuf settings. */
#define MBUF_NUM_MBUFS      (12)
#define MBUF_BUF_SIZE       OS_ALIGN(BLE_MBUF_PAYLOAD_SIZE, 4)
#define MBUF_MEMBLOCK_SIZE  (MBUF_BUF_SIZE + BLE_MBUF_MEMBLOCK_OVERHEAD)
#define MBUF_MEMPOOL_SIZE   OS_MEMPOOL_SIZE(MBUF_NUM_MBUFS, MBUF_MEMBLOCK_SIZE)

static os_membuf_t  mbuf_mpool_data[MBUF_MEMPOOL_SIZE];
struct os_mbuf_pool mbuf_pool;
struct os_mempool   mbuf_mpool;

/** Log data. */
static struct log_handler log_hdlr;
struct log mylog;

#define MAX_CBMEM_BUF 600
static uint32_t cbmem_buf[MAX_CBMEM_BUF];
struct cbmem cbmem;

//--------------------------------------------------------------------+
// TASK Settings
//--------------------------------------------------------------------+
/** Priority of the nimble host and controller tasks. */
#define BLE_LL_TASK_PRI             (OS_TASK_PRI_HIGHEST)

/** bleprph task settings. */
#define BLEPRPH_TASK_PRIO           1
#define BLEPRPH_STACK_SIZE          (OS_STACK_ALIGN(336))
struct os_eventq bleprph_evq;
struct os_task bleprph_task;
bssnz_t os_stack_t bleprph_stack[BLEPRPH_STACK_SIZE];

// shell task
#define SHELL_TASK_PRIO             (3)
#define SHELL_MAX_INPUT_LEN         (256)
#define SHELL_TASK_STACK_SIZE (OS_STACK_ALIGN(384))
os_stack_t shell_stack[SHELL_TASK_STACK_SIZE];

// netmgr task
#define NEWTMGR_TASK_PRIO            (4)
#define NEWTMGR_TASK_STACK_SIZE      (OS_STACK_ALIGN(1024))
os_stack_t newtmgr_stack[NEWTMGR_TASK_STACK_SIZE];

/* Task Blinky */
#define BLINKY_TASK_PRIO     10
#define BLINKY_STACK_SIZE    OS_STACK_ALIGN(128)

struct os_task blinky_task;
os_stack_t blinky_stack[BLINKY_STACK_SIZE];

// BLEUART to UART bridge task
#define BLEUART_BRIDGE_TASK_PRIO     5
#define BLEUART_BRIDGE_STACK_SIZE    OS_STACK_ALIGN(256)

struct os_task bleuart_bridge_task;
os_stack_t bleuart_bridge_stack[BLEUART_BRIDGE_STACK_SIZE];

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+
/** Our global device address (public) */
uint8_t g_dev_addr[BLE_DEV_ADDR_LEN] = {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a};

/** Our random address (in case we need it) */
uint8_t g_random_addr[BLE_DEV_ADDR_LEN];

static int bleprph_gap_event(struct ble_gap_event *event, void *arg);

void
print_addr(const void *addr)
{
    const uint8_t *u8p;

    u8p = addr;
    BLEPRPH_LOG(INFO, "%02x:%02x:%02x:%02x:%02x:%02x",
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

/**
 * Logs information about a connection to the console.
 */
static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    BLEPRPH_LOG(INFO, "handle=%d our_ota_addr_type=%d our_ota_addr=", desc->conn_handle, desc->our_ota_addr_type);
    print_addr(desc->our_ota_addr);

    BLEPRPH_LOG(INFO, " our_id_addr_type=%d our_id_addr=", desc->our_id_addr_type);
    print_addr(desc->our_id_addr);

    BLEPRPH_LOG(INFO, " peer_ota_addr_type=%d peer_ota_addr=", desc->peer_ota_addr_type);
    print_addr(desc->peer_ota_addr);

    BLEPRPH_LOG(INFO, " peer_id_addr_type=%d peer_id_addr=", desc->peer_id_addr_type);
    print_addr(desc->peer_id_addr);

    BLEPRPH_LOG(INFO, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void
bleprph_advertise(void)
{
    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof fields);

    /* Indicate that the flags field should be included; specify a value of 0
     * to instruct the stack to fill the value in for us.
     */
    fields.flags_is_present = 1;
    fields.flags = 0;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this one automatically as well.  This is done by assiging the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.uuids128 = &((uint8_t[])BLEUART_SERVICE_UUID ) ;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 0;

    ASSERT_STATUS_RETVOID( ble_gap_adv_set_fields(&fields) );

    //------------- Scan response data -------------//
    const char *name = ble_svc_gap_device_name();
    struct ble_hs_adv_fields rsp_fields =
    {
        .name = (uint8_t*) name,
        .name_len = strlen(name),
        .name_is_complete = 1
    };
    ble_gap_adv_rsp_set_fields(&rsp_fields);

    /* Begin advertising. */
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ASSERT_STATUS_RETVOID( ble_gap_adv_start(BLE_ADDR_TYPE_PUBLIC, 0, NULL, BLE_HS_FOREVER,
                           &adv_params, bleprph_gap_event, NULL) );
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
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
bleprph_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed. */
        BLEPRPH_LOG(INFO, "connection %s; status=%d ",
                       event->connect.status == 0 ? "established" : "failed",
                       event->connect.status);
        if (event->connect.status == 0) {
          ASSERT_STATUS( ble_gap_conn_find(event->connect.conn_handle, &desc) );
          bleprph_print_conn_desc(&desc);
          conn_handle = event->connect.conn_handle;
        }
        BLEPRPH_LOG(INFO, "\n");

        if (event->connect.status != 0) {
            /* Connection failed; resume advertising. */
            bleprph_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        BLEPRPH_LOG(INFO, "disconnect; reason=%d ", event->disconnect.reason);
        bleprph_print_conn_desc(&event->disconnect.conn);
        BLEPRPH_LOG(INFO, "\n");

        /* Connection terminated; resume advertising. */
        bleprph_advertise();
        return 0;

    case BLE_GAP_EVENT_CONN_UPDATE:
        /* The central has updated the connection parameters. */
        BLEPRPH_LOG(INFO, "connection updated; status=%d ",
                    event->conn_update.status);
        ASSERT_STATUS ( ble_gap_conn_find(event->connect.conn_handle, &desc) );
        bleprph_print_conn_desc(&desc);
        BLEPRPH_LOG(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_ENC_CHANGE:
        /* Encryption has been enabled or disabled for this connection. */
        BLEPRPH_LOG(INFO, "encryption change event; status=%d ",
                    event->enc_change.status);
        ASSERT_STATUS( ble_gap_conn_find(event->connect.conn_handle, &desc) );
        bleprph_print_conn_desc(&desc);
        BLEPRPH_LOG(INFO, "\n");
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        BLEPRPH_LOG(INFO, "subscribe event; conn_handle=%d attr_handle=%d "
                          "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                    event->subscribe.conn_handle,
                    event->subscribe.attr_handle,
                    event->subscribe.reason,
                    event->subscribe.prev_notify,
                    event->subscribe.cur_notify,
                    event->subscribe.prev_indicate,
                    event->subscribe.cur_indicate);
        return 0;
    }

    return 0;
}

/**
 * Event loop for the main bleprph task.
 */
static void
bleprph_task_handler(void *unused)
{
    struct os_event *ev;
    struct os_callout_func *cf;
    int rc;

    rc = ble_hs_start();
    assert(rc == 0);

    /* Begin advertising. */
    bleprph_advertise();

    while (1) {
        ev = os_eventq_get(&bleprph_evq);

        /* Check if the event is a nmgr ble mqueue event */
        rc = nmgr_ble_proc_mq_evt(ev);
        if (!rc) {
            continue;
        }

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

void blinky_task_handler(void* arg)
{
  hal_gpio_init_out(LED_BLINK_PIN, 1);

  while(1)
  {
    os_time_delay(1000);

    hal_gpio_toggle(LED_BLINK_PIN);
  }
}

void bleuart_bridge_task_handler(void* arg)
{
  // register 'nus' command to send BLEUART
  bf_gatts_bleuart_shell_register();

  while(1)
  {
    int ch;

    // Get data from bleuart to hwuart
    if ( (ch = bf_gatts_bleuart_getc()) != EOF )
    {
      console_write( (char*)&ch, 1);
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
int main(void)
{
    struct ble_hs_cfg cfg;
    uint32_t seed;
    int i;

    /* Initialize OS */
    os_init();

    /* Set cputime to count at 1 usec increments */
    ASSERT_STATUS( cputime_init(1000000) );

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
    ASSERT_STATUS( os_mempool_init(&mbuf_mpool, MBUF_NUM_MBUFS,
                                   MBUF_MEMBLOCK_SIZE, mbuf_mpool_data,
                                   "bleprph_mbuf_data") );
    ASSERT_STATUS( os_mbuf_pool_init(&mbuf_pool, &mbuf_mpool, MBUF_MEMBLOCK_SIZE, MBUF_NUM_MBUFS) );
    ASSERT_STATUS( os_msys_register(&mbuf_pool) );

    /* Initialize the logging system. */
    log_init();
//    log_console_handler_init(&log_hdlr);
    cbmem_init(&cbmem, cbmem_buf, MAX_CBMEM_BUF);
    log_cbmem_handler_init(&log_hdlr, &cbmem);
    log_register("bleprph", &mylog, &log_hdlr);


    //------------- Task Init -------------//
    shell_task_init(SHELL_TASK_PRIO, shell_stack, SHELL_TASK_STACK_SIZE, SHELL_MAX_INPUT_LEN);
    console_init(shell_console_rx_cb); // console_init(NULL);

    nmgr_task_init(NEWTMGR_TASK_PRIO, newtmgr_stack, NEWTMGR_TASK_STACK_SIZE);
    imgmgr_module_init();

    os_task_init(&blinky_task, "blinky", blinky_task_handler, NULL,
                 BLINKY_TASK_PRIO, OS_WAIT_FOREVER, blinky_stack, BLINKY_STACK_SIZE);

    os_task_init(&bleuart_bridge_task, "bleuart_bridge", bleuart_bridge_task_handler, NULL,
                 BLEUART_BRIDGE_TASK_PRIO, OS_WAIT_FOREVER, bleuart_bridge_stack, BLEUART_BRIDGE_STACK_SIZE);

    os_task_init(&bleprph_task, "bleprph", bleprph_task_handler, NULL,
                 BLEPRPH_TASK_PRIO, OS_WAIT_FOREVER, bleprph_stack, BLEPRPH_STACK_SIZE);

    /* Initialize the BLE LL */
    ASSERT_STATUS( ble_ll_init(BLE_LL_TASK_PRI, MBUF_NUM_MBUFS, BLE_MBUF_PAYLOAD_SIZE) );

    /* Initialize the BLE host. */
    cfg = ble_hs_cfg_dflt;
    cfg.max_hci_bufs = 3;
    cfg.max_connections = 1;
    cfg.max_gattc_procs = 2;
    cfg.max_l2cap_chans = 3;
    cfg.max_l2cap_sig_procs = 1;
    cfg.sm_bonding = 1;
    cfg.sm_our_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC;
    cfg.store_read_cb = ble_store_ram_read;
    cfg.store_write_cb = ble_store_ram_write;

    /* Populate config with the required GATT server settings. */
    cfg.max_attrs = 0;
    cfg.max_services = 0;
    cfg.max_client_configs = 0;

    /* GATT server initialization */
    ASSERT_STATUS( ble_svc_gap_init(&cfg) );
    ASSERT_STATUS( ble_svc_gatt_init(&cfg) );
    ASSERT_STATUS( nmgr_ble_gatt_svr_init(&bleprph_evq, &cfg) );
    ASSERT_STATUS( bf_gatts_init(&cfg) );

    /* Initialize eventq */
    os_eventq_init(&bleprph_evq);

    ASSERT_STATUS( ble_hs_init(&bleprph_evq, &cfg) );

    /* Set the default device name. */
    ASSERT_STATUS( ble_svc_gap_device_name_set(CFG_GAP_DEVICE_NAME) );

    /* Register GATT attributes (services, characteristics, and descriptors). */
    ASSERT_STATUS( ble_svc_gap_register() );
    ASSERT_STATUS( ble_svc_gatt_register() );
    ASSERT_STATUS( nmgr_ble_svc_register() );
    ASSERT_STATUS( bf_gatts_register() );

    /* Start the OS */
    os_start();

    /* os start should never return. If it does, this should be an error */
    assert(0);

    return 0;
}
