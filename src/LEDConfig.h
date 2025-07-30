#ifndef LEDCONFIG_H
#define LEDCONFIG_H

#include <NeoPixelBus.h>

#define RED_POWER 0.110    // Power per LED at full brightness for Red
#define GREEN_POWER 0.079  // Power per LED at full brightness for Green
#define BLUE_POWER 0.103   // Power per LED at full brightness for Blue

void setupLEDs();
void setLEDColor(RgbColor color);
void setBrightness(uint8_t brightness);
void blinkGreenTwice();
void calculatePowerUsage();

extern float totalPower;
extern NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod>* strip;

#endif  // LEDCONFIG_H
