

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include <stdbool.h>
#include "nimble/ble.h"
#include "esp_peripheral.h"

#include "services/ans/ble_svc_ans.h"

static const ble_uuid128_t gatt_dev_svc_uuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

static const ble_uuid128_t gatt_dev_chr_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

static const ble_uuid128_t gatt_dev_dsc_uuid =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);



static uint16_t dev_notify_value_handle;


static int gatt_access(uint16_t conn_handle, uint16_t attr_handle,
               struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_dev_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_dev_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[])
        {
           {
                //subscription characteristic
                .uuid = &gatt_dev_chr_uuid.u,
                .access_cb = gatt_access,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &dev_notify_value_handle,
                .descriptors = (struct ble_gatt_dsc_def[])
                {
                    {
                      .uuid = &gatt_dev_dsc_uuid.u,
                      .att_flags = BLE_ATT_F_READ | BLE_ATT_F_WRITE,
                      .access_cb = gatt_access,
                    },
                    {
                      0, // end
                    }
                },
            },
            {
                0, //end
            }
        },
    },
    {
        0, //end
    },
};



static int gatt_access(uint16_t conn_handle, uint16_t attr_handle,
               struct ble_gatt_access_ctxt *ctxt, void *arg)
{

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        printf("called read chr\n");
        break ;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        printf("called write chr\n");
        break ;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        printf("called read dsc\n");
        break ;

    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        printf("called write dsc\n");
        break ;

    default:
        break ;
    }

    return BLE_ATT_ERR_UNLIKELY;
}



int gatt_dev_init(void)
{
    int ret;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();

    ret = ble_gatts_count_cfg(gatt_dev_svcs);
    if (ret != 0) return ret;

    ret = ble_gatts_add_svcs(gatt_dev_svcs);
    if (ret != 0) return ret;

  return 0;
}


