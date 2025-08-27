#pragma once

// Initialize UART for Modbus communication
void uart_init(void);

// UART task for receiving and processing Modbus frames
void uart_task(void *pvParameter);