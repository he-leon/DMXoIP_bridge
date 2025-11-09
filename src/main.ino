#include <WiFiManager.h>
#include "DMXoIPHandler.h"
#include "NeoPixelDMXFrameHandler.h"
#include "SerialDMXFrameHandler.h"
#include "HardwareSerialDMXOutput.h"
#include "ConfigParameters.h" // Assumed to define 'universe', 'protocol', PROTO_ARTNET, etc.
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"

ArtnetWifi artnet;
ESPAsyncE131 e131;

NeoPixelDMXFrameHandler* neoPixelHandlerPtr = nullptr;
HardwareSerialDMXOutput* dmxOutputPtr = nullptr; // Note: We use a pointer here too
SerialDMXFrameHandler* serialDMXHandlerPtr = nullptr;

DMXoIPHandler* dmxoipHandlerPtr = nullptr;

StatusLED* statusLEDPtr = nullptr;


void setup()
{
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    
    initializePreferences(); // Must load 'universe' and 'protocol' here
    setupSensors();
    setupLEDs(); // Assumed to initialize NeoPixel hardware if used
    setupWiFiManager();
    
    IDMXFrameHandler* activeHandler = nullptr;

    if (/* check for output mode == SERIAL_DMX */ 0) { // Replace '1' with actual check
        Serial.println("Using Serial DMX Output Handler.");
        dmxOutputPtr = new HardwareSerialDMXOutput(Serial); 
        serialDMXHandlerPtr = new SerialDMXFrameHandler(*dmxOutputPtr, universe); 
        activeHandler = serialDMXHandlerPtr;
    } else {
        Serial.println("Using NeoPixel Output Handler.");
        neoPixelHandlerPtr = new NeoPixelDMXFrameHandler(); // Assuming NeoPixel handler also needs universe
        activeHandler = neoPixelHandlerPtr;
    }

    dmxoipHandlerPtr = new DMXoIPHandler(*activeHandler, artnet, e131);

    statusLEDPtr = new StatusLED(*dmxoipHandlerPtr, *activeHandler);
    statusLEDPtr->begin();

    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandlerPtr->setupArtNet();
            break;
        case PROTO_E131:
            dmxoipHandlerPtr->setupE131();
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
    // Ensure the handler pointer is valid before use
    if (!dmxoipHandlerPtr) return; 
    
    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandlerPtr->readArtNet();
            break;
        case PROTO_E131:
            dmxoipHandlerPtr->readE131();
            break;
        default:
            // Handle unknown protocol
            break;
    }
    if (!dmxoipHandlerPtr->isReceiving()) {
        handleWiFiManager();
        handleOTA();
    }
    
    // Ensure the status LED pointer is valid before use
    if (statusLEDPtr) {
        statusLEDPtr->update();
    }
}
