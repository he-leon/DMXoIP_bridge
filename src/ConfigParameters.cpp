#include "ConfigParameters.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>

int numLeds = 30;
int universe = 0;
int startAddress = 2;
String chipID = String((uint32_t)ESP.getEfuseMac(), HEX);
String defaultDeviceName = "ESP32-" + chipID;
String deviceName = defaultDeviceName;

ProtocolType protocol = PROTO_E131;
ColorModeType colorMode = COLOR_MODE_MULTIPLE;
OutputModeType outputMode = OUTPUT_NEOPIXEL;

const char *configFilePath = "/config.json";
const char *wifiConfigFilePath = "/wifi_config.json";
std::vector<WiFiConfig> wifiConfigs;

// Wi-FiManager Parameters
WiFiManagerParameter custom_numLeds("numLeds", "Number of LEDs", String(numLeds).c_str(), 5);
WiFiManagerParameter custom_universe("universe", "Universe", String(universe).c_str(), 5);
WiFiManagerParameter custom_startAddress("startAddress", "Start Address", String(startAddress).c_str(), 5);
WiFiManagerParameter custom_deviceName("deviceName", "Device Name", defaultDeviceName.c_str(), 32);
WiFiManagerParameter custom_protocol("protocol", "Protocol", String(protocol).c_str(), 1);
WiFiManagerParameter custom_colorMode("colorMode", "LED Addressing", String(colorMode).c_str(), 1); 
WiFiManagerParameter custom_outputMode("outputMode", "Output Mode", String(outputMode).c_str(), 1); // <-- NEW

const char *protocolDropdownStr = R"(
  <label for='protocol_dummy'>Protocol</label>
  <select name='protocol_dummy' id='protocol_dummy' class='button'>
    <option value='0'>ArtNet</option>
    <option value='1'>E1.31</option>
    <option value='2'>ESP-NOW</option>
  </select>
)";
WiFiManagerParameter custom_protocol_select(protocolDropdownStr);

const char *colorModeDropdownStr = R"(
  <label for='colorMode_dummy'>LED Addressing</label>
  <select name='colorMode_dummy' id='colorMode_dummy' class='button'>
    <option value='0'>Individual</option>
    <option value='1'>Single combined</option>
  </select>
)";
WiFiManagerParameter custom_colorMode_select(colorModeDropdownStr);

const char *outputModeDropdownStr = R"(
  <label for='outputMode_dummy'>Output Mode</label>
  <select name='outputMode_dummy' id='outputMode_dummy' class='button'>
    <option value='0'>NeoPixel</option>
    <option value='1'>DMX512</option>
    <option value='2'>ESP-NOW</option>
  </select>
)";
WiFiManagerParameter custom_outputMode_select(outputModeDropdownStr);

// ----------------- Wi-Fi Config -----------------
void loadWiFiConfigs() {
    File configFile = SPIFFS.open(wifiConfigFilePath, FILE_READ);
    if (!configFile) { Serial.println("No Wi-Fi config file found, using defaults"); return; }

    DynamicJsonDocument doc(1024);
    if (deserializeJson(doc, configFile)) { Serial.println("Failed to read Wi-Fi config file"); return; }

    wifiConfigs.clear();
    for (JsonObject obj : doc["wifiConfigs"].as<JsonArray>()) {
        WiFiConfig config;
        config.ssid = obj["ssid"].as<String>();
        config.password = obj["password"].as<String>();
        config.priority = obj["priority"] | 0;
        wifiConfigs.push_back(config);
    }
    configFile.close();
}

// Save Wi-Fi configs
void saveWiFiConfigs() {
    File configFile = SPIFFS.open(wifiConfigFilePath, FILE_WRITE);
    if (!configFile) { Serial.println("Failed to open Wi-Fi config file for writing"); return; }

    DynamicJsonDocument doc(1024);
    JsonArray array = doc.createNestedArray("wifiConfigs");
    for (auto &config : wifiConfigs) {
        JsonObject obj = array.createNestedObject();
        obj["ssid"] = config.ssid;
        obj["password"] = config.password;
        obj["priority"] = config.priority;
    }
    serializeJson(doc, configFile);
    configFile.close();
}

// Add/update a Wi-Fi network
void addWiFiConfig(const String &ssid, const String &password, int priority) {
    bool exists = false;
    WiFiConfig newConfig{ssid, password, priority};
    for (auto &cfg : wifiConfigs) { if (cfg.ssid == ssid) { cfg = newConfig; exists = true; break; } }
    if (!exists) wifiConfigs.push_back(newConfig);
    saveWiFiConfigs();
}

// ----------------- Preferences -----------------
void initializePreferences() {
    Serial.println("Initializing preferences...");
    loadWiFiConfigs();
    loadPreferences();
}

void loadPreferences() {
    Serial.println("Loading preferences from SPIFFS...");
    File configFile = SPIFFS.open(configFilePath, FILE_READ);
    if (!configFile) { Serial.println("Config file missing, using defaults"); return; }

    size_t size = configFile.size();
    if (size > 1024) { Serial.println("Config file too large"); configFile.close(); return; }

    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    DynamicJsonDocument json(1024);
    if (deserializeJson(json, buf.get())) { Serial.println("Failed to parse config file, using defaults"); }
    else {
        numLeds = json["numLeds"] | 30;
        universe = json["universe"] | 0;
        startAddress = json["startAddress"] | 2;
        deviceName = json["deviceName"] | defaultDeviceName;
        protocol = static_cast<ProtocolType>(json["protocol"] | PROTO_E131);
        colorMode = static_cast<ColorModeType>(json["colorMode"] | COLOR_MODE_MULTIPLE);
        outputMode = static_cast<OutputModeType>(json["outputMode"] | OUTPUT_NEOPIXEL); // <-- NEW
    }
    configFile.close();
    Serial.printf(
        "Loaded preferences - numLeds: %d, universe: %d, startAddress: %d, deviceName: %s, protocol: %d, colorMode: %d, outputMode: %d\n", // <-- MODIFIED
        numLeds, universe, startAddress, deviceName.c_str(), protocol, colorMode, outputMode // <-- MODIFIED
    );
}

void savePreferences() {
    File configFile = SPIFFS.open(configFilePath, FILE_WRITE);
    if (!configFile) { Serial.println("Failed to open config file for writing"); return; }
    Serial.println("Saving preferences to SPIFFS...");
    Serial.printf(
        "numLeds: %d, universe: %d, startAddress: %d, deviceName: %s, protocol: %d, colorMode: %d, outputMode: %d\n", // <-- MODIFIED
        numLeds, universe, startAddress, deviceName.c_str(), protocol, colorMode, outputMode // <-- MODIFIED
    );

    configFile.printf(
        "{\"numLeds\": %d, \"universe\": %d, \"startAddress\": %d, \"deviceName\": \"%s\", \"protocol\": %d, \"colorMode\": %d, \"outputMode\": %d}\n", // <-- MODIFIED
        numLeds, universe, startAddress, deviceName.c_str(), protocol, colorMode, outputMode // <-- MODIFIED
    );
    configFile.close();
}
