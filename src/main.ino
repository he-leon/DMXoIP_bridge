#include <WiFiManager.h>

#include "ConfigParameters.h"
#include "DMXoIPHandler.h"
#include "ESPDMXNowFrameHandler.h"
#include "HardwareSerialDMXOutput.h"
#include "LEDConfig.h"
#include "NeoPixelDMXFrameHandler.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "Sensors.h"
#include "SerialDMXFrameHandler.h"
#include "StatusLED.h"

// --- Global Objects (Required for DMXoIPHandler constructor) ---
ArtnetWifi artnet;
ESPAsyncE131 e131;
DMX_ESPNOW dmxEspNow;

// --- Global Pointers (Required for loop() execution) ---
DMXoIPHandler* dmxoipHandlerPtr       = nullptr;
StatusLED* statusLEDPtr               = nullptr;
HardwareSerialDMXOutput* dmxOutputPtr = nullptr;

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

  // --- Local Pointers for Initialization ---
  IDMXFrameHandler* activeHandler             = nullptr;
  NeoPixelDMXFrameHandler* neoPixelHandlerPtr = nullptr;
  SerialDMXFrameHandler* serialDMXHandlerPtr  = nullptr;
  ESPDMXNowFrameHandler* espDmxNowHandlerPtr  = nullptr;

  if (outputMode == OUTPUT_DMX512)
  {
    Serial.println("Using Serial DMX Output Handler.");
    dmxOutputPtr        = new HardwareSerialDMXOutput(Serial2);
    serialDMXHandlerPtr = new SerialDMXFrameHandler(*dmxOutputPtr, universe);
    activeHandler       = serialDMXHandlerPtr;
  }
  else if (outputMode == OUTPUT_ESPNOW)
  {
    Serial.println("Using ESP-NOW DMX Frame Handler.");
    if (dmxEspNow.beginSender(1))
    {
      espDmxNowHandlerPtr = new ESPDMXNowFrameHandler(dmxEspNow);
      activeHandler       = espDmxNowHandlerPtr;
    }
    else
    {
      Serial.println("ERROR: Failed to initialize DMX_ESPNOW sender!");
    }
  }
  else
  {  // Default to NeoPixel
    Serial.println("Using NeoPixel Output Handler.");
    neoPixelHandlerPtr = new NeoPixelDMXFrameHandler();
    activeHandler      = neoPixelHandlerPtr;
  }

  if (activeHandler)
  {  // Check if initialization was successful
    dmxoipHandlerPtr = new DMXoIPHandler(*activeHandler, artnet, e131, dmxEspNow);
    statusLEDPtr     = new StatusLED(*dmxoipHandlerPtr, *activeHandler);
  }
  else
  {
    Serial.println("ERROR: Failed to instantiate any active DMX frame handler.");
  }

  if (statusLEDPtr)
  {
    statusLEDPtr->begin();
  }

  switch (protocol)
  {
  case PROTO_ARTNET:
    dmxoipHandlerPtr->setupArtNet();
    break;
  case PROTO_E131:
    dmxoipHandlerPtr->setupE131();
    break;
  case PROTO_ESPNOW:
    dmxoipHandlerPtr->setupEspNowReceiver();
    dmxEspNow.setReceiveUniverseId(universe);  // Set universe for receiving
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
  if (!dmxoipHandlerPtr)
    return;

  switch (protocol)
  {
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
    // Handle unknown protocol
    break;
  }

  // Send buffered data only if DMX512 or ESP-NOW is the output mode
  if (outputMode == OUTPUT_DMX512 && dmxOutputPtr)
  {
    dmxOutputPtr->sendDMX();
    dmxOutputPtr->updatePersistence();
  }
  else if (outputMode == OUTPUT_ESPNOW)
  {
    dmxEspNow.update();
  }

  // Only handle WiFiManager and OTA when not receiving DMX data
  if (!dmxoipHandlerPtr->isReceiving())
  {
    handleWiFiManager();
    handleOTA();
  }

  if (statusLEDPtr)
  {
    statusLEDPtr->update();
  }
}
