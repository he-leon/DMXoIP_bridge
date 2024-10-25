
#ifndef CONFIGPARAMETERS_H
#define CONFIGPARAMETERS_H

#include <WiFiManager.h>
#include <Preferences.h>

#define LED_PIN 5
#define DEFAULT_BRIGHTNESS 50
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

extern int numLeds;
extern int universe;
extern int startAddress;
extern String deviceName;

// WiFiManager custom parameters
extern WiFiManagerParameter custom_numLeds;
extern WiFiManagerParameter custom_universe;
extern WiFiManagerParameter custom_startAddress;
extern WiFiManagerParameter custom_deviceName;

// Preferences
extern Preferences preferences;

void initializePreferences(); // Add this line
void loadPreferences();
void savePreferences();

#endif // CONFIGPARAMETERS_H

