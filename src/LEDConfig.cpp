#include "LEDConfig.h"

#include "ConfigParameters.h"

#define LED_PIN 16  // Replace or keep depending on your setup

float totalPower = 0.0;

// Store current brightness separately, since NeoPixelBus handles brightness differently
static uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

NeoPixelBus<ActiveNeoFeature, NeoEsp32I2s0Ws2812xMethod>* strip = nullptr;

void setupLEDs()
{
  if (strip)
    delete strip;
  strip = new NeoPixelBus<ActiveNeoFeature, NeoEsp32I2s0Ws2812xMethod>(numLeds, LED_PIN);
  strip->Begin();
  strip->Show();
}

void setLEDColor(RgbColor color)
{
  RgbColor scaledColor = color;
  scaledColor.Dim(currentBrightness);  // Apply brightness scaling

  for (int i = 0; i < numLeds; i++)
  {
    strip->SetPixelColor(i, scaledColor);
  }

  strip->Show();
}

void setBrightness(uint8_t brightness)
{
  // Brightness is applied during setLEDColor() via Dim()
  currentBrightness = brightness;
}

void blinkGreenTwice()
{
  RgbColor green(0, 255, 0);
  RgbColor off(0);

  for (int i = 0; i < 2; i++)
  {
    setLEDColor(green);
    delay(500);
    setLEDColor(off);
    delay(500);
  }
}

void calculatePowerUsage()
{
  totalPower = 0.0;

  for (int i = 0; i < numLeds; i++)
  {
    RgbColor color   = strip->GetPixelColor(i);
    float redPower   = color.R / 255.0 * RED_POWER;
    float greenPower = color.G / 255.0 * GREEN_POWER;
    float bluePower  = color.B / 255.0 * BLUE_POWER;
    totalPower += redPower + greenPower + bluePower;
  }

  // Scale by brightness level (which was pre-applied)
  totalPower *= currentBrightness / 255.0;
}
