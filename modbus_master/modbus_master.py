import serial
import struct
import time

# Configuration
PORT = 'COM6'  # Change this to match your port
BAUD = 115200

def modbus_crc16(data: bytes) -> int:
    """Calculate Modbus RTU CRC-16"""
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if crc & 0x0001:
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    return crc

def build_modbus_frame(slave_id: int, func_code: int, *data_bytes) -> bytes:
    """Build a complete Modbus RTU frame with CRC"""
    # Create frame without CRC
    if len(data_bytes) == 2:  # Address and value format (for functions 5, 6)
        frame = struct.pack('>B B H H', slave_id, func_code, data_bytes[0], data_bytes[1])
    elif len(data_bytes) == 4:  # Raw bytes format
        frame = struct.pack('>B B B B B B', slave_id, func_code, *data_bytes)
    else:
        frame = struct.pack('>B B', slave_id, func_code) + bytes(data_bytes)
    
    # Calculate and append CRC
    crc = modbus_crc16(frame)
    crc_bytes = struct.pack('<H', crc)  # LSB first
    return frame + crc_bytes

def send_command(ser, description, frame):
    """Send a Modbus command with detailed output"""
    print(f"\n{'='*70}")
    print(f"SENDING: {description}")
    print(f"Frame:   {frame.hex().upper()}")
    print(f"Length:  {len(frame)} bytes")
    
    # Break down the frame
    print("Breakdown:")
    print(f"  Slave ID:     0x{frame[0]:02X}")
    print(f"  Function:     0x{frame[1]:02X}")
    if len(frame) >= 8:
        print(f"  Data:         {frame[2:-2].hex().upper()}")
        print(f"  CRC:          {frame[-2:].hex().upper()}")
    print(f"{'='*70}")
    
    ser.write(frame)
    ser.flush()
    
    print("✓ Frame sent! Check ESP32 output above...")
    time.sleep(1)  # Give ESP32 time to process

def main():
    print("╔══════════════════════════════════════════════════════════════╗")
    print("║            MODBUS RTU EDUCATIONAL MASTER                    ║")
    print("║                                                              ║")
    print("║  This tool will send various Modbus commands to help you    ║")
    print("║  learn the protocol structure and behavior.                 ║")
    print("╚══════════════════════════════════════════════════════════════╝\n")
    
    try:
        print(f"Opening serial port {PORT} at {BAUD} baud...")
        ser = serial.Serial(PORT, BAUD, timeout=0.5)
        ser.flushInput()
        ser.flushOutput()
        time.sleep(2)
        print("✓ Connected to ESP32\n")
        
        while True:
            print("\n" + "="*70)
            print("MODBUS COMMAND MENU:")
            print("-"*20)
            print("WRITE COMMANDS:")
            print("  1 - Write Single Register: Set value 0x0001 (ON)")
            print("  2 - Write Single Register: Set value 0x0000 (OFF)")  
            print("  3 - Write Single Register: Set value 0x1234 (Custom)")
            print("  4 - Write Single Coil: Turn ON (0xFF00)")
            print("  5 - Write Single Coil: Turn OFF (0x0000)")
            print()
            print("READ COMMANDS:")
            print("  6 - Read Holding Registers (3 registers from address 0)")
            print("  7 - Read Input Registers (5 registers from address 10)")
            print("  8 - Read Coils (8 coils from address 0)")
            print("  9 - Read Discrete Inputs (12 inputs from address 5)")
            print()
            print("TEST COMMANDS:")
            print("  t - Send frame with bad CRC (test error handling)")
            print("  b - Send broadcast message (slave ID = 0)")
            print("  w - Send frame for wrong slave ID (test addressing)")
            print("  q - Quit")
            print("="*70)
            
            choice = input("Enter your choice: ").lower().strip()
            
            if choice == 'q':
                break
                
            elif choice == '1':
                frame = build_modbus_frame(0x01, 0x06, 0x0000, 0x0001)
                send_command(ser, "Write Single Register - Set to ON (0x0001)", frame)
                
            elif choice == '2':
                frame = build_modbus_frame(0x01, 0x06, 0x0000, 0x0000)
                send_command(ser, "Write Single Register - Set to OFF (0x0000)", frame)
                
            elif choice == '3':
                frame = build_modbus_frame(0x01, 0x06, 0x0000, 0x1234)
                send_command(ser, "Write Single Register - Set to Custom (0x1234)", frame)
                
            elif choice == '4':
                frame = build_modbus_frame(0x01, 0x05, 0x0000, 0xFF00)
                send_command(ser, "Write Single Coil - Turn ON", frame)
                
            elif choice == '5':
                frame = build_modbus_frame(0x01, 0x05, 0x0000, 0x0000)
                send_command(ser, "Write Single Coil - Turn OFF", frame)
                
            elif choice == '6':
                frame = build_modbus_frame(0x01, 0x03, 0x0000, 0x0003)
                send_command(ser, "Read Holding Registers (3 regs from addr 0)", frame)
                
            elif choice == '7':
                frame = build_modbus_frame(0x01, 0x04, 0x000A, 0x0005)
                send_command(ser, "Read Input Registers (5 regs from addr 10)", frame)
                
            elif choice == '8':
                frame = build_modbus_frame(0x01, 0x01, 0x0000, 0x0008)
                send_command(ser, "Read Coils (8 coils from addr 0)", frame)
                
            elif choice == '9':
                frame = build_modbus_frame(0x01, 0x02, 0x0005, 0x000C)
                send_command(ser, "Read Discrete Inputs (12 inputs from addr 5)", frame)
                
            elif choice == 't':
                # Create frame with intentionally bad CRC
                frame = build_modbus_frame(0x01, 0x06, 0x0000, 0x0001)
                bad_frame = frame[:-2] + b'\xFF\xFF'  # Replace CRC with 0xFFFF
                send_command(ser, "BAD CRC TEST - Should be rejected", bad_frame)
                
            elif choice == 'b':
                frame = build_modbus_frame(0x00, 0x06, 0x0000, 0x0001)  # Slave ID = 0 (broadcast)
                send_command(ser, "BROADCAST MESSAGE (Slave ID = 0)", frame)
                
            elif choice == 'w':
                frame = build_modbus_frame(0x99, 0x06, 0x0000, 0x0001)  # Wrong slave ID
                send_command(ser, "WRONG SLAVE ID (0x99) - Should be ignored", frame)
                
            else:
                print("Invalid choice. Please try again.")
                
            # Small delay before showing menu again
            time.sleep(0.5)
            
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    except Exception as e:
        print(f"Error: {e}") 
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("\nSerial connection closed. Goodbye!")

if __name__ == "__main__":
    main()