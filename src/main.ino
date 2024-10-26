
#include <WiFiManager.h>
#include "NetworkConfig.h"
#include "ArtNetHandler.h"
#include "LEDConfig.h"
#include "ConfigParameters.h"
#include "SPIFFS.h"
#include "FS.h"


void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
  }

  // FSInfo fs_info;
  // SPIFFS.info(fs_info);
  // Serial.println("SPIFFS Info:");
  // Serial.printf("Total Bytes: %u\n", fs_info.totalBytes);
  // Serial.printf("Used Bytes: %u\n", fs_info.usedBytes);
  // Serial.printf("Free Bytes: %u\n", fs_info.totalBytes - fs_info.usedBytes);
  
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

