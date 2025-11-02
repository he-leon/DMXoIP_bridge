#include "ConfigParameters.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>

int numLeds                = 30;
int universe               = 0;
int startAddress           = 2;
String chipID              = String((uint32_t) ESP.getEfuseMac(), HEX);
String defaultDeviceName   = "ESP32-" + chipID;
String deviceName          = defaultDeviceName;
ProtocolType protocol =  PROTO_E131;

const char *configFilePath = "/config.json";

std::vector<WiFiConfig> wifiConfigs;  // Holds multiple Wi-Fi configurations
const char *wifiConfigFilePath = "/wifi_config.json";

WiFiManagerParameter custom_numLeds("numLeds",
                                    "Number of LEDs",
                                    String(numLeds).c_str(),
                                    5);
WiFiManagerParameter custom_universe("universe",
                                     "Art-Net Universe",
                                     String(universe).c_str(),
                                     5);
WiFiManagerParameter custom_startAddress("startAddress",
                                         "Start Address",
                                         String(startAddress).c_str(),
                                         5);
WiFiManagerParameter custom_deviceName("deviceName",
                                       "Device Name",
                                       defaultDeviceName.c_str(),
                                       32);
WiFiManagerParameter custom_protocol("protocol",
                                     "Protocol",
                                     String(protocol).c_str(),
                                     1);

                                    

void loadWiFiConfigs()
{
  File configFile = SPIFFS.open(wifiConfigFilePath, FILE_READ);
  if (!configFile)
  {
    Serial.println("No Wi-Fi config file found, using defaults");
    return;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    Serial.println("Failed to read Wi-Fi config file, using defaults");
    return;
  }

  // Clear existing configurations
  wifiConfigs.clear();

  // Load configurations from JSON array
  JsonArray array = doc["wifiConfigs"].as<JsonArray>();
  for (JsonObject obj : array)
  {
    WiFiConfig config;
    config.ssid     = obj["ssid"].as<String>();
    config.password = obj["password"].as<String>();
    config.priority = obj["priority"] | 0;
    wifiConfigs.push_back(config);
  }

  configFile.close();
  Serial.println("Wi-Fi configurations loaded:");
  // Print wifi config
  for (const auto &config : wifiConfigs)
  {
    Serial.println("SSID: " + config.ssid);
    Serial.println("Password: " + config.password);
    Serial.println("Priority: " + String(config.priority));
  }
}

void saveWiFiConfigs()
{
  File configFile = SPIFFS.open(wifiConfigFilePath, FILE_WRITE);
  if (!configFile)
  {
    Serial.println("Failed to open Wi-Fi config file for writing");
    return;
  }

  DynamicJsonDocument doc(1024);
  JsonArray array = doc.createNestedArray("wifiConfigs");

  for (const auto &config : wifiConfigs)
  {
    JsonObject obj  = array.createNestedObject();
    obj["ssid"]     = config.ssid;
    obj["password"] = config.password;
    obj["priority"] = config.priority;
  }

  serializeJson(doc, configFile);
  configFile.close();
  Serial.println("Wi-Fi configurations saved.");
}

void addWiFiConfig(const String &ssid, const String &password, int priority = 0)
{
  WiFiConfig newConfig;
  newConfig.ssid     = ssid;
  newConfig.password = password;
  newConfig.priority = priority;

  // Check if SSID already exists, then update; otherwise, add new
  bool exists = false;
  for (auto &config : wifiConfigs)
  {
    if (config.ssid == ssid)
    {
      config = newConfig;
      exists = true;
      break;
    }
  }

  if (!exists)
    wifiConfigs.push_back(newConfig);

  saveWiFiConfigs();  // Save updated configurations to SPIFFS
}

void initializePreferences()
{
  Serial.println("Initializing preferences...");
  loadWiFiConfigs();
  loadPreferences();
}

void loadPreferences()
{
  Serial.println("Loading preferences from SPIFFS...");

  File configFile = SPIFFS.open(configFilePath, FILE_READ);

  if (!configFile)
  {
    Serial.println("Failed to open config file, using defaults");
    return;
  }

  size_t size = configFile.size();
  if (size > 1024)
  {
    Serial.println("Config file size is too large");
    configFile.close();
    return;
  }

  // Allocate buffer to store contents of the file
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  // Parse JSON
  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, buf.get());
  if (error)
  {
    Serial.println("Failed to parse config file, using defaults");
  }
  else
  {
    numLeds      = json["numLeds"] | 30;
    universe     = json["universe"] | 0;
    startAddress = json["startAddress"] | 2;
    deviceName   = json["deviceName"] | defaultDeviceName;
    protocol     = static_cast<ProtocolType>(json["protocol"] | PROTO_E131);
  }

  configFile.close();
  Serial.println("Loaded preferences:");
  Serial.println("numLeds: " + String(numLeds));
  Serial.println("universe: " + String(universe));
  Serial.println("startAddress: " + String(startAddress));
  Serial.println("deviceName: " + deviceName);
  Serial.println("protocol: " + String(protocol));
}

void savePreferences()
{
  Serial.println("Saving preferences...");
  Serial.println("numLeds: " + String(numLeds));
  Serial.println("universe: " + String(universe));
  Serial.println("startAddress: " + String(startAddress));
  Serial.println("deviceName: " + deviceName);
  Serial.println("protocol: " + String(protocol));
  Serial.println("Saving preferences to SPIFFS...");
  File configFile = SPIFFS.open(configFilePath, FILE_WRITE);
  if (!configFile)
  {
    Serial.println("Failed to open config file for writing");
    return;
  }

  // Write settings as JSON format
  configFile.printf(
      "{\"numLeds\": %d, \"universe\": %d, \"startAddress\": %d, \"deviceName\": \"%s\", \"protocol\": %d}\n",
      numLeds,
      universe,
      startAddress,
      deviceName.c_str(),
      protocol
      );
        
  configFile.close();

  Serial.println("Preferences saved.");
}
