#ifndef HARDWARE_SERIAL_DMX_OUTPUT_H
#define HARDWARE_SERIAL_DMX_OUTPUT_H
#define MIN_FRAME_INTERVAL_MS 30

#include "interfaces/IDMXOutput.h"
#include <Arduino.h>
#include <easydmx.h>

class HardwareSerialDMXOutput : public IDMXOutput {
public:
    HardwareSerialDMXOutput(HardwareSerial& serialPort);

    void writeFrame(uint16_t length, uint8_t* data) override;

private:
    HardwareSerial& _serialPort;
    EasyDMX _dmx;
    
    static constexpr uint16_t DMX_BREAK_TIME_US = 92;
    static constexpr uint16_t DMX_MAB_TIME_US   = 12;
    static constexpr long DMX_BAUD_RATE = 250000;
    long _lastSendTime = 0;
};

#endif // HARDWARE_SERIAL_DMX_OUTPUT_H
