#ifndef DMXOIPHANDLER_H
#define DMXOIPHANDLER_H
#ifdef UNIT_TEST
#include "fakes/MockDMXoIPLibs.h" 
#else
#include <ArtnetWifi.h>
#include <ESPAsyncE131.h>
#endif
#include "interfaces/IDMXoIPStatus.h"
#include "interfaces/IDMXFrameHandler.h" // <-- Use the interface header

#define PACKET_TIMEOUT_MS 5000

class DMXoIPHandler : public IDMXoIPStatus {
public: 
    DMXoIPHandler(IDMXFrameHandler& frameHandler, 
                  ArtnetWifi& artnet, 
                  ESPAsyncE131& e131);   

    void setupArtNet();
    void readArtNet();
    void setupE131();
    void readE131();
    bool isReceiving() const override;
    int getFrameRate() const override;
    void processFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data);
    
private:    
    IDMXFrameHandler& _frameHandler; // <-- Changed to use the interface
                                     //
    ArtnetWifi& _artnet;  // <-- Now a member
    ESPAsyncE131& _e131;  // <-- Now a member

    void handleE131Packet(e131_packet_t* packet);
    void updateFrameRate();
    unsigned long lastPacketTime = 0;
};

#endif // DMXOIPHANDLER_H
