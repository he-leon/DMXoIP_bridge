#include "HardwareSerialDMXOutput.h"
#include <string.h> // For memcpy

HardwareSerialDMXOutput::HardwareSerialDMXOutput(HardwareSerial& serialPort)
    : _serialPort(serialPort) {
    _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N2); 
}

void HardwareSerialDMXOutput::writeFrame(uint16_t length, uint8_t* data) {
    if (length > MAX_DMX_SLOTS) {
        length = MAX_DMX_SLOTS;
    }
    memcpy(_dmxBuffer, data, length); 
    _dmxLength = length;
}

void HardwareSerialDMXOutput::sendDMX() {
    if (_dmxLength == 0) {
        return; // Nothing to send
    }
    
    // The following will take ~23ms and will block execution during that time.
    // 1. Generate the DMX BREAK condition 
    // Flush any pending outgoing data
    _serialPort.flush(); 
    
    // Change to a lower baud rate (or simply 90000) to force a BREAK condition
    _serialPort.begin(90000, SERIAL_8N2);
    // Send a dummy byte (0) at the low rate to ensure the break is long enough
    _serialPort.write(0);
    // Wait for the BREAK to finish
    _serialPort.flush();
    
    // Return to the DMX baud rate for data transmission
    _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N2); 
    
    // Send the Start Code (0x00)
    _serialPort.write(0x00);
    
    // Send the DMX data slots 
    for (int i = 0; i < _dmxLength; i++) {
        _serialPort.write(_dmxBuffer[i]);
    }
    
    // Wait for all data to be transmitted
    _serialPort.flush();
    
}
