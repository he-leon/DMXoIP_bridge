# 1 "/tmp/tmpuey5k3td"
#include <Arduino.h>
# 1 "/home/lenni/projects/lighting/esp32_ws2812_artnet/src/main.ino"
#include <WiFiManager.h>
#include "DMXoIPHandler.h"
#include "NeoPixelDMXFrameHandler.h"
#include "SerialDMXFrameHandler.h"
#include "ESPDMXNowFrameHandler.h"
#include "HardwareSerialDMXOutput.h"
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"


ArtnetWifi artnet;
ESPAsyncE131 e131;
DMX_ESPNOW dmxEspNow;


DMXoIPHandler* dmxoipHandlerPtr = nullptr;
StatusLED* statusLEDPtr = nullptr;
HardwareSerialDMXOutput* dmxOutputPtr = nullptr;
void setup();
void loop();
#line 24 "/home/lenni/projects/lighting/esp32_ws2812_artnet/src/main.ino"
void setup()
{
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    initializePreferences();
    setupSensors();
    setupLEDs();
    setupWiFiManager();


    IDMXFrameHandler* activeHandler = nullptr;
    NeoPixelDMXFrameHandler* neoPixelHandlerPtr = nullptr;
    SerialDMXFrameHandler* serialDMXHandlerPtr = nullptr;
    ESPDMXNowFrameHandler* espDmxNowHandlerPtr = nullptr;

    if (outputMode == OUTPUT_DMX512) {
        Serial.println("Using Serial DMX Output Handler.");
        dmxOutputPtr = new HardwareSerialDMXOutput(Serial2);
        serialDMXHandlerPtr = new SerialDMXFrameHandler(*dmxOutputPtr, universe);
        activeHandler = serialDMXHandlerPtr;
    } else if (outputMode == OUTPUT_ESPNOW) {
        Serial.println("Using ESP-NOW DMX Frame Handler.");
        if (dmxEspNow.beginSender(1)) {
            espDmxNowHandlerPtr = new ESPDMXNowFrameHandler(dmxEspNow);
            activeHandler = espDmxNowHandlerPtr;
        } else {
            Serial.println("ERROR: Failed to initialize DMX_ESPNOW sender!");
        }
    } else {
        Serial.println("Using NeoPixel Output Handler.");
        neoPixelHandlerPtr = new NeoPixelDMXFrameHandler();
        activeHandler = neoPixelHandlerPtr;
    }

    if (activeHandler) {
        dmxoipHandlerPtr = new DMXoIPHandler(*activeHandler, artnet, e131, dmxEspNow);
        statusLEDPtr = new StatusLED(*dmxoipHandlerPtr, *activeHandler);
    } else {
        Serial.println("ERROR: Failed to instantiate any active DMX frame handler.");
    }

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
            dmxEspNow.setReceiveUniverseId(universe);
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
    if (!dmxoipHandlerPtr) return;

    switch (protocol) {
        case PROTO_ARTNET:
            dmxoipHandlerPtr->readArtNet();
            break;
        case PROTO_E131:
            dmxoipHandlerPtr->readE131();
            break;
        case PROTO_ESPNOW:
            dmxoipHandlerPtr->readEspNow();
            break;
        default:

            break;
    }


    if (outputMode == OUTPUT_DMX512 && dmxOutputPtr) {
        dmxOutputPtr->sendDMX();
    } else if (outputMode == OUTPUT_ESPNOW) {

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