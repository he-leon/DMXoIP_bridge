#include "HardwareSerialDMXOutput.h"

HardwareSerialDMXOutput::HardwareSerialDMXOutput(HardwareSerial& serialPort)
    : _serialPort(serialPort) {
    _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N1); 
}

void HardwareSerialDMXOutput::writeFrame(uint16_t length, uint8_t* data) {
    // 1. Generate the DMX BREAK condition (Blocking)
    _serialPort.begin(100000, SERIAL_8N1); 
    _serialPort.write(0x00);
    _serialPort.flush();
    
    // 2. Generate the Mark After Break (MAB) (Blocking)
    delayMicroseconds(DMX_MAB_TIME_US);
    
    // 3. Restore the DMX Baud Rate
    _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N1); 
    
    // 4. Send the Start Code (0x00)
    _serialPort.write(0x00);
    
    // 5. Send the DMX data slots (Blocking)
    _serialPort.write(data, length);
    
    // 6. Wait for all data to be transmitted
    _serialPort.flush();
}
