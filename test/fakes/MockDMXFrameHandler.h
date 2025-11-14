#ifndef MOCK_DMXFRAMEHANDLER_H
#define MOCK_DMXFRAMEHANDLER_H

#include "ArduinoFake.h"  // For basic types
#include "interfaces/IDMXFrameHandler.h"

class MockDMXFrameHandler : public IDMXFrameHandler
{
 public:
  // Simple tracking variables (Counters/Storage)
  int callCount         = 0;
  uint16_t lastUniverse = 0;
  uint16_t lastLength   = 0;
  uint8_t lastData[512] = {0};
  int fps               = 0;

  void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override
  {
    callCount++;
    lastUniverse = universeIn;
    lastLength   = length;

    // Copy a small amount of data for basic verification (up to 512 bytes)
    size_t bytesToCopy = (length > 512) ? 512 : length;
    if (data != nullptr)
    {
      memcpy(lastData, data, bytesToCopy);
    }
    else
    {
      memset(lastData, 0, bytesToCopy);
    }
  }

  int getFrameRate() const override { return fps; }

  // Test Utility: Check if handleFrame was called exactly N times
  bool wasCalled(int expectedCount) const { return callCount == expectedCount; }
};

#endif  // MOCK_DMXFRAMEHANDLER_H
