#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_driver.h"
#include "esp_log.h"

void app_main(void)
{
    // Enable all debug output for learning
    // esp_log_level_set("*", ESP_LOG_NONE);  // Commented out for learning mode
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              ESP32 MODBUS RTU LEARNING SYSTEM               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  This system will:                                          ║\n");
    printf("║  • Receive Modbus RTU frames via UART                       ║\n");
    printf("║  • Validate CRC checksums                                   ║\n");
    printf("║  • Parse and display frame contents                         ║\n");
    printf("║  • Show detailed protocol analysis                          ║\n");
    printf("║                                                              ║\n");
    printf("║  Send commands from your Python Modbus master!              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    uart_init();       // Initialize UART only
    
    printf("System ready - waiting for Modbus RTU frames...\n");
    printf("================================================================\n\n");
    
    // Create UART task to handle Modbus
    xTaskCreate(
        uart_task,
        "uart_task",
        4096,
        NULL,
        5,
        NULL
    );
}