#ifndef MOCK_DMXOIPLIBS_H
#define MOCK_DMXOIPLIBS_H

#include "ArduinoFake.h"

#define E131_MULTICAST 0
#define DMX_UNIVERSE_SIZE 512

// --- ArtnetWifi Mock ---

// Signature of the callback function that ArtnetWifi passes frames to
typedef void (*ArtnetDmxCallback)(uint16_t universe,
                                  uint16_t length,
                                  uint8_t sequence,
                                  uint8_t* data);

class ArtnetWifi
{
 public:
  ArtnetDmxCallback storedCallback = nullptr;
  bool beginCalled                 = false;

  void begin() { beginCalled = true; }
  void setArtDmxCallback(ArtnetDmxCallback cb) { storedCallback = cb; }
  void read() {}  // No-op for now

  // Helper for tests to manually trigger the callback
  void triggerDmxCallback(uint16_t universe,
                          uint16_t length,
                          uint8_t sequence,
                          uint8_t* data)
  {
    if (storedCallback)
    {
      storedCallback(universe, length, sequence, data);
    }
  }
};

// --- ESPAsyncE131 Mock ---

class ESPAsyncE131
{
 public:
  bool beginCalled  = false;
  bool isEmptyValue = true;
  e131_packet_t nextPacket;  // Packet to be returned by pull()

  // Note: DMXoIPHandler::setupE131 calls begin(E131_MULTICAST)
  bool begin(uint8_t mode)
  {
    beginCalled = true;
    return true;
  }

  // DMXoIPHandler calls isEmpty()
  bool isEmpty() { return isEmptyValue; }

  // DMXoIPHandler calls pull(&packet)
  bool pull(e131_packet_t* packet)
  {
    if (!isEmptyValue)
    {
      memcpy(packet, &nextPacket, sizeof(e131_packet_t));
      isEmptyValue = true;  // Typically, it empties after a pull
      return true;
    }
    return false;
  }
};

// --- DMX_ESPNOW Mock ---

class DMX_ESPNOW
{
 public:
  bool beginSenderCalled               = false;
  bool beginSenderReturnValue          = true;
  uint8_t dmxBuffer[DMX_UNIVERSE_SIZE] = {0};
  uint8_t receiveUniverseId            = 0;

  bool beginSender(uint8_t channel)
  {
    beginSenderCalled = true;
    return beginSenderReturnValue;
  }

  bool beginReceiver(void (*callback)(uint8_t universeId))
  {
    return true;  // Assume success for mock
  }

  void setChannels(uint16_t startChannel, uint8_t* values, uint16_t count)
  {
    for (uint16_t i = 0; i < count && (startChannel + i) < DMX_UNIVERSE_SIZE; i++)
    {
      dmxBuffer[startChannel + i] = values[i];
    }
  }

  void sendFrame()
  {
    // No-op for now
  }

  uint8_t* getDMXBuffer() { return dmxBuffer; }

  void setReceiveUniverseId(uint8_t universeId) { receiveUniverseId = universeId; }

  void update()
  {
    // No-op for now
  }
};

#endif  // MOCK_DMXOIPLIBS_H
