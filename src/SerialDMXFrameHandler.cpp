#include "SerialDMXFrameHandler.h"
#ifdef UNIT_TEST
#include "fakes/ArduinoFake.h"
#else
#include <Arduino.h>
#endif

static uint16_t frameCount    = 0;
static uint32_t lastFpsMicros = 0;
static uint16_t currentFps    = 0;  // Integer FPS

SerialDMXFrameHandler::SerialDMXFrameHandler(IDMXOutput& dmxOutput,
                                             uint16_t targetUniverse)
    : _dmxOutput(dmxOutput), _targetUniverse(targetUniverse)
{
}

void SerialDMXFrameHandler::handleFrame(uint16_t universeIn,
                                        uint16_t length,
                                        uint8_t* data)
{
  if (universeIn != _targetUniverse)
  {
    return;
  }

  if (length == 0 || length > 512)
  {
    Serial.printf("Warning: Received DMX frame for U%u with invalid length %u.\n",
                  universeIn,
                  length);
    return;
  }
  _dmxOutput.writeFrame(length, data);
}

// ----------------- IDMXoIPStatus Implementation -----------------
int SerialDMXFrameHandler::getFrameRate() const { return currentFps; }

// ----------------- Frame rate counter -----------------
void SerialDMXFrameHandler::updateFrameRate()
{
  frameCount++;
  uint32_t now     = micros();
  uint32_t elapsed = now - lastFpsMicros;

  if (elapsed >= 100000)
  {
    currentFps    = (frameCount * 1000000UL) / elapsed;  // Pure integer math
    frameCount    = 0;
    lastFpsMicros = now;
  }
}
