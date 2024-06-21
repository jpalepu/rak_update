// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "esp_log.h"
// #include "driver/uart.h"
// #include "driver/gpio.h"
// #include <string.h>
// #include "esp_err.h"
// #include "esp_log.h"
// #include "ymodem.h"
// #include "include/firmware.h"

// #define TX_PIN 17
// #define RX_PIN 16
// #define RESET_PIN 4
// #define BAUD_RATE 115200
// #define MAX_FILE_SIZE 256 * 1024 
// #define CHUNK_SIZE 8 
// #define UART_PORT UART_NUM_2

// static const char* TAG = "RAK3172_FirmwareUpdate";
// static const char* at_boot_cmd = "AT+BOOT\r\n";
// static const char* at_run_cmd = "AT+RUN\r\n";
// static const char* at_update_cmd = "AT+UPDATE\r\n";
// static const char* at_version_cmd = "AT+VER=?\r\n";
// static const char* required_version = "4.1.0";

//     SemaphoreHandle_t               lora_mutex = NULL;


// // static const char* at_bootstatus_cmd = "AT+BOOTSTATUS\r\n";

// void uart_init(void) {

//     lora_mutex = xSemaphoreCreateMutex();

//     uart_config_t uart_config = {
//         .baud_rate = BAUD_RATE,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = 4,
//     };

//     uart_param_config(UART_PORT, &uart_config);
//     uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
//     uart_driver_install(UART_PORT, 2048, 0, 0, NULL, ESP_INTR_FLAG_SHARED);
    
//     ESP_LOGI(TAG, "UART initialized");
//     }
// void transmit_firmware(const uint8_t* firmware_data, size_t size) {
    
//     ESP_LOGI(TAG, "Starting YModem firmware transmission...");
//     size_t offset = 0;
//     int res = 0;
    
   
//     res = Ymodem_Transmit("RAK3172-E_latest.bin", firmware_data, size);
       

//     if (res == 0) {
//         ESP_LOGI(TAG, "Firmware transmission completed successfully.");
//         ESP_LOGE(TAG, "Firmware update success..." );
//     } else {
//         ESP_LOGE(TAG, "Firmware update failed with error code: %d", res);
//     }
// }

// void extract_version(char* response, char* version, int len) {
//     // prv output test: "AT+VER=RUI_4.0.5_RAK3172-E"
//     char* start = strstr(response, "AT+VER=RUI_");
//     if (start) {
//         start += strlen("AT+VER=RUI_");
//         char* end = strchr(start, '_');
//         if (end) {
//             int version_len = end - start;
//             if (version_len < len) {
//                 strncpy(version, start, version_len);
//                 version[version_len] = '\0';
//             }
//         }
//     }
// }


// void read_from_modem(void *args) {

//     uint8_t data[1024];
//     while(true) {

//         int read = uart_read_bytes(UART_NUM_2, data, sizeof(data), pdMS_TO_TICKS(10));
//         if (read > 0) {
//             data[read] = '\0';
//             printf("%s", data);
//             fflush(stdout);
//         }
//     }
// }

// void write_to_modem(void *args) {
//     uint8_t data[1024];
//     while(true) {
//         int c = fgetc(stdin);
//         if (c != EOF) {
//             uart_write_bytes(UART_NUM_2, &c, 1);
//         }
//     }
// }


// void app_main() {

//     gpio_config_t config = {
//         .pin_bit_mask =  (1UL << RESET_PIN),
//         .mode = GPIO_MODE_OUTPUT,
//         .pull_up_en = GPIO_PULLUP_DISABLE,
//         .pull_down_en = GPIO_PULLDOWN_DISABLE,
//         .intr_type = GPIO_INTR_DISABLE,
//     };

//     esp_err_t result;
//     result = gpio_config(&config);
//     if (result != ESP_OK) {
//         ESP_LOGE(TAG, "Error during gpios configuration!");
//         abort();
//     }

//     result = gpio_set_level((gpio_num_t)RESET_PIN, 1);
//     if (result != ESP_OK) {
//         ESP_LOGE(TAG, "Error during gpio<%d> level set", RESET_PIN);
//         abort();
//     }
//     vTaskDelay(pdMS_TO_TICKS(200));
//     result = gpio_set_level((gpio_num_t)RESET_PIN, 0);
//     if (result != ESP_OK) {
//         ESP_LOGE(TAG, "Error during gpio<%d> level set", RESET_PIN);
//         abort();
//     }
//     vTaskDelay(pdMS_TO_TICKS(2000));
    
//     // uint8_t version_data[64];
//     uint8_t data[1024];
//     // char current_version[16] = {0};
    
//     uart_init();
    
//     xTaskCreate(read_from_modem, "read_from_modem", 4 * 2048, NULL, 0, NULL);
//     xTaskCreate(write_to_modem, "write_to_modem", 4 * 2048, NULL, 1, NULL);

//     while(true) {
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
    
//     // uart_write_bytes(UART_PORT, at_run_cmd, strlen(at_run_cmd));
//     // vTaskDelay(pdMS_TO_TICKS(500));

//     // // uart_write_bytes(UART_PORT, at_version_cmd, strlen(at_version_cmd));
//     // // vTaskDelay(pdMS_TO_TICKS(500));
//     // // int version_len = uart_read_bytes(UART_PORT, version_data, sizeof(version_data), pdMS_TO_TICKS(1000));
//     // // ESP_LOGI(TAG, "Received Version data: %s", version_data);

//     // // if (version_len > 0) {
//     //     // version_data[version_len] = '\0';
//     //     // extract_version((char*)version_data, current_version, sizeof(current_version));
//     //     // ESP_LOGI(TAG, "RAK3172 version: %s", current_version);

//     //     if (true/*strcmp(current_version, required_version) < 0*/) {
//     //         ESP_LOGI(TAG, "RAK3172 firmware update started");
        
//     //         // uart_write_bytes(UART_PORT, at_boot_cmd, strlen(at_boot_cmd));
//     //         // vTaskDelay(pdMS_TO_TICKS(200));
//     //         // int res = uart_read_bytes(UART_PORT, data, sizeof(data), pdMS_TO_TICKS(1000));
//     //         // ESP_LOGI(TAG, "boot response: %s", data);

//     //         // uart_write_bytes(UART_PORT, at_version_cmd, strlen(at_version_cmd));
//     //         // vTaskDelay(pdMS_TO_TICKS(500));
//     //         // int version_len = uart_read_bytes(UART_PORT, version_data, sizeof(version_data), pdMS_TO_TICKS(1000));
//     //         // ESP_LOGI(TAG, "Received Boot Version data: %s", version_data);

//     //         // uart_write_bytes(UART_PORT, at_update_cmd, strlen(at_update_cmd));
//     //         transmit_firmware(program, sizeof(program));

//     //         ESP_LOGI(TAG, "RAK3172 firmware update completed");
//     //         vTaskDelay(pdMS_TO_TICKS(2000));
//     //         esp_restart();
//     //     } 
        
//     //     else {
//     //         ESP_LOGI(TAG, "RAK3172 firmware is up-to-date");
//     //     }
//     // // } 
    
//     // else {
//     //     ESP_LOGI(TAG, "Unable to get RAK3172 version");
//     // }
// }





// -----------------------------------------------
// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/semphr.h"
// #include "esp_log.h"
// #include "driver/uart.h"
// #include "driver/gpio.h"
// #include <string.h>
// #include "esp_err.h"
// #include "esp_log.h"

// #define TX_PIN 17
// #define RX_PIN 16
// #define RESET_PIN 4
// #define BAUD_RATE 115200
// #define BUFFER_SIZE 1024
// #define UART_PORT UART_NUM_2

// static const char* TAG = "RAK_DFU Comm Sniffer";

// void uart_init(void) {

//     uart_config_t uart_config = {
//         .baud_rate = BAUD_RATE,
//         .data_bits = UART_DATA_8_BITS,
//         .parity = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = 4,
//     };

//     uart_driver_install(UART_PORT, 2048, 0, 0, NULL, 0);
//     uart_param_config(UART_PORT, &uart_config);
//     uart_set_pin(UART_PORT, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

// }
//     void UART_Sniffer_task(void *arg)
//     {
//         // u_int data[BUFFER_SIZE];
//         uint8_t *data = (u_int8_t *)malloc(BUFFER_SIZE);
//         while (1) {
//             int len = uart_read_bytes(UART_PORT, data, BUFFER_SIZE, 20 / portTICK_PERIOD_MS);
//             if (len > 0) {
//                 ESP_LOGI(TAG, "Received %d bytes", len);
//                 // for (int i = 0; i < len; i++) {
//                 //     ESP_LOGI(TAG, "0x%02x", data[i]);
//                 // }
//                 esp_log_buffer_hex(TAG, data, len);
//             }
//         }
//         free(data);

//     }

//     void app_main(void) {

//         uart_init();
//         xTaskCreate(UART_Sniffer_task, "UART_Sniffer_task", 1024, NULL, 1, NULL);

// }






































// -------------------------------------------------------

// #include <stdio.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/uart.h"
// #include "esp_log.h"
 
// #define UART_NUM_RX UART_NUM_1  // UART to listen to DFU Programmer TX
// #define UART_NUM_TX UART_NUM_2  // UART to listen to DFU Programmer RX
 
// #define BUF_SIZE (1024)
// #define RD_BUF_SIZE (BUF_SIZE)
 
// static const char *TAG_RX = "UART_RX";
// static const char *TAG_TX = "UART_TX";
 
// // Configure UART parameters
// void init_uart(uart_port_t uart_num, int tx_pin, int rx_pin)
// {
//     const uart_config_t uart_config = {
//         .baud_rate = 115200,
//         .data_bits = UART_DATA_8_BITS,
//         .parity    = UART_PARITY_DISABLE,
//         .stop_bits = UART_STOP_BITS_1,
//         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//         .source_clk = 4,
//     };
 
//     // Install UART driver
//     uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
//     uart_param_config(uart_num, &uart_config);
//     uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
// }
 
// // UART sniffer task
// void uart_sniffer_task(void *arg)
// {
//     uart_port_t uart_num = (uart_port_t)arg;
//     const char *TAG = (uart_num == UART_NUM_RX) ? TAG_RX : TAG_TX;
//     uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
 
//     if (data == NULL) {
//         ESP_LOGE(TAG, "Failed to allocate memory for UART buffer");
//         vTaskDelete(NULL);  // Delete task if memory allocation fails
//     }
 
//     while (1) {
//         // Read data from UART
//         int len = uart_read_bytes(uart_num, data, RD_BUF_SIZE, 20 / portTICK_PERIOD_MS);
//         if (len > 0) {
//             ESP_LOGI(TAG, "Received %d bytes", len);
//             ESP_LOG_BUFFER_HEX(TAG, data, len);
//         }
//     }
 
//     free(data);  // Free the buffer when the task is done (though this line will never be reached)
// }
 
// void app_main(void)
// {
//     // Initialize the UARTs
//     init_uart(UART_NUM_RX, UART_PIN_NO_CHANGE, 16); // Listen to DFU Programmer TX (RX 16)
//     init_uart(UART_NUM_TX, UART_PIN_NO_CHANGE, 17); // Listen to DFU Programmer RX
 
//     // Create tasks to handle the UART sniffing
//     xTaskCreate(uart_sniffer_task, "uart_sniffer_task_rx", 4096, (void *)UART_NUM_RX, 10, NULL);
//     xTaskCreate(uart_sniffer_task, "uart_sniffer_task_tx", 4096, (void *)UART_NUM_TX, 10, NULL);
// }


// ------- using circular buffer --------------------------------


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/ringbuf.h"
#include "driver/uart.h"
#include "esp_log.h"
 
#define UART_NUM_RX UART_NUM_1  
#define UART_NUM_TX UART_NUM_2  // rx
 
#define BUF_SIZE 1024
#define RD_BUF_SIZE BUF_SIZE
#define RINGBUF_SIZE BUF_SIZE * 10
 
static const char *TAG_RX = "UART_RX";
static const char *TAG_TX = "UART_TX";
static const char *TAG = "UART_BUFFER";


RingbufHandle_t ringbuf_handle;
 
// Configure UART parameters
void init_uart(uart_port_t uart_num, int tx_pin, int rx_pin)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = 4,
    };
 
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
 
// UART sniffer task
void uart_sniffer_task(void *arg)
{
    uart_port_t uart_num = (uart_port_t)arg;
    const char *TAG = (uart_num == UART_NUM_RX) ? TAG_RX : TAG_TX;
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE); // yesterady problem (stack overflow)
 
    if (data == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for UART buffer");
        vTaskDelete(NULL); 
    }
 
    while (1) {
    
        int len = uart_read_bytes(uart_num, data, RD_BUF_SIZE, 20 / portTICK_PERIOD_MS);
        if (len > 0) {
            ESP_LOGI(TAG, "Received %d bytes", len);
            // have to write the received data to the ring buffer
            if (xRingbufferSend(ringbuf_handle, data, len, pdMS_TO_TICKS(100)) != pdTRUE) {
                ESP_LOGW(TAG, "Failed to send data to ring buffer");
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(100)); // Delay to avoid busy waiting
        }
    }
 
    free(data);  // free buffer memory
}
 
void print_logs_from_ringbuffer(void)
{
    size_t item_size;
    uint8_t *item;
 
    while ((item = (uint8_t *)xRingbufferReceive(ringbuf_handle, &item_size, pdMS_TO_TICKS(1000))) != NULL) {
        ESP_LOGI("RINGBUF", "Logging %d bytes from ring buffer", item_size);
        ESP_LOG_BUFFER_HEX("RINGBUF", item, item_size);
        vRingbufferReturnItem(ringbuf_handle, (void *)item);
    }
}
 
void app_main(void)
{
    // Initialize the ring buffer
    ringbuf_handle = xRingbufferCreate(RINGBUF_SIZE, RINGBUF_TYPE_BYTEBUF);
    if (ringbuf_handle == NULL) {
        ESP_LOGE("RINGBUF", "Failed to create ring buffer");
        return;
    }
 
    init_uart(UART_NUM_RX, UART_PIN_NO_CHANGE, 16); // programmer - tx
    init_uart(UART_NUM_TX, UART_PIN_NO_CHANGE, 17); // Programmer - rX
 
    xTaskCreate(uart_sniffer_task, "uart_sniffer_task_rx", 4096, (void *)UART_NUM_RX, 10, NULL);
    xTaskCreate(uart_sniffer_task, "uart_sniffer_task_tx", 4096, (void *)UART_NUM_TX, 10, NULL);
 
    ESP_LOGI(TAG, "Will start to print");
    vTaskDelay(pdMS_TO_TICKS(10000));  
 
    // Print logs from ring buffer
    print_logs_from_ringbuffer();
 
    // Clean up
    vRingbufferDelete(ringbuf_handle);
}