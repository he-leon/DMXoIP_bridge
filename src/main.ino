#include <WiFiManager.h>

#include "ArtNetHandler.h"
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"
#include "NetworkConfig.h"
#include "SPIFFS.h"

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
  setupArtNet();
  setupOTA();

  Serial.println("Setup complete.");
}

void loop()
{
  handleWiFiManager();
  readArtNet();
  handleOTA();
  handleSensors(); 
}
