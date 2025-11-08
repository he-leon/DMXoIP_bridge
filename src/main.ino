#include <WiFiManager.h>
#include "DMXoIPHandler.h"
#include "NeoPixelDMXFrameHandler.h" // <-- ADD
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"


ArtnetWifi artnet;
ESPAsyncE131 e131;

NeoPixelDMXFrameHandler dmxFrameHandler;

DMXoIPHandler dmxoipHandler(dmxFrameHandler, artnet, e131);

// Pass the DMXoIPHandler instance (which implements IDMXoIPStatus) to StatusLED
StatusLED statusLED(dmxoipHandler);

void setup()
{
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    statusLED.begin();
    initializePreferences();
    setupSensors();
    setupLEDs();
    setupWiFiManager();
    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandler.setupArtNet();
            break;
        case PROTO_E131:
            dmxoipHandler.setupE131();
            break;
        default:
            Serial.println("Unknown protocol selected.");
            break;
    }
    setupOTA();

    Serial.println("Setup complete.");
}

void loop()
{
    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandler.readArtNet();
            break;
        case PROTO_E131:
            dmxoipHandler.readE131();
            break;
        default:
            // Handle unknown protocol
            break;
    }
    if (!dmxoipHandler.isReceiving()) {
        handleWiFiManager();
        handleOTA();
    }
    statusLED.update();
}
