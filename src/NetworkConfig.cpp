
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>

#include "ConfigParameters.h"
#include "LEDConfig.h"

WiFiManager wm;

void saveConfigCallback()
{
  numLeds      = atoi(custom_numLeds.getValue());
  universe     = atoi(custom_universe.getValue());
  startAddress = atoi(custom_startAddress.getValue());
  deviceName   = custom_deviceName.getValue();
  savePreferences();
}

void handleMonitor()
{
  String totalPowerString;
  totalPowerString = String(totalPower);
  wm.server->send(200, "text/plain", totalPowerString.c_str());
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
  wm.setMenu(menu);  // custom menu, pass vector

  // set custom html menu content, inside menu item custom
  const char *menuhtml
      = "<form action='/monitor' "
        "method='get'><button>Monitor</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);
}

void setupWiFiManager()
{
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

  Serial.println("Connecting to saved Wi-Fi networks...");

  // Sort Wi-Fi configurations by priority (higher first)
  std::sort(wifiConfigs.begin(),
            wifiConfigs.end(),
            [](WiFiConfig &a, WiFiConfig &b) { return a.priority > b.priority; });

  for (const auto &config : wifiConfigs)
  {
    Serial.print("Trying to connect to: ");
    Serial.println(config.ssid);

    WiFi.begin(config.ssid.c_str(), config.password.c_str());

    // Wait up to 10 seconds for connection
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
    {
      delay(100);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("Connected to ");
      if (config.priority == 0)
      {
        // Setting priority to 1 so that this network is preferred next time
        addWiFiConfig(config.ssid, config.password, 1);
        saveWiFiConfigs();
      }
      Serial.println(config.ssid);
      blinkGreenTwice();
      wm.setAPCallback(
          [](WiFiManager *myWiFiManager)
          { Serial.println("Started non-blocking webinterface in STA mode."); });
      wm.setConfigPortalBlocking(false);
      wm.startConfigPortal(deviceName.c_str());

      if (!MDNS.begin(deviceName.c_str()))
      {
        Serial.println("Error setting up MDNS responder!");
      }
      else
      {
        Serial.println("mDNS responder started");
        MDNS.addService("http", "tcp", 80);
      }
      return;  // Exit if connected
    }
    else
    {
      Serial.print("Failed to connect to ");
      Serial.println(config.ssid);
      // Set priority to 0 for non-available networks
      // Will be saved if connected to a network eventually
      addWiFiConfig(config.ssid, config.password, 0);
    }
  }

  // If no saved network connects, start WiFiManager to configure a new one
  Serial.println("No saved networks connected. Starting WiFiManager...");

  wm.setAPCallback(
      [](WiFiManager *myWiFiManager)
      {
        Serial.println("Entered config mode");
        Serial.println(WiFi.softAPIP());
        Serial.println(myWiFiManager->getConfigPortalSSID());
        setLEDColor(CRGB::Blue);  // Indicate AP mode
      });

  wm.setSaveParamsCallback(saveConfigCallback);
  wm.setSaveConfigCallback(
      []()
      {
        Serial.println("WiFiManager portal saved new Wi-Fi credentials.");

        // Store connected network in the configs
        WiFiConfig newConfig;
        newConfig.ssid     = WiFi.SSID();
        newConfig.password = WiFi.psk();
        newConfig.priority = 0;  // Default priority for new networks
        addWiFiConfig(newConfig.ssid, newConfig.password, newConfig.priority);
        ESP.restart();
      });
  wm.startConfigPortal(deviceName.c_str());
}

void setupOTA()
{
  ArduinoOTA.setHostname(deviceName.c_str());

  ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
  ArduinoOTA.onEnd([]() { Serial.println("\\nEnd"); });
  ArduinoOTA.onProgress(
      [](unsigned int progress, unsigned int total)
      { Serial.printf("Progress: %u%%\\r", (progress / (total / 100))); });
  ArduinoOTA.onError(
      [](ota_error_t error)
      {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
          Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR)
          Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR)
          Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR)
          Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR)
          Serial.println("End Failed");
      });
  ArduinoOTA.begin();
}

void handleWiFiManager() { wm.process(); }

void handleOTA() { ArduinoOTA.handle(); }
