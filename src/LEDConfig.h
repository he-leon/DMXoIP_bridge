
#ifndef LEDCONFIG_H
#define LEDCONFIG_H

#include <FastLED.h>

void setupLEDs();
void setLEDColor(CRGB color);
void setBrightness(uint8_t brightness);
void blinkGreenTwice();

#endif // LEDCONFIG_H

