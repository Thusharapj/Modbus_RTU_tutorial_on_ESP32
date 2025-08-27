#include "modbus_slave.h" //local header file
#include <stdio.h>
#include <string.h>
#define MODBUS_SLAVE_ID 0x01  //Defines the Modbus slave address (this device’s ID). Only frames addressed to 0x01 are processed.

static uint32_t frame_counter = 0;  //Keeps count of how many Modbus frames have been processed.
                                    //static makes it private to this file.
                                        
static uint16_t bytes_to_uint16_t(const uint8_t *data) {  //Takes two bytes and combines them into a 16-bit integer (big-endian).
    return (data[0] << 8) | data[1];
}


/*static →
Means the function is only visible inside this C file (not accessible from other files). This is common for helper functions.

uint16_t →
The function returns a 16-bit unsigned integer. That’s the CRC result (two bytes, often appended at the end of Modbus frames).

const uint8_t *data →

uint8_t = unsigned 8-bit integer (a byte).

*data = pointer to a buffer of bytes.

const = we will not modify the bytes in this buffer.

So: The function takes in a pointer to the data array whose CRC we want to calculate.

size_t len →This tells us the number of bytes in the data array.*/

/*For each byte:

XOR it into the current CRC.

For each of its 8 bits:

Shift right by 1.

If the shifted-out bit was 1, XOR with 0xA001.

At the end, we return crc, which is the 16-bit checksum for the whole message.*/
static uint16_t modbus_crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) { //  Loop through each byte in the data array.
        crc ^= data[i]; // XOR the byte with the current CRC value.
        for (int j = 0; j < 8; j++) { //  Loop through each bit in the byte.
            if (crc & 0x0001) {     //If the least significant bit (LSB) of the CRC is 1,
                crc >>= 1;          //Right shift the CRC by 1 bit.
                crc ^= 0xA001; //If the LSB is 1, XOR with 0xA001.
            } else {        //  if LSB not 1
                crc >>= 1;     // right shift CRC by 1 bit
            }
        }
    }
    return crc;
}

static const char* get_function_name(uint8_t func_code) {
    switch(func_code) {
        case 0x01: return "Read Coils";
        case 0x02: return "Read Discrete Inputs";
        case 0x03: return "Read Holding Registers";
        case 0x04: return "Read Input Registers";
        case 0x05: return "Write Single Coil";
        case 0x06: return "Write Single Register";
        case 0x0F: return "Write Multiple Coils";
        case 0x10: return "Write Multiple Registers";
        default: return "Unknown Function";
    }
}

void modbus_frame_handler(uint8_t *data, size_t len) {
    frame_counter++;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    MODBUS FRAME #%-4lu                          ║\n", frame_counter);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    // Show raw frame
    printf("║ RAW FRAME (%2d bytes):                                         ║\n", len);
    printf("║ ");
    for (int i = 0; i < len; i++) {
        printf("%02X ", data[i]);
        if (i == 7 && len > 8) printf("║\n║ "); // Line break for long frames
    }
    // Pad the line to align the border
    int padding = 58 - (len * 3);
    if (len > 8) padding = 58 - ((len - 8) * 3);
    for (int i = 0; i < padding; i++) printf(" ");
    printf("║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    if (len < 4) {
        printf("║ ERROR: Frame too short (minimum 4 bytes required)           ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        return;
    }

    // Parse frame components
    uint8_t slave_id = data[0];
    uint8_t function_code = data[1];
    
    printf("║ FRAME ANALYSIS:                                              ║\n");
    printf("║   Slave ID:       0x%02X (%3d)                                ║\n", slave_id, slave_id);
    printf("║   Function Code:  0x%02X (%s)%*s║\n", 
           function_code, get_function_name(function_code),
           25 - (int)strlen(get_function_name(function_code)), "");
    
    // CRC validation (if frame is long enough)
    if (len >= 4) {
        uint16_t received_crc = data[len - 2] | (data[len - 1] << 8);
        uint16_t computed_crc = modbus_crc16(data, len - 2);
        
        printf("║   CRC Check:                                                 ║\n");
        printf("║     Received:   0x%04X                                      ║\n", received_crc);
        printf("║     Computed:   0x%04X                                      ║\n", computed_crc);
        printf("║     Status:     %s                                    ║\n", 
               (received_crc == computed_crc) ? "✓ VALID  " : "✗ INVALID");
        
        if (received_crc != computed_crc) {
            printf("╠══════════════════════════════════════════════════════════════╣\n");
            printf("║ FRAME REJECTED: CRC Mismatch                                ║\n");
            printf("╚══════════════════════════════════════════════════════════════╝\n\n");
            return;
        }
    }
    
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    // Address check
    if (slave_id == MODBUS_SLAVE_ID) {
        printf("║ ADDRESS MATCH: Frame is for this slave                      ║\n");
    } else if (slave_id == 0x00) {
        printf("║ BROADCAST: Frame is for all slaves                          ║\n");
    } else {
        printf("║ ADDRESS MISMATCH: Frame is for slave 0x%02X (ignored)        ║\n", slave_id);
        printf("╚══════════════════════════════════════════════════════════════╝\n\n");
        return;
    }
    
    // Function-specific parsing
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ FUNCTION DATA:                                               ║\n");
    
    switch(function_code) {
        case 0x06: // Write Single Register
            if (len >= 8) {
                uint16_t reg_addr = bytes_to_uint16_t(&data[2]);
                uint16_t reg_value = bytes_to_uint16_t(&data[4]);
                printf("║   Register Address: 0x%04X (%5d)                         ║\n", reg_addr, reg_addr);
                printf("║   Register Value:   0x%04X (%5d)                         ║\n", reg_value, reg_value);
                
                // Interpret common values
                if (reg_value == 0x0000) {
                    printf("║   Interpretation:   OFF/DISABLE/FALSE                       ║\n");
                } else if (reg_value == 0x0001) {
                    printf("║   Interpretation:   ON/ENABLE/TRUE                          ║\n");
                } else if (reg_value == 0xFFFF) {
                    printf("║   Interpretation:   ALL BITS SET                            ║\n");
                }
            }
            break;
            
        case 0x03: // Read Holding Registers
        case 0x04: // Read Input Registers
            if (len >= 8) {
                uint16_t start_addr = bytes_to_uint16_t(&data[2]);
                uint16_t num_regs = bytes_to_uint16_t(&data[4]);
                printf("║   Starting Address: 0x%04X (%5d)                         ║\n", start_addr, start_addr);
                printf("║   Number of Regs:   0x%04X (%5d)                         ║\n", num_regs, num_regs);
            }
            break;
            
        case 0x01: // Read Coils
        case 0x02: // Read Discrete Inputs
            if (len >= 8) {
                uint16_t start_addr = bytes_to_uint16_t(&data[2]);
                uint16_t num_coils = bytes_to_uint16_t(&data[4]);
                printf("║   Starting Address: 0x%04X (%5d);                         ║\n", start_addr, start_addr);
                printf("║   Number of Coils:  0x%04X (%5d);                        ║\n", num_coils, num_coils);
            }
            break;
            
        case 0x05: // Write Single Coil
            if (len >= 8) {
                uint16_t coil_addr = bytes_to_uint16_t(&data[2]);
                uint16_t coil_value = bytes_to_uint16_t(&data[4]);
                printf("║   Coil Address:     0x%04X (%5d)                         ║\n", coil_addr, coil_addr);
                printf("║   Coil Value:       0x%04X (%s)                      ║\n", 
                       coil_value, (coil_value == 0xFF00) ? "ON " : (coil_value == 0x0000) ? "OFF" : "???");
            }
            break;
            
        default:
            printf("║   Raw Data Bytes:                                           ║\n");
            printf("║   ");
            for (int i = 2; i < len - 2 && i < 10; i++) {
                printf("0x%02X ", data[i]);
            }
            if (len > 12) printf("...");
            printf("%*s║\n", 40 - ((len - 4) * 5), "");
            break;
    }
    
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ FRAME PROCESSING: COMPLETE                                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
}