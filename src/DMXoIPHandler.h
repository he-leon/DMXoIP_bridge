#ifndef DMXOIPHANDLER_H
#define DMXOIPHANDLER_H

#ifdef UNIT_TEST
#include "fakes/MockDMXoIPLibs.h" 
#else
#include <ArtnetWifi.h>
#include <ESPAsyncE131.h>
#include "DMX_ESPNOW.h"
#endif

#include "interfaces/IDMXoIPStatus.h"
#include "interfaces/IDMXFrameHandler.h"

#define PACKET_TIMEOUT_MS 5000

class DMXoIPHandler : public IDMXoIPStatus {
public: 
    DMXoIPHandler(IDMXFrameHandler& frameHandler, 
                  ArtnetWifi& artnet, 
                  ESPAsyncE131& e131,
                  DMX_ESPNOW& dmxEspNow);

    void setupArtNet();
    void readArtNet();
    void setupE131();
    void readE131();

    void setupEspNowReceiver();
    void readEspNow();

    bool isReceiving() const override;
    void processFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data);
    
private:    
    IDMXFrameHandler& _frameHandler;
    ArtnetWifi& _artnet;
    ESPAsyncE131& _e131;
    DMX_ESPNOW& _dmxEspNow;

    void handleE131Packet(e131_packet_t* packet);
    unsigned long lastPacketTime = 0;

    static void espNowDmxFrameCallback(uint8_t universeId);
};

#endif // DMXOIPHANDLER_H
