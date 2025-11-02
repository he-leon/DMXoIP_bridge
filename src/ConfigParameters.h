#ifndef CONFIGPARAMETERS_H
#define CONFIGPARAMETERS_H

#include <WiFiManager.h>

#include "WiFiConfig.h"

#define LED_PIN 5
#define DEFAULT_BRIGHTNESS 50
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

extern int numLeds;
extern int universe;
extern int startAddress;
extern String deviceName;
enum ProtocolType { PROTO_ARTNET = 0, PROTO_E131 = 1 };
extern ProtocolType protocol;



// WiFiManager custom parameters
extern WiFiManagerParameter custom_numLeds;
extern WiFiManagerParameter custom_universe;
extern WiFiManagerParameter custom_startAddress;
extern WiFiManagerParameter custom_deviceName;
extern WiFiManagerParameter custom_protocol;
extern WiFiManagerParameter custom_protocol_select;

void initializePreferences();  // Add this line
void loadPreferences();
void savePreferences();
void loadWiFiConfigs();
void saveWiFiConfigs();
void addWiFiConfig(const String &ssid, const String &password, int priority);

extern std::vector<WiFiConfig> wifiConfigs;

#endif  // CONFIGPARAMETERS_H
