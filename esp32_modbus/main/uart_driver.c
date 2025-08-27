#include "driver/uart.h"
#include "modbus_slave.h"
#include <string.h>
#include <stdio.h>

#define UART_NUM UART_NUM_2
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define BUF_SIZE 1024
#define MODBUS_TIMEOUT_MS 200

void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_set_pin(UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_param_config(UART_NUM, &uart_config);
    
    printf("UART2 initialized: 9600 baud, 8N1 on GPIO17 (TX), GPIO16 (RX)\n");
    printf("Ready to receive Modbus RTU frames...\n\n");
}

void uart_task(void *pvParameter)
{
    uint8_t data[BUF_SIZE];
    
    printf("🔄 UART task started - listening for Modbus frames...\n\n");

    while (1) {
        // Clear buffer
        memset(data, 0, BUF_SIZE);
        //printf("Inside while loop of uart driver....\n\n");
        
        int len = uart_read_bytes(UART_NUM, data, BUF_SIZE, pdMS_TO_TICKS(MODBUS_TIMEOUT_MS));
        if (len > 0) {
            printf("📥 INCOMING DATA (%d bytes):\n", len);
            printf("   Raw bytes: ");
            for (int i = 0; i < len; i++) {
                printf("%02X ", data[i]);
            }
            printf("\n\n");

            // Process as Modbus frame
            modbus_frame_handler(data, len);
        }
        
        // Small delay to prevent overwhelming the system
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}