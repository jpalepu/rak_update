#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"

#define UART_PORT UART_NUM_1
#define TX_PIN 17
#define RX_PIN 16
#define BAUD_RATE 115200

static const char *TAG = "RAK3172_FirmwareUpdate";
const char* firmware_path = "/firmwares/RAK3172-4.1.0.hex";
static const char* required_version = "4.1.0";
static const char* at_cmd = "AT+VER=?\r\n";
const char* current_version;

//check the current RAK version+
//update the RAK firmware if the current version is not the latest version. 
//use UART to send the command to update the firmware.

void app_main() {
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    
    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, 1024, 0, 0, NULL, 0);
    uart_write_bytes(UART_PORT, at_cmd, strlen(at_cmd));

    uint8_t data[128];
    int len = uart_read_bytes(UART_PORT, data, sizeof(data), 10000 / portTICK_PERIOD_MS);
    if (len > 0) {
        
        ESP_LOGI(TAG, "RAK3172 version: %s\n", data);
        current_version = data;
    } 
    else {
        ESP_LOGI(TAG, "Unable to get RAK3172 version");
    }

     if (strcmp(current_version, required_version) < 0) {
        
        ESP_LOGI(TAG, "RAK3172 firmware update started");
        char update_cmd[64];
        snprintf(update_cmd, sizeof(update_cmd), "AT+UPDATE=2,%s\r\n", firmware_path);
        uart_write_bytes(UART_PORT, update_cmd, strlen(update_cmd));
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "RAK3172 firmware update finished");

     }

}