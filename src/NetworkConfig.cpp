#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Ticker.h>
#include <WiFiManager.h>

#include "ConfigParameters.h"
#include "LEDConfig.h"
#include "SPIFFS.h"

Ticker restartTimer;

#include "Sensors.h"
#define WM_DEBUG_LEVEL DEBUG_VERBOSE

WiFiManager wm;

void performRestart() { ESP.restart(); }

// In NetworkConfig.cpp or a similar file
// In NetworkConfig.cpp

void handleFileDownload() {
    // wm.server ist global nach bindServerCallback verfügbar
    if (!wm.server) return; 

    // ✅ NEU: Dateinamen aus dem GET-Parameter 'name' abrufen
    if (!wm.server->hasArg("name")) {
        wm.server->send(400, "text/plain", "Error: Missing 'name' parameter.");
        return;
    }

    String filename = wm.server->arg("name");
    String fullPath = "/" + filename; 

    // Security check: Nur die beiden Konfigurationsdateien zulassen
    if (fullPath == "/config.json" || fullPath == "/wifi_config.json") {
        if (SPIFFS.exists(fullPath)) {
            File file = SPIFFS.open(fullPath, "r");
            if (file) {
                // Den Dateinamen für den Browser im Header senden (optional, aber nützlich)
                wm.server->sendHeader("Content-Disposition", "attachment; filename=" + filename);
                wm.server->streamFile(file, "application/json");
                file.close();
                Serial.printf("Served dynamic file: %s via GET parameter.\n", fullPath.c_str());
            } else {
                wm.server->send(500, "text/plain", "File could not be opened.");
            }
        } else {
            wm.server->send(404, "text/plain", "File not found on filesystem.");
        }
    } else {
        wm.server->send(403, "text/plain", "Forbidden file name.");
    }
}
// This is the function called when a file chunk arrives
void handleFileUpload()
{
  if (!wm.server)
    return;

  HTTPUpload &upload = wm.server->upload();
  static File configFile;

  if (upload.status == UPLOAD_FILE_START)
  {
    // Start of upload: Get the original filename from the Python script's request
    String fullPath = "/" + upload.filename;

    // Open the file for writing
    configFile = SPIFFS.open(fullPath, "w");
    if (!configFile)
    {
      Serial.println("Failed to open file for writing!");
      return;
    }
    Serial.printf("Starting upload of %s\n", fullPath.c_str());
  }
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    // Write the chunk of data
    if (configFile)
    {
      configFile.write(upload.buf, upload.currentSize);
    }
  }
  else if (upload.status == UPLOAD_FILE_END)
  {
    // End of upload: Close the file and respond
    if (configFile)
    {
      configFile.close();
    }

    Serial.println("Upload complete. Responding OK.");
    // We must send a response back to the client (Python script) immediately
    wm.server->send(200, "text/plain", "Upload OK");

    // Schedule a restart *after* the response has been sent
    // Using the Ticker/performRestart logic you already implemented
    Serial.println("Scheduling restart in 3 seconds to load new config...");
    restartTimer.once(3, performRestart);
  }
  // UPLOAD_FILE_ABORTED is another status you might check
}

void bindServerCallback()
{
  wm.server->on("/downloadfile", HTTP_GET, handleFileDownload);
  wm.server->on(
      "/upload_config",
      HTTP_POST,
      []() { wm.server->send(200, "text/plain", "Processing upload..."); },
      handleFileUpload);
  wm.server->serveStatic("/static", SPIFFS, "/static");
}

void saveConfigCallback()
{
  Serial.println("Saving configuration from Wi-FiManager parameters...");
  numLeds      = atoi(custom_numLeds.getValue());
  universe     = atoi(custom_universe.getValue());
  startAddress = atoi(custom_startAddress.getValue());
  deviceName   = custom_deviceName.getValue();
  protocol     = static_cast<ProtocolType>(atoi(custom_protocol.getValue()));
  colorMode    = static_cast<ColorModeType>(atoi(custom_colorMode.getValue()));
  outputMode
      = static_cast<OutputModeType>(atoi(custom_outputMode.getValue()));  // <-- NEW
  savePreferences();
  // Restart device to reinitialize with new settings
  restartTimer.once(1, performRestart);
}

void setupMenu()
{
  // ... (menu setup unchanged) ...
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

  const char *menuhtml
      = "<form action='static/monitor.html' method='get'><button>Monitor</button></form><br/>\n";
  wm.setCustomMenuHTML(menuhtml);
}

void setupWiFiManager()
{
  wm.setTitle("DMXoIP Bridge");
  wm.setWebServerCallback(bindServerCallback);
  wm.setSaveParamsCallback(saveConfigCallback);
  wm.setSaveConfigCallback(
      []()
      {
        Serial.println("WiFiManager portal saved new Wi-Fi credentials.");
        WiFiConfig newConfig{WiFi.SSID(), WiFi.psk(), 0};
        addWiFiConfig(newConfig.ssid, newConfig.password, newConfig.priority);
        ESP.restart();
      });

  // Load existing config values into Wi-FiManager parameters
  custom_numLeds.setValue(String(numLeds).c_str(), 5);
  custom_universe.setValue(String(universe).c_str(), 5);
  custom_startAddress.setValue(String(startAddress).c_str(), 5);
  custom_deviceName.setValue(deviceName.c_str(), 32);
  custom_protocol.setValue(String(protocol).c_str(), 1);
  custom_colorMode.setValue(String(colorMode).c_str(), 1);
  custom_outputMode.setValue(String(outputMode).c_str(), 1);

  // Add all parameters to Wi-FiManager
  wm.addParameter(&custom_outputMode);
  wm.addParameter(&custom_outputMode_select);
  wm.addParameter(&custom_protocol);
  wm.addParameter(&custom_protocol_select);
  wm.addParameter(&custom_numLeds);
  wm.addParameter(&custom_colorMode);
  wm.addParameter(&custom_colorMode_select);
  wm.addParameter(&custom_universe);
  wm.addParameter(&custom_startAddress);
  wm.addParameter(&custom_deviceName);

  WiFi.setSleep(false);
  WiFi.hostname(deviceName.c_str());

  // ✅ JS to sync dropdowns AND show/hide NeoPixel-specific fields
  const char *headhtml = R"rawliteral(
    <script>
    document.addEventListener("DOMContentLoaded", function() {

      // --- 1. Helper function to sync dropdowns ---
      function syncDropdown(inputId, selectId) {
        var input = document.getElementById(inputId);
        var select = document.getElementById(selectId);
        if (input && select) {
          select.value = input.value || "0";
          select.addEventListener("change", function() { input.value = select.value; });
          return select; // Return the <select> element
        }
        return null;
      }

      // --- 2. Get all the elements we need to toggle ---
      // We must find the <label> and its following <br> and <input>/<select>
      
      // NeoPixel Field 1: Number of LEDs
      var numLedsLabel = document.querySelector("label[for='numLeds']");
      var numLedsBr = numLedsLabel ? numLedsLabel.nextElementSibling : null; // Gets the <br>
      var numLedsInput = document.getElementById('numLeds');

      // NeoPixel Field 2: LED Addressing (Color Mode)
      var colorModeLabel = document.querySelector("label[for='colorMode_dummy']");
      var colorModeBr = colorModeLabel ? colorModeLabel.nextElementSibling : null; // Gets the <br>
      var colorModeSelect = document.getElementById('colorMode_dummy');

      // Get the output mode <select> element
      var outputModeSelect = syncDropdown("outputMode", "outputMode_dummy");

      // --- 3. Define the toggle function ---
      function toggleNeoPixelFields() {
        // value '0' is "NeoPixel", value '1' is "DMX512"
        var show = (outputModeSelect && outputModeSelect.value == '0'); 
        var displayStyle = show ? '' : 'none';

        // Toggle visibility for all 6 elements
        if (numLedsLabel) numLedsLabel.style.display = displayStyle;
        if (numLedsBr) numLedsBr.style.display = displayStyle;
        if (numLedsInput) numLedsInput.style.display = displayStyle;
        
        if (colorModeLabel) colorModeLabel.style.display = displayStyle;
        if (colorModeBr) colorModeBr.style.display = displayStyle;
        if (colorModeSelect) colorModeSelect.style.display = displayStyle;
      }

      // --- 4. Attach event listener ---
      if (outputModeSelect) {
        outputModeSelect.addEventListener('change', toggleNeoPixelFields);
      }
      
      // --- 5. Run on initial load ---
      toggleNeoPixelFields();

      // --- 6. Sync all other dropdowns ---
      syncDropdown("protocol", "protocol_dummy");
      syncDropdown("colorMode", "colorMode_dummy");

    });
    </script>
    <style>
      /* Hide all original text inputs and their labels */
      #protocol, label[for='protocol'] { display: none; }
      #colorMode, label[for='colorMode'] { display: none; }
      #outputMode, label[for='outputMode'] { display: none; }
    </style>
    )rawliteral";

  wm.setCustomHeadElement(headhtml);
  setupMenu();

  Serial.println("Connecting to saved Wi-Fi networks...");

  // ... (rest of WiFi connection logic unchanged) ...
  // Sort by priority (high → low)
  std::sort(wifiConfigs.begin(),
            wifiConfigs.end(),
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

      wm.setAPCallback(
          [](WiFiManager *myWiFiManager)
          { Serial.println("Started non-blocking webinterface in STA mode."); });
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

  wm.setAPCallback(
      [](WiFiManager *myWiFiManager)
      {
        Serial.println("Entered config mode");
        Serial.println(WiFi.softAPIP());
        Serial.println(myWiFiManager->getConfigPortalSSID());
        setLEDColor(RgbColor(0, 0, 255));  // Blue
      });

  wm.startConfigPortal(deviceName.c_str());
}

void setupOTA()
{
  // ... (OTA setup unchanged) ...
  ArduinoOTA.setHostname(deviceName.c_str());

  ArduinoOTA.onStart([]() { Serial.println("Start updating"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress(
      [](unsigned int progress, unsigned int total)
      { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
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
