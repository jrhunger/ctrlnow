#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "espnow_comm.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// logging tag
static const char *TAG = "CtrlNow";

// game output values
#define OUTPUT_UP 0
#define OUTPUT_RIGHT 1
#define OUTPUT_DOWN 2
#define OUTPUT_LEFT 3
#define OUTPUT_FIRE 4

// game input GPIOs
#define GPIO_DOWN 0
#define GPIO_RIGHT 1
#define GPIO_FIRE 2
#define GPIO_UP 3
#define GPIO_LEFT 10

// lookup table of GPIOs to corresponding output value
static int map_gpio_to_output[11] = {99,99,99,99,99,99,99,99,99,99,99};

// Queue for inputs
static QueueHandle_t gpio_evt_queue = NULL;

// ISR handler needs to be short and sweet
static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static int64_t now = 0;
static int64_t last = 0;
#define THRESHOLD 300000
// process events from queue
static void gpio_task(void* arg) {
    uint32_t gpio_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY)) {
            now = esp_timer_get_time();
            // re-read the GPIO to make sure it is low. 
            // seeing crosstalk bounces between buttons, but on read the undesired one is high
            if (gpio_get_level(gpio_num) != 0) {
                ESP_LOGI(TAG, "nonzero ignore GPIO %ld", gpio_num);
                continue;
            }
            if (now - last > THRESHOLD) {
                last = now;
                espnow_comm_send(map_gpio_to_output[gpio_num]);
            }
            //else {
            //    ESP_LOGI(TAG, "ignored GPIO %ld - too late", gpio_num);
            //}
        }
    }
}

static void init_inputs(void) {
    // init gpio to output map
    map_gpio_to_output[GPIO_DOWN] = OUTPUT_DOWN;
    map_gpio_to_output[GPIO_UP] = OUTPUT_UP;
    map_gpio_to_output[GPIO_LEFT] = OUTPUT_LEFT;
    map_gpio_to_output[GPIO_RIGHT] = OUTPUT_RIGHT;
    map_gpio_to_output[GPIO_FIRE] = OUTPUT_FIRE;

    ESP_LOGI(TAG, "enable GPIO inputs");
    gpio_pullup_en(GPIO_UP);
    gpio_set_direction(GPIO_UP, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_UP, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_UP);

    gpio_pullup_en(GPIO_DOWN);
    gpio_set_direction(GPIO_DOWN, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_DOWN, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_DOWN);

    gpio_pullup_en(GPIO_LEFT);
    gpio_set_direction(GPIO_LEFT, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_LEFT, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_LEFT);

    gpio_pullup_en(GPIO_RIGHT);
    gpio_set_direction(GPIO_RIGHT, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_RIGHT, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_RIGHT);

    gpio_pullup_en(GPIO_FIRE);
    gpio_set_direction(GPIO_FIRE, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_FIRE, GPIO_INTR_NEGEDGE);
    gpio_intr_enable(GPIO_FIRE);

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
    gpio_isr_handler_add(GPIO_FIRE, gpio_isr_handler, (void*) GPIO_FIRE);
}

void app_main(void)
{
    espnow_comm_init();
    init_inputs();
}