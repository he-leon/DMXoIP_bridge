
#include <FastLED.h>
#include "ConfigParameters.h"

CRGB* leds;

void setupLEDs() {
  leds = new CRGB[numLeds];
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, numLeds).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);
  fill_solid(leds, numLeds, CRGB::Black);
  FastLED.show();
}

void setLEDColor(CRGB color) {
  fill_solid(leds, numLeds, color);
  FastLED.show();
}

void setBrightness(uint8_t brightness) {
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void blinkGreenTwice() {
  for (int i = 0; i < 2; i++) {
    setLEDColor(CRGB::Green);
    delay(500);
    setLEDColor(CRGB::Black);
    delay(500);
  }
}

