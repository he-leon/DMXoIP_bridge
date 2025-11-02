#include <ArtnetWifi.h>
#include "ConfigParameters.h"
#include "LEDConfig.h"

ArtnetWifi artnet;

RgbColor currentColor(0);
uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

#ifdef SINGLE_COLOR
void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universeIn != ::universe)
    return;

  if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length)
    return;

  uint8_t brightness = data[startAddress - 1];
  RgbColor color(
    data[startAddress],
    data[startAddress + 1],
    data[startAddress + 2]);

  bool needsUpdate = false;
  
  if (brightness != currentBrightness) {
    currentBrightness = brightness;
    needsUpdate = true;
  }

  if (color != currentColor) {
    currentColor = color;
    needsUpdate = true;
  }

  if (needsUpdate) {
    RgbColor scaledColor = currentColor;
    scaledColor.Dim(currentBrightness);
    
    for (int i = 0; i < numLeds; i++) {
      strip->SetPixelColor(i, scaledColor);
    }
    strip->Show();
    
    // Only calculate power occasionally, not on every frame
    static unsigned long lastPowerCalc = 0;
    if (millis() - lastPowerCalc > 1000) { // Every second
      calculatePowerUsage();
      lastPowerCalc = millis();
    }
  }
}
#else

void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data)
{
  if (universeIn != ::universe)
    return;

  if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length)
    return;


  for (int i = 0; i < numLeds; i++) {
      uint8_t brightness = data[i + startAddress - 1];
      RgbColor color(
        data[i + startAddress],
        data[i + startAddress + 1],
        data[i + startAddress + 2]);

      bool needsUpdate = false;
      
      if (brightness != currentBrightness) {
        currentBrightness = brightness;
        needsUpdate = true;
      }

      if (color != currentColor) {
        currentColor = color;
        needsUpdate = true;
      }

      if (needsUpdate) {
        RgbColor scaledColor = currentColor;
        scaledColor.Dim(currentBrightness);
        strip->SetPixelColor(i, scaledColor);
      }
    }

    strip->Show();
    
    // Only calculate power occasionally, not on every frame
    static unsigned long lastPowerCalc = 0;
    if (millis() - lastPowerCalc > 1000) { // Every second
      calculatePowerUsage();
      lastPowerCalc = millis();
    }
  }
#endif

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

