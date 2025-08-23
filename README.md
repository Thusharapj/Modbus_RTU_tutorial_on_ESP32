# Modbus_RTU_tutorial_on_ESP32
This is a simple tutorial project on Modbus communication. Basically my PC acts as a Modbus master and sends an RTU frame via UART to me ESP32 module. The objective was to learn about Modbus communication.This project runs on an ESP32 and listens for Modbus RTU messages sent over UART (serial port). When a Modbus frame arrives, it checks if the message is valid, analyzes its contents (like function code, addresses, and CRC), and prints a detailed breakdown to the serial console. It's designed to help you learn how Modbus RTU communication works by showing exactly what each message contains and how the protocol operates. It does not control any hardware outputs—it's focused on receiving, parsing, and explaining Modbus frames for educational purposes.
##Features
- Receives Modbus RTU frames via UART
- Validates CRC checksums
- Parses and displays frame contents
- Provides detailed protocol analysis output
- Ready for integration with Python Modbus master tools
## File Overview
- `app_main.c`: Entry point; initializes UART and starts the Modbus frame analysis task.
- `modbus_slave.c` / `modbus_slave.h`: Implements Modbus frame parsing, CRC validation, and protocol analysis.
- `uart_driver.c` / `uart_driver.h`: Handles UART initialization and continuous frame reception.
- `CMakeLists.txt`: Build configuration for ESP-IDF.
## Getting Started
1. **Requirements**
   - ESP32 development board
   - ESP-IDF installed and configured
2. **Build and Flash**
   - Open a terminal in this directory
   - Run:
     ```bash
     idf.py build
     idf.py -p [PORT] flash
     ```
   - Replace `[PORT]` with your ESP32's serial port
3. **Usage**
   - Connect a Modbus master (e.g., Python script) to ESP32 UART
   - Send Modbus RTU frames and observe detailed analysis output via serial console
