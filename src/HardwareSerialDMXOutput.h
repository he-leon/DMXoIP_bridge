#ifndef HARDWARE_SERIAL_DMX_OUTPUT_H
#define HARDWARE_SERIAL_DMX_OUTPUT_H
#define MIN_FRAME_INTERVAL_MS 30
#define MAX_DMX_SLOTS 512

#include <Arduino.h>

#include "interfaces/IDMXOutput.h"

class HardwareSerialDMXOutput : public IDMXOutput
{
 public:
  HardwareSerialDMXOutput(HardwareSerial& serialPort);

  // Buffers the data; does NOT send it
  void writeFrame(uint16_t length, uint8_t* data) override;

  // Sends the buffered DMX frame
  void sendDMX();

  // Updates persistence logic (call in loop)
  void updatePersistence();

 private:
  HardwareSerial& _serialPort;

  static constexpr uint16_t DMX_BREAK_TIME_US = 92;
  static constexpr uint16_t DMX_MAB_TIME_US   = 12;
  static constexpr long DMX_BAUD_RATE         = 250000;
  static constexpr uint32_t PERSIST_DELAY_MS  = 10000;  // 10 seconds

  uint8_t _dmxBuffer[MAX_DMX_SLOTS + 1];
  uint16_t _dmxLength = 0;

  // State persistence
  uint8_t _lastPersistedBuffer[MAX_DMX_SLOTS + 1];
  uint16_t _lastPersistedLength = 0;
  uint32_t _lastChangeMillis    = 0;
  bool _stateChanged            = false;

  void loadDMXState();
  void saveDMXState();
  bool hasStateChanged() const;
};

#endif  // HARDWARE_SERIAL_DMX_OUTPUT_H
