#ifndef IDMXFRAMEHANDLER_H
#define IDMXFRAMEHANDLER_H

#include <stdint.h>

/**
 * @brief Interface for any class responsible for decoding a DMX frame
 * and controlling an output device (like LEDs).
 */
class IDMXFrameHandler
{
 public:
  virtual ~IDMXFrameHandler() = default;

  /**
   * @brief Processes the DMX data and updates the output device.
   * @param universeIn The universe number.
   * @param length The length of the DMX data (up to 512).
   * @param data Pointer to the DMX data array.
   */
  virtual void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) = 0;
  virtual int getFrameRate() const                                              = 0;
};

#endif  // IDMXFRAMEHANDLER_H
