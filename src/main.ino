#include <WiFiManager.h>
// #include "ArtNetHandler.h" <-- REMOVE
#include "DMXoIPHandler.h" // <-- ADD
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"

// Instantiate the new DMXoIPHandler
DMXoIPHandler dmxoipHandler;

// Pass the handler instance directly to StatusLED, as it implements IDMXoIPStatus
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
            dmxoipHandler.readArtNet(); // <-- CALL METHOD
            break;
        case PROTO_E131:
            dmxoipHandler.readE131(); // <-- CALL METHOD
            break;
        default:
            // Handle unknown protocol
            break;
    }
    // Change: isReceiving() is now a method of the dmxoipHandler object
    if (!dmxoipHandler.isReceiving()) { // <-- CALL METHOD
        handleWiFiManager();
        handleOTA();
    }
    statusLED.update();
}
