
#include <WiFiManager.h>
#include "NetworkConfig.h"
#include "ArtNetHandler.h"
#include "LEDConfig.h"
#include "ConfigParameters.h"


void setup() {
  Serial.begin(115200);
  
  initializePreferences();
  setupLEDs();
  setupWiFiManager();
  setupArtNet();
  setupOTA();

  Serial.println("Setup complete.");
}

void loop() {
  handleWiFiManager();
  readArtNet();
  handleOTA();
}

