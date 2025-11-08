#ifndef DMXOIPHANDLER_H
#define DMXOIPHANDLER_H
#include <ESPAsyncE131.h>
#include "interfaces/IDMXoIPStatus.h"

#define PACKET_TIMEOUT_MS 5000

class DMXoIPHandler : public IDMXoIPStatus {
public:
    // Lifecycle methods
    void setupArtNet();
    void readArtNet();
    void setupE131();
    void readE131();
    
    // IDMXoIPStatus Interface implementation
    bool isReceiving() const override;
    int getFrameRate() const override;

private:
    // DMX frame processing
    void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data);
    void handleE131Packet(e131_packet_t* packet);

    // Frame rate counter
    void updateFrameRate();
    
    unsigned long lastPacketTime = 0;
};

#endif // DMXOIPHANDLER_H
