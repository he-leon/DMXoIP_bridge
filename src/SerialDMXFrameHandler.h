#ifndef SERIAL_DMX_FRAME_HANDLER_H
#define SERIAL_DMX_FRAME_HANDLER_H

#include "interfaces/IDMXFrameHandler.h"
#include "interfaces/IDMXOutput.h"

class SerialDMXFrameHandler : public IDMXFrameHandler {
public:
    SerialDMXFrameHandler(IDMXOutput& dmxOutput, uint16_t targetUniverse);

    void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override;
    int getFrameRate() const override;

private:
    IDMXOutput& _dmxOutput;
    const uint16_t _targetUniverse;


    void updateFrameRate();
};

#endif // SERIAL_DMX_FRAME_HANDLER_H
