#include <WiFiManager.h>
#include "DMXoIPHandler.h"
#include "NeoPixelDMXFrameHandler.h"
#include "SerialDMXFrameHandler.h"
#include "HardwareSerialDMXOutput.h"
#include "ConfigParameters.h" // Includes universe, protocol, and new outputMode
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"

// --- Global Objects (Required for DMXoIPHandler constructor) ---
ArtnetWifi artnet;
ESPAsyncE131 e131;

// --- Global Pointers (Required for loop() execution) ---
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
    
    initializePreferences(); // Loads 'universe', 'protocol', and 'outputMode'
    setupSensors();
    setupLEDs(); // Assumed to initialize NeoPixel hardware if needed
    setupWiFiManager();
    
    // --- Local Pointers for Initialization ---
    IDMXFrameHandler* activeHandler = nullptr;
    HardwareSerialDMXOutput* dmxOutputPtr = nullptr;
    NeoPixelDMXFrameHandler* neoPixelHandlerPtr = nullptr;
    SerialDMXFrameHandler* serialDMXHandlerPtr = nullptr;

    // --- Phase 1: Instantiate Active Handler based on new config ---
    if (outputMode == OUTPUT_DMX512) { 
        Serial.println("Using Serial DMX Output Handler.");
        // Using Serial (UART0) for DMX. Assumes debug is on Serial1 or disabled.
        // If Serial is for debug, change this to Serial1 or Serial2
        dmxOutputPtr = new HardwareSerialDMXOutput(Serial); 
        serialDMXHandlerPtr = new SerialDMXFrameHandler(*dmxOutputPtr, universe); 
        activeHandler = serialDMXHandlerPtr;
    } else { // Default to NeoPixel
        Serial.println("Using NeoPixel Output Handler.");
        neoPixelHandlerPtr = new NeoPixelDMXFrameHandler();
        activeHandler = neoPixelHandlerPtr;
    }

    // --- Phase 2: Instantiate Global Handlers ---
    dmxoipHandlerPtr = new DMXoIPHandler(*activeHandler, artnet, e131);
    statusLEDPtr = new StatusLED(*dmxoipHandlerPtr, *activeHandler);
    
    // --- Phase 3: Setup ---
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
    // Execution relies only on the globally accessible dmxoipHandlerPtr and statusLEDPtr
    
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
    
    if (statusLEDPtr) {
        statusLEDPtr->update();
    }
}
