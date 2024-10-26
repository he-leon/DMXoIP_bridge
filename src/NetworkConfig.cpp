
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include "ConfigParameters.h"
#include "LEDConfig.h"

WiFiManager wm;

void saveConfigCallback() {
  numLeds = atoi(custom_numLeds.getValue());
  universe = atoi(custom_universe.getValue());
  startAddress = atoi(custom_startAddress.getValue());
  deviceName = custom_deviceName.getValue();
  savePreferences();
}

void handleMonitor(){
  String totalPowerString;
  totalPowerString = String(totalPower);
  wm.server->send(200, "text/plain", totalPowerString.c_str());
}

void bindServerCallback(){
  wm.server->on("/monitor",handleMonitor); 
}

void setupMenu(){
  wm.setWebServerCallback(bindServerCallback);

  std::vector<const char *> menu = {"wifi","info","param","custom","close","sep","erase","update","restart","exit"};
  wm.setMenu(menu); // custom menu, pass vector

  // set custom html menu content, inside menu item custom
  const char* menuhtml = "<form action='/monitor' method='get'><button>Monitor</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);
}

void setupWiFiManager() {
  
  custom_numLeds.setValue(String(numLeds).c_str(), 5);
  custom_universe.setValue(String(universe).c_str(), 5);
  custom_startAddress.setValue(String(startAddress).c_str(), 5);
  custom_deviceName.setValue(deviceName.c_str(), 32);
    
  wm.addParameter(&custom_numLeds);
  wm.addParameter(&custom_universe);
  wm.addParameter(&custom_startAddress);
  wm.addParameter(&custom_deviceName);

  WiFi.hostname(deviceName.c_str());

  setupMenu();
 
  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());
    setLEDColor(CRGB::Blue);  // Indicate AP mode
  });

  wm.setSaveParamsCallback(saveConfigCallback);
  wm.autoConnect(deviceName.c_str());

  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Started non-blocking webinterface in STA mode.");
  });

  if (WiFi.status() == WL_CONNECTED) {
    blinkGreenTwice();
    wm.setConfigPortalBlocking(false);
    wm.startConfigPortal(deviceName.c_str());
  }

  if (!MDNS.begin(deviceName.c_str())) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
}

void setupOTA() {
  ArduinoOTA.setHostname(deviceName.c_str());

  ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
  ArduinoOTA.onEnd([]() { Serial.println("\\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void handleWiFiManager() {
  wm.process();
}

void handleOTA() {
  ArduinoOTA.handle();
}

