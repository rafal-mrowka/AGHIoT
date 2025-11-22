#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "UART";

static const char *data = "Hello World from ESP32 AGH IoT\n\r";

#define BUF_SIZE (1024)

#define UART_PORT_NUM 2
#define UART_PIN_TXD 17
#define UART_PIN_RXD 16


void app_main(void)
{
	
	// Configure parameters of an UART driver,
    // communication pins and install the driver 
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;


    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_PIN_TXD, UART_PIN_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        
    while (true) {
        ESP_LOGI(TAG,"UART write start!\n");
        // Write data back to the UART
        uart_write_bytes(UART_PORT_NUM, (const char *) data, strlen(data));
        sleep(1);
    }
}
