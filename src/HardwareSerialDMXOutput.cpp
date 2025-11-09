#include "HardwareSerialDMXOutput.h"


HardwareSerialDMXOutput::HardwareSerialDMXOutput(HardwareSerial& serialPort)
    : _serialPort(serialPort) {
    _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N2); 
}

void HardwareSerialDMXOutput::writeFrame(uint16_t length, uint8_t* data) {
    // 1. Generate the DMX BREAK condition (Blocking)
    _serialPort.flush();
    _serialPort.begin(90000, SERIAL_8N2);
    _serialPort.write(0);
    _serialPort.flush();
    _serialPort.begin(250000, SERIAL_8N2);
    
    // 4. Send the Start Code (0x00)
    _serialPort.write(0x00);
    
    // 5. Send the DMX data slots (Blocking)
    for (int i = 0; i < length; i++) {
        _serialPort.write(data[i]);
    }
    
    // 6. Wait for all data to be transmitted
    _serialPort.flush();
}
