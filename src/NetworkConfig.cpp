#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>

#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "Sensors.h"

WiFiManager wm;

void saveConfigCallback()
{
  numLeds      = atoi(custom_numLeds.getValue());
  universe     = atoi(custom_universe.getValue());
  startAddress = atoi(custom_startAddress.getValue());
  deviceName   = custom_deviceName.getValue();
  protocol     = static_cast<ProtocolType>(atoi(custom_protocol.getValue()));
  colorMode    = static_cast<ColorModeType>(atoi(custom_colorMode_select.getValue())); // ✅ Save color mode
  savePreferences();
}

void handleMonitor()
{
  StaticJsonDocument<200> doc;
  doc["totalPower"]               = totalPower;
  doc["currentTemperature"]       = currentTemperature;
  doc["currentInternalTemperature"] = currentInternalTemperature;
  wm.server->send(200, "application/json", doc.as<String>());
}

void bindServerCallback() { wm.server->on("/monitor", handleMonitor); }

void setupMenu()
{
  wm.setWebServerCallback(bindServerCallback);

  std::vector<const char *> menu = {"wifi",
                                    "info",
                                    "param",
                                    "custom",
                                    "close",
                                    "sep",
                                    "erase",
                                    "update",
                                    "restart",
                                    "exit"};
  wm.setMenu(menu);

  const char *menuhtml =
      "<form action='/monitor' method='get'><button>Monitor</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);
}

void setupWiFiManager()
{
  // Load existing config values into Wi-FiManager parameters
  custom_numLeds.setValue(String(numLeds).c_str(), 5);
  custom_universe.setValue(String(universe).c_str(), 5);
  custom_startAddress.setValue(String(startAddress).c_str(), 5);
  custom_deviceName.setValue(deviceName.c_str(), 32);
  custom_protocol.setValue(String(protocol).c_str(), 1);
  custom_colorMode_select.setValue(String(static_cast<int>(colorMode)).c_str(), 1); // ✅ Preload color mode

  // Add all parameters to Wi-FiManager
  wm.addParameter(&custom_numLeds);
  wm.addParameter(&custom_universe);
  wm.addParameter(&custom_startAddress);
  wm.addParameter(&custom_deviceName);
  wm.addParameter(&custom_protocol);
  wm.addParameter(&custom_protocol_select);
  wm.addParameter(&custom_colorMode_select); // ✅ Add dropdown to portal

  WiFi.setSleep(false);
  WiFi.hostname(deviceName.c_str());

  // ✅ JS to sync both dropdowns (protocol + color mode)
  const char *headhtml = R"rawliteral(
  <script>
  document.addEventListener("DOMContentLoaded", function() {
    // Protocol dropdown sync
    var protoInput = document.getElementById("protocol");
    var protoSelect = document.getElementById("protocol_dummy");
    if (protoInput && protoSelect) {
      protoSelect.value = protoInput.value || "0";
      protoSelect.addEventListener("change", function() {
        protoInput.value = protoSelect.value;
      });
    }

    // Color mode dropdown sync
    var colorInput = document.getElementById("colorMode");
    var colorSelect = document.getElementById("colorMode");
    if (colorInput && colorSelect) {
      colorSelect.value = colorInput.value || "0";
      colorSelect.addEventListener("change", function() {
        colorInput.value = colorSelect.value;
      });
    }
  });
  </script>
  <style>
    #protocol, label[for='protocol'] { display: none; }
  </style>
  )rawliteral";

  wm.setCustomHeadElement(headhtml);
  setupMenu();

  Serial.println("Connecting to saved Wi-Fi networks...");

  // Sort by priority (high → low)
  std::sort(wifiConfigs.begin(), wifiConfigs.end(),
            [](WiFiConfig &a, WiFiConfig &b) { return a.priority > b.priority; });

  for (const auto &config : wifiConfigs)
  {
    Serial.print("Trying to connect to: ");
    Serial.println(config.ssid);

    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
      delay(100);

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("Connected to ");
      Serial.println(config.ssid);

      if (config.priority == 0)
      {
        addWiFiConfig(config.ssid, config.password, 1);
        saveWiFiConfigs();
      }

      blinkGreenTwice();

      wm.setAPCallback([](WiFiManager *myWiFiManager) {
        Serial.println("Started non-blocking webinterface in STA mode.");
      });
      wm.setConfigPortalBlocking(false);
      wm.startConfigPortal(deviceName.c_str());

      if (!MDNS.begin(deviceName.c_str()))
        Serial.println("Error setting up MDNS responder!");
      else
      {
        Serial.println("mDNS responder started");
        MDNS.addService("http", "tcp", 80);
      }
      return;
    }
    else
    {
      Serial.print("Failed to connect to ");
      Serial.println(config.ssid);
      addWiFiConfig(config.ssid, config.password, 0);
    }
  }

  Serial.println("No saved networks connected. Starting WiFiManager...");

  wm.setAPCallback([](WiFiManager *myWiFiManager) {
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    Serial.println(myWiFiManager->getConfigPortalSSID());
    setLEDColor(RgbColor(0, 0, 255));  // Blue
  });

  wm.setSaveParamsCallback(saveConfigCallback);
  wm.setSaveConfigCallback([]() {
    Serial.println("WiFiManager portal saved new Wi-Fi credentials.");
    WiFiConfig newConfig{WiFi.SSID(), WiFi.psk(), 0};
    addWiFiConfig(newConfig.ssid, newConfig.password, newConfig.priority);
    ESP.restart();
  });

  wm.startConfigPortal(deviceName.c_str());
}

void setupOTA()
{
  ArduinoOTA.setHostname(deviceName.c_str());

  ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
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

void handleWiFiManager() { wm.process(); }
void handleOTA() { ArduinoOTA.handle(); }

