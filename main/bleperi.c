

#include <string.h>
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "console/console.h"
#include "services/gap/ble_svc_gap.h"

#include <stdbool.h>
#include "nimble/ble.h"
#include "esp_peripheral.h"

#include "esp_log.h"

#include "gatt_dev.c"


int notify_period_ms = 1000;
static bool notify_status;
static uint16_t conn_handle;


static void notify_task(void *arg)
{
   int ret ;
   static uint8_t tval[2] = {0};
   struct os_mbuf *om;

   while(1) {

     if ( notify_status) {

	if ( tval[1] == 0xFF ) { tval[1] = 0; }
	else { tval[1]++ ; }

        om =  ble_hs_mbuf_from_flat(tval, sizeof(tval));
        ret = ble_gatts_notify_custom(conn_handle, dev_notify_value_handle, om);

        assert(ret == 0);
     }

     vTaskDelay(notify_period_ms / portTICK_PERIOD_MS );
   }
}

static void bleperi_on_reset(int reason)
{
}



static void ble_advertise(void);

static int bledev_gap_event(struct ble_gap_event *event, void *arg)
{
    struct ble_gap_conn_desc desc;
    int ret;


    switch (event->type) {

        case BLE_GAP_EVENT_CONNECT:
           printf("connect: %d\n", event->connect.status );
           if (event->connect.status == 0) {
             ret = ble_gap_conn_find(event->connect.conn_handle, &desc);
             assert(ret == 0);
             conn_handle = event->connect.conn_handle;
           }
           break;

        case BLE_GAP_EVENT_SUBSCRIBE:
           if (event->subscribe.attr_handle == dev_notify_value_handle) {
               notify_status = event->subscribe.cur_notify;
	       //notification is enabled by the notify_status;
           } else if (event->subscribe.attr_handle != dev_notify_value_handle) {
               notify_status = event->subscribe.cur_notify;
	       //notification is stopped by the notify_status;
           }
           printf("subscribe status %d\n", notify_status );

           break;

        case BLE_GAP_EVENT_DISCONNECT:
           printf("disconnected\n");
           ble_advertise();
           break;
    }

  return 0;
}



static uint8_t own_addr_type;

static void ble_advertise(void)
{

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int ret;

    memset(&fields, 0, sizeof fields);

    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    ret = ble_gap_adv_set_fields(&fields);
    if (ret != 0) {
	printf("error advertisement config %d\n", ret);
        return;
    }


    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ret = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, bledev_gap_event, NULL);
    if (ret != 0) {
	printf("error advertising %d\n", ret);
        return;
    }
}

static void bleperi_on_sync(void)
{
     int ret ;
     ret = ble_hs_util_ensure_addr(0);
     assert(ret == 0);

     //begin advertising
     ble_advertise();
}


void bleperi_host_task(void *param)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}



void bleperi_main(void)
{
    int ret;

    nimble_port_init();

    ble_hs_cfg.reset_cb = bleperi_on_reset;
    ble_hs_cfg.sync_cb = bleperi_on_sync;


    ret = gatt_dev_init();
    assert(ret == 0);

    //create a task to send notify values periodically
    xTaskCreate(notify_task, "notify_task", 2048, NULL, 10, NULL);

    ret = ble_svc_gap_device_name_set("mybledev");
    assert(ret == 0);


    nimble_port_freertos_init(bleperi_host_task);
}
