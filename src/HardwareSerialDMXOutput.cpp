#include "HardwareSerialDMXOutput.h"


HardwareSerialDMXOutput::HardwareSerialDMXOutput(HardwareSerial& serialPort)
    : _serialPort(serialPort) {
    _dmx.begin(DMXMode::Transmit, DMXPin::NoRx, 1);
}

void HardwareSerialDMXOutput::writeFrame(uint16_t length, uint8_t* data) {
    for (uint16_t i = 0; i < length; i++) {
        _dmx.setChannel(i + 1, data[i]); // DMX channels are 1-based
    }
}
