#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espnow_comm.h"

void app_main(void)
{
    init_comms();
    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        send_comms(i);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}