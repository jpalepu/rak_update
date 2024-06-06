idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "driver" "freertos" "esp_rom" "esp_timer" "esp_adc" "nvs_flash" "ymodem")