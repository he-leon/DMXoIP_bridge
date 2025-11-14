#ifndef IDMX_OUTPUT_H
#define IDMX_OUTPUT_H

#include <cstdint>

/**
 * @brief Interface for writing DMX data to a physical output (e.g., UART).
 * * Abstracts the hardware-specific details like setting baud rates,
 * controlling break conditions, and sending the byte stream.
 */
class IDMXOutput
{
 public:
  virtual ~IDMXOutput() = default;

  /**
   * @brief Writes a complete DMX frame to the output.
   * * @param length The number of DMX slots (1-512).
   * @param data Pointer to the DMX slot data (excluding the start code).
   */
  virtual void writeFrame(uint16_t length, uint8_t* data) = 0;
};

#endif  // IDMX_OUTPUT_H
