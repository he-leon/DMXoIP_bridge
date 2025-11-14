#ifndef NEOPIXELDMXFRAMEHANDLER_H
#define NEOPIXELDMXFRAMEHANDLER_H

#include <NeoPixelBus.h>
#include <stdint.h>

#include "interfaces/IDMXFrameHandler.h"  // <-- Include the new interface

class NeoPixelDMXFrameHandler : public IDMXFrameHandler
{  // <-- Implement the interface
 public:
  NeoPixelDMXFrameHandler();

  void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override;
  int getFrameRate() const override;

 private:
  // Tracking for single color mode to optimize updates
  RgbColor currentColor     = RgbColor(0);
  uint8_t currentBrightness = 0;  // Default brightness (0-255)

  // Power calculation tracking
  unsigned long lastPowerCalc  = 0;
  unsigned long lastPacketTime = 0;

  void updateFrameRate();
};

#endif  // NEOPIXELDMXFRAMEHANDLER_H
