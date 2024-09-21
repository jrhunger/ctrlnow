#include "esp_wifi.h"
#include "esp_now.h"

#define MY_ESPNOW_WIFI_MODE WIFI_MODE_STA
#define MY_ESPNOW_WIFI_IF ESP_IF_WIFI_STA

// Destination MAC address
// The default address is the broadcast address, which will work out of the box, but the slave will assume every tx succeeds.
// Setting to the master's address will allow the slave to determine if sending succeeded or failed.
//   note: with default config, the master's WiFi driver will log this for you. eg. I (721) wifi:mode : sta (12:34:56:78:9a:bc)
// joystick-wired one: a0:76:4e:77:55:4c
// other one: 10:91:a8:38:0f:38
#define MY_RECEIVER_MAC {0x10, 0x91, 0xA8, 0x38, 0x0F, 0x38}

#define MY_ESPNOW_PMK "pmk1234567890123"
#define MY_ESPNOW_CHANNEL 1

// #define MY_ESPNOW_ENABLE_LONG_RANGE 1

#define MY_SLAVE_DEEP_SLEEP_TIME_MS 10000

// call once first to set everything up
void espnow_comm_init(void);

// call this every time data should be sent
void espnow_comm_send(int);
