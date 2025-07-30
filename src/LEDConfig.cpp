
#include "LEDConfig.h"

#include <FastLED.h>

#include "ConfigParameters.h"

float totalPower = 0.0;  // Define totalPower here
CRGB *leds;

void setupLEDs()
{
  leds = new CRGB[numLeds];
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, numLeds)
      .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(DEFAULT_BRIGHTNESS);
  fill_solid(leds, numLeds, CRGB::Black);
  FastLED.show();
}

void setLEDColor(CRGB color)
{
  fill_solid(leds, numLeds, color);
}

void setBrightness(uint8_t brightness)
{
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void blinkGreenTwice()
{
  for (int i = 0; i < 2; i++)
  {
    setLEDColor(CRGB::Green);
    delay(500);
    setLEDColor(CRGB::Black);
    delay(500);
  }
}

void calculatePowerUsage()
{
  totalPower = 0.0;
  for (int i = 0; i < numLeds; i++)
  {
    float redPower   = leds[i].r / 255.0 * RED_POWER;
    float greenPower = leds[i].g / 255.0 * GREEN_POWER;
    float bluePower  = leds[i].b / 255.0 * BLUE_POWER;
    totalPower += redPower + greenPower + bluePower;
  }

  // Adjust for global brightness setting
  totalPower *= FastLED.getBrightness() / 255.0;

  Serial.print("Current power usage: ");
  Serial.print(totalPower);
  Serial.println(" W");
}
