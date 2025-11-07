#include <WiFiManager.h>
#include "ArtNetHandler.h"
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"
#include "StatusLED.h"

StatusLED statusLED; // uses LED_BUILTIN by default

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
      setupArtNet();
      break;
    case PROTO_E131:
      setupE131();
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
      readArtNet();
      break;
    case PROTO_E131:
      readE131();
      break;
    default:
      // Handle unknown protocol
      break;
  }
  if (!isReceiving()) {
    handleWiFiManager();
    handleOTA();
  }
  statusLED.update(isReceiving());
}
