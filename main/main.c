#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "ymodem.h"
#include "include/firmware.h"

#define UART_PORT UART_NUM_1
#define TX_PIN 17
#define RX_PIN 16
#define BAUD_RATE 115200
#define MAX_FILE_SIZE 256 * 1024 // 256 KB

static const char* TAG = "RAK3172_FirmwareUpdate";
static const char* required_version = "4.1.0";
static const char* at_cmd = "AT+VER=?\r\n";
char* firmware_path = "/firmwares/RAK3172-E_latest.bin";

void uart_init(void) {
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
}

void transmit_firmware(const uint8_t* firmware_data, size_t size) {
    ESP_LOGI(TAG, "Starting YModem firmware transmission...");
    int res = Ymodem_Transmit(firmware_data, size, firmware_data); // Pass firmware data and size
    if (res == 0) {
        ESP_LOGI(TAG, "Firmware transmission completed successfully.");
    } else {
        ESP_LOGE(TAG, "Firmware transmission failed with error code: %d", res);
    }
}



void extract_version(char* response, char* version, int len) {
    // prv output test: "AT+VER=RUI_4.0.5_RAK3172-E"
    char* start = strstr(response, "AT+VER=RUI_");
    if (start) {
        start += strlen("AT+VER=RUI_");
        char* end = strchr(start, '_');
        if (end) {
            int version_len = end - start;
            if (version_len < len) {
                strncpy(version, start, version_len);
                version[version_len] = '\0';
            }
        }
    }
}

void app_main() {
    uint8_t data[64];
    char current_version[16] = {0};

    uart_init();

    // Sending AT command
    uart_write_bytes(UART_PORT, at_cmd, strlen(at_cmd));
    int len = uart_read_bytes(UART_PORT, data, sizeof(data), pdMS_TO_TICKS(1000));

    if (len > 0) {
        data[len] = '\0'; // Null-terminate the response
        ESP_LOGI(TAG, "Received data: %s", data);
        extract_version((char*)data, current_version, sizeof(current_version));
        ESP_LOGI(TAG, "RAK3172 version: %s", current_version);

        if (strcmp(current_version, required_version) < 0) {
            ESP_LOGI(TAG, "RAK3172 firmware update started");
            char update_cmd[64];
            snprintf(update_cmd, sizeof(update_cmd), "AT+UPDATE=2,%s\r\n", firmware_path);
            uart_write_bytes(UART_PORT, update_cmd, strlen(update_cmd));
            vTaskDelay(pdMS_TO_TICKS(1000));
            transmit_firmware(program, sizeof(program));
            vTaskDelay(pdMS_TO_TICKS(20000));
            ESP_LOGI(TAG, "RAK3172 firmware update finished");
        } else {
            ESP_LOGI(TAG, "RAK3172 firmware is up-to-date");
        }
    } else {
        ESP_LOGI(TAG, "Unable to get RAK3172 version");
    }
}
