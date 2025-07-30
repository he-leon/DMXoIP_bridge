#include "LEDConfig.h"
#include "ConfigParameters.h"

#define LED_PIN 5  // Replace or keep depending on your setup

// Use RMT with 800kHz timing (standard for WS2812)
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> strip(numLeds, LED_PIN);


float totalPower = 0.0;

// Store current brightness separately, since NeoPixelBus handles brightness differently
static uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

void setupLEDs()
{
  strip.Begin();
  strip.Show();  // Initialize all LEDs off
}

void setLEDColor(RgbColor color)
{
  RgbColor scaledColor = color;
  scaledColor.Dim(currentBrightness);  // Apply brightness scaling

  for (int i = 0; i < numLeds; i++)
  {
    strip.SetPixelColor(i, scaledColor);
  }

  strip.Show();
}

void setBrightness(uint8_t brightness)
{
  currentBrightness = brightness;
  // Brightness is applied during setLEDColor() via Dim()
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
    RgbColor color = strip.GetPixelColor(i);
    float redPower   = color.R / 255.0 * RED_POWER;
    float greenPower = color.G / 255.0 * GREEN_POWER;
    float bluePower  = color.B / 255.0 * BLUE_POWER;
    totalPower += redPower + greenPower + bluePower;
  }

  // Scale by brightness level (which was pre-applied)
  totalPower *= currentBrightness / 255.0;

  Serial.print("Current power usage: ");
  Serial.print(totalPower);
  Serial.println(" W");
}

