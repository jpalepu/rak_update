#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "ymodem.h"
#include "include/firmware.h"

#define TX_PIN 17
#define RX_PIN 16
#define RESET_PIN 4
#define BAUD_RATE 115200
#define MAX_FILE_SIZE 256 * 1024 
#define CHUNK_SIZE 8 
#define UART_PORT UART_NUM_2

static const char* TAG = "RAK3172_FirmwareUpdate";
static const char* at_boot_cmd = "AT+BOOT\r\n";
static const char* at_run_cmd = "AT+RUN\r\n";
static const char* at_update_cmd = "AT+UPDATE\r\n";
static const char* at_version_cmd = "AT+VER=?\r\n";
static const char* required_version = "4.1.0";

    SemaphoreHandle_t               lora_mutex = NULL;


// static const char* at_bootstatus_cmd = "AT+BOOTSTATUS\r\n";

void uart_init(void) {

    lora_mutex = xSemaphoreCreateMutex();

    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = 4,
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, 2048, 0, 0, NULL, ESP_INTR_FLAG_SHARED);
    
    ESP_LOGI(TAG, "UART initialized");
    }
void transmit_firmware(const uint8_t* firmware_data, size_t size) {
    
    ESP_LOGI(TAG, "Starting YModem firmware transmission...");
    size_t offset = 0;
    int res = 0;
    
   
    res = Ymodem_Transmit("RAK3172-E_latest.bin", firmware_data, size);
       

    if (res == 0) {
        ESP_LOGI(TAG, "Firmware transmission completed successfully.");
        ESP_LOGE(TAG, "Firmware update success..." );
    } else {
        ESP_LOGE(TAG, "Firmware update failed with error code: %d", res);
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


void read_from_modem(void *args) {

    uint8_t data[1024];
    while(true) {

        int read = uart_read_bytes(UART_NUM_2, data, sizeof(data), pdMS_TO_TICKS(10));
        if (read > 0) {
            data[read] = '\0';
            printf("%s", data);
            fflush(stdout);
        }
    }
}

void write_to_modem(void *args) {
    uint8_t data[1024];
    while(true) {
        int c = fgetc(stdin);
        if (c != EOF) {
            uart_write_bytes(UART_NUM_2, &c, 1);
        }
    }
}


void app_main() {

    gpio_config_t config = {
        .pin_bit_mask =  (1UL << RESET_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t result;
    result = gpio_config(&config);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error during gpios configuration!");
        abort();
    }

    result = gpio_set_level((gpio_num_t)RESET_PIN, 1);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error during gpio<%d> level set", RESET_PIN);
        abort();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    result = gpio_set_level((gpio_num_t)RESET_PIN, 0);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error during gpio<%d> level set", RESET_PIN);
        abort();
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // uint8_t version_data[64];
    uint8_t data[1024];
    // char current_version[16] = {0};
    
    uart_init();
    
    xTaskCreate(read_from_modem, "read_from_modem", 4 * 2048, NULL, 0, NULL);
    xTaskCreate(write_to_modem, "write_to_modem", 4 * 2048, NULL, 1, NULL);

    while(true) {
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
    // uart_write_bytes(UART_PORT, at_run_cmd, strlen(at_run_cmd));
    // vTaskDelay(pdMS_TO_TICKS(500));

    // // uart_write_bytes(UART_PORT, at_version_cmd, strlen(at_version_cmd));
    // // vTaskDelay(pdMS_TO_TICKS(500));
    // // int version_len = uart_read_bytes(UART_PORT, version_data, sizeof(version_data), pdMS_TO_TICKS(1000));
    // // ESP_LOGI(TAG, "Received Version data: %s", version_data);

    // // if (version_len > 0) {
    //     // version_data[version_len] = '\0';
    //     // extract_version((char*)version_data, current_version, sizeof(current_version));
    //     // ESP_LOGI(TAG, "RAK3172 version: %s", current_version);

    //     if (true/*strcmp(current_version, required_version) < 0*/) {
    //         ESP_LOGI(TAG, "RAK3172 firmware update started");
        
    //         // uart_write_bytes(UART_PORT, at_boot_cmd, strlen(at_boot_cmd));
    //         // vTaskDelay(pdMS_TO_TICKS(200));
    //         // int res = uart_read_bytes(UART_PORT, data, sizeof(data), pdMS_TO_TICKS(1000));
    //         // ESP_LOGI(TAG, "boot response: %s", data);

    //         // uart_write_bytes(UART_PORT, at_version_cmd, strlen(at_version_cmd));
    //         // vTaskDelay(pdMS_TO_TICKS(500));
    //         // int version_len = uart_read_bytes(UART_PORT, version_data, sizeof(version_data), pdMS_TO_TICKS(1000));
    //         // ESP_LOGI(TAG, "Received Boot Version data: %s", version_data);

    //         // uart_write_bytes(UART_PORT, at_update_cmd, strlen(at_update_cmd));
    //         transmit_firmware(program, sizeof(program));

    //         ESP_LOGI(TAG, "RAK3172 firmware update completed");
    //         vTaskDelay(pdMS_TO_TICKS(2000));
    //         esp_restart();
    //     } 
        
    //     else {
    //         ESP_LOGI(TAG, "RAK3172 firmware is up-to-date");
    //     }
    // // } 
    
    // else {
    //     ESP_LOGI(TAG, "Unable to get RAK3172 version");
    // }
}
