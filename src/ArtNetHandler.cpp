
#include <ArtnetWifi.h>

#include "ConfigParameters.h"
#include "LEDConfig.h"

ArtnetWifi artnet;
CRGB currentColor;
uint8_t currentBrightness;

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  if (universe == ::universe)
  {
    uint8_t brightness = data[startAddress-1];
    CRGB color         = CRGB::Black;

    if (startAddress + 2 < length)
    {
      color = CRGB(data[startAddress], data[startAddress + 1], data[startAddress + 2]);
    }
    if (currentBrightness != brightness)
    {
      currentBrightness = brightness;
      setBrightness(brightness);
      calculatePowerUsage();
    }
    if (currentColor != color)
    {
      currentColor = color;
      setLEDColor(color);
      calculatePowerUsage();
    }
  }
}

void setupArtNet()
{
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
  Serial.println("Art-Net initialized and ready to receive data.");
}

void readArtNet() { artnet.read(); }
