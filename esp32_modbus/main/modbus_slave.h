#pragma once

#include <stddef.h>
#include <stdint.h>

// Process and analyze a received Modbus RTU frame
void modbus_frame_handler(uint8_t *data, size_t len);