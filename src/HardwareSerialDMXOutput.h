#ifndef HARDWARE_SERIAL_DMX_OUTPUT_H
#define HARDWARE_SERIAL_DMX_OUTPUT_H
#define MIN_FRAME_INTERVAL_MS 30
#define MAX_DMX_SLOTS 512

#include "interfaces/IDMXOutput.h"
#include <Arduino.h>

class HardwareSerialDMXOutput : public IDMXOutput {
public:
    HardwareSerialDMXOutput(HardwareSerial& serialPort);

    // Buffers the data; does NOT send it
    void writeFrame(uint16_t length, uint8_t* data) override;
    
    // Sends the buffered DMX frame
    void sendDMX();

private:
    HardwareSerial& _serialPort;
    
    static constexpr uint16_t DMX_BREAK_TIME_US = 92;
    static constexpr uint16_t DMX_MAB_TIME_US   = 12;
    static constexpr long DMX_BAUD_RATE = 250000;
    
    uint8_t _dmxBuffer[MAX_DMX_SLOTS + 1];
    uint16_t _dmxLength = 0;
};

#endif // HARDWARE_SERIAL_DMX_OUTPUT_H
