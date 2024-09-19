#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espnow_comm.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// logging tag
static const char *TAG = "CtrlNow";

// game input GPIOs
#define GPIO_UP 3
#define GPIO_DOWN 0
#define GPIO_LEFT 10
#define GPIO_RIGHT 1

// Queue for inputs
static QueueHandle_t gpio_evt_queue = NULL;

// ISR handler needs to be short and sweet
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

int64_t now = 0;
int64_t last = 0;
#define THRESHOLD 300000
// process events from queue
static void gpio_task(void* arg) {
    uint32_t gpio_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY)) {
            now = esp_timer_get_time();
            if (now - last > THRESHOLD) {
                last = now;
                send_comms(gpio_num);
            }
        }
    }
}

static void init_inputs(void) {
    ESP_LOGI(TAG, "enable GPIO inputs");
    // 3 = up
    gpio_pullup_en(GPIO_UP);
    gpio_set_direction(GPIO_UP, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_UP, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_UP);
    // 0 = down
    gpio_pullup_en(GPIO_DOWN);
    gpio_set_direction(GPIO_DOWN, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_DOWN, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_DOWN);
    // 10 = left
    gpio_pullup_en(GPIO_LEFT);
    gpio_set_direction(GPIO_LEFT, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_LEFT, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_LEFT);
    // 1 = right
    gpio_pullup_en(GPIO_RIGHT);
    gpio_set_direction(GPIO_RIGHT, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_RIGHT, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_RIGHT);

    ESP_LOGI(TAG, "add GPIO isr service");
    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0); // TODO - find out what the 0 means
    //hook isr handler for specific gpio pins
    gpio_isr_handler_add(GPIO_UP, gpio_isr_handler, (void*) GPIO_UP);
    gpio_isr_handler_add(GPIO_DOWN, gpio_isr_handler, (void*) GPIO_DOWN);
    gpio_isr_handler_add(GPIO_LEFT, gpio_isr_handler, (void*) GPIO_LEFT);
    gpio_isr_handler_add(GPIO_RIGHT, gpio_isr_handler, (void*) GPIO_RIGHT);
}

void app_main(void)
{
    init_comms();
    init_inputs();
    /*
    int i = 0;
    while (1) {
        printf("[%d] Hello world!\n", i);
        i++;
        send_comms(i);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
    */
}