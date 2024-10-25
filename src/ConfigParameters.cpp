
#include "ConfigParameters.h"

int numLeds = 30;
int universe = 0;
int startAddress = 2;
String chipID = String((uint32_t)ESP.getEfuseMac(), HEX);
String defaultDeviceName = "ESP32-" + chipID;
String deviceName = defaultDeviceName;

WiFiManagerParameter custom_numLeds("numLeds", "Number of LEDs", String(numLeds).c_str(), 5);
WiFiManagerParameter custom_universe("universe", "Art-Net Universe", String(universe).c_str(), 5);
WiFiManagerParameter custom_startAddress("startAddress", "Start Address", String(startAddress).c_str(), 5);
WiFiManagerParameter custom_deviceName("deviceName", "Device Name", defaultDeviceName.c_str(), 32);

Preferences preferences;

void initializePreferences() {
  Serial.println("Initializing preferences...");
  preferences.begin("config", false);
  loadPreferences();
}

void loadPreferences() {
  numLeds = preferences.getInt("numLeds", 30);
  universe = preferences.getInt("universe", 0);
  startAddress = preferences.getInt("startAddress", 2);
  deviceName = preferences.getString("deviceName", defaultDeviceName);
  Serial.println("Loaded preferences:");
  Serial.println("numLeds: " + String(numLeds));
  Serial.println("universe: " + String(universe));
  Serial.println("startAddress: " + String(startAddress));
  Serial.println("deviceName: " + deviceName);
}

void savePreferences() {
  Serial.println("Saving preferences...");
  Serial.println("numLeds: " + String(numLeds));
  Serial.println("universe: " + String(universe));
  Serial.println("startAddress: " + String(startAddress));
  Serial.println("deviceName: " + deviceName);
  preferences.putInt("numLeds", numLeds);
  preferences.putInt("universe", universe);
  preferences.putInt("startAddress", startAddress);
  preferences.putString("deviceName", deviceName);
}

