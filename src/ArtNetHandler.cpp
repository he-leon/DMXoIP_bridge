#include <ArtnetWifi.h>
#include "ConfigParameters.h"
#include "LEDConfig.h"

ArtnetWifi artnet;

RgbColor currentColor(0);
uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universeIn != ::universe)
    return;

  if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length)
    return;

  uint8_t brightness = data[startAddress - 1];
  RgbColor color(
    data[startAddress],         // Red
    data[startAddress + 1],     // Green
    data[startAddress + 2]);    // Blue

  bool needsUpdate = false;

  if (brightness != currentBrightness) {
    currentBrightness = brightness;
    setBrightness(brightness);
    needsUpdate = true;
  }

  if (color != currentColor) {
    currentColor = color;
    setLEDColor(color);
    needsUpdate = true;
  }

  if (needsUpdate) {
    calculatePowerUsage();
  }
}

void setupArtNet()
{
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
  Serial.println("Art-Net initialized and ready to receive data.");
}

void readArtNet()
{
  artnet.read();
}

