#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"



#define CAN_TX_PRIO            8 
#define CAN_RX_PRIO            9 
#define CAN_CTRL_PRIO          10 
#define NMAX_MSGS              100
#define MESSAGE_ID             0x555 
#define NMAX_ITERS             3

#define TX_GPIO                10 
#define RX_GPIO                15



static void can_transmit_task(void *arg)
{
}

static void can_receive_task(void *arg)
{
}

static void can_control_task(void *arg)
{

}


void cantrans_main(void)
{

    const twai_general_config_t genconf = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO, 
		                                  RX_GPIO, TWAI_MODE_NO_ACK);
    const twai_timing_config_t  timeconf = TWAI_TIMING_CONFIG_25KBITS();

    const twai_filter_config_t filterconf = {.acceptance_code = (MESSAGE_ID << 21),
                                              .acceptance_mask = ~(TWAI_STD_ID_MASK << 21),
                                              .single_filter = true
                                             };

    xTaskCreatePinnedToCore(can_control_task, "can_ctrl", 4096, NULL, CAN_CTRL_PRIO, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(can_receive_task, "can_rx", 4096, NULL, CAN_RX_PRIO,    NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(can_transmit_task, "can_tx", 4096, NULL, CAN_TX_PRIO,   NULL, tskNO_AFFINITY);

    twai_driver_install(&genconf, &timeconf, &filterconf);
}
