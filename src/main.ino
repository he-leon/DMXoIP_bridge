#include <WiFiManager.h>
#include "DMXoIPHandler.h"
#include "NeoPixelDMXFrameHandler.h"
#include "SerialDMXFrameHandler.h"
#include "ESPDMXNowFrameHandler.h" // <-- NEW
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
DMX_ESPNOW dmxEspNow; // <-- NEW: Global object for ESP-NOW

// --- Global Pointers (Required for loop() execution) ---
DMXoIPHandler* dmxoipHandlerPtr = nullptr;
StatusLED* statusLEDPtr = nullptr;
HardwareSerialDMXOutput* dmxOutputPtr = nullptr;

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
    NeoPixelDMXFrameHandler* neoPixelHandlerPtr = nullptr;
    SerialDMXFrameHandler* serialDMXHandlerPtr = nullptr;
    ESPDMXNowFrameHandler* espDmxNowHandlerPtr = nullptr; // <-- NEW

    // --- Phase 1: Instantiate Active Handler based on new config ---
    if (outputMode == OUTPUT_DMX512) { 
        Serial.println("Using Serial DMX Output Handler.");
        // Using Serial (UART0) for DMX. Assumes debug is on Serial1 or disabled.
        // If Serial is for debug, change this to Serial1 or Serial2
        dmxOutputPtr = new HardwareSerialDMXOutput(Serial2); 
        serialDMXHandlerPtr = new SerialDMXFrameHandler(*dmxOutputPtr, universe); 
        activeHandler = serialDMXHandlerPtr;
    } else if (outputMode == OUTPUT_ESPNOW) { // <-- NEW
        Serial.println("Using ESP-NOW DMX Frame Handler.");
        // Initialize DMX_ESPNOW as sender. Assuming channel 1 for now.
        if (dmxEspNow.beginSender(1)) {
            espDmxNowHandlerPtr = new ESPDMXNowFrameHandler(dmxEspNow);
            activeHandler = espDmxNowHandlerPtr;
        } else {
            Serial.println("ERROR: Failed to initialize DMX_ESPNOW sender!");
            // Fallback needed, but for now we'll continue with null handler if initialization fails.
        }
    } else { // Default to NeoPixel
        Serial.println("Using NeoPixel Output Handler.");
        neoPixelHandlerPtr = new NeoPixelDMXFrameHandler();
        activeHandler = neoPixelHandlerPtr;
    }

    // --- Phase 2: Instantiate Global Handlers ---
    if (activeHandler) { // Check if initialization was successful
        dmxoipHandlerPtr = new DMXoIPHandler(*activeHandler, artnet, e131, dmxEspNow);
        statusLEDPtr = new StatusLED(*dmxoipHandlerPtr, *activeHandler);
    } else {
        Serial.println("ERROR: Failed to instantiate any active DMX frame handler.");
    }
    
    // --- Phase 3: Setup ---
    if (statusLEDPtr) {
        statusLEDPtr->begin();
    }

    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandlerPtr->setupArtNet();
            break;
        case PROTO_E131:
            dmxoipHandlerPtr->setupE131();
            break;
        case PROTO_ESPNOW:
            dmxoipHandlerPtr->setupEspNowReceiver();
            dmxEspNow.setReceiveUniverseId(universe); // Set universe for receiving
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
        case PROTO_ESPNOW: // <-- NEW
            dmxoipHandlerPtr->readEspNow();
            break;
        default:
            // Handle unknown protocol
            break;
    }
    
    // Send buffered data only if DMX512 or ESP-NOW is the output mode
    if (outputMode == OUTPUT_DMX512 && dmxOutputPtr) {
        dmxOutputPtr->sendDMX();
    } else if (outputMode == OUTPUT_ESPNOW) { // <-- NEW
        // Call the update method for DMX_ESPNOW to handle packet slicing and transmission timing
        dmxEspNow.update(); 
    }
    
    if (!dmxoipHandlerPtr->isReceiving()) {
        handleWiFiManager();
        handleOTA();
    }
    
    if (statusLEDPtr) {
        statusLEDPtr->update();
    }
}
