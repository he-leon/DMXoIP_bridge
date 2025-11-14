#include "ESPDMXNowFrameHandler.h"

#include <algorithm>

ESPDMXNowFrameHandler::ESPDMXNowFrameHandler(DMX_ESPNOW& dmxEspNow)
    : _dmxEspNow(dmxEspNow)
{
}

void ESPDMXNowFrameHandler::handleFrame(uint16_t universeIn,
                                        uint16_t length,
                                        uint8_t* data)
{
  uint16_t count = std::min((uint16_t) DMX_UNIVERSE_SIZE, length);

  _dmxEspNow.setChannels(0, data, count);
  _dmxEspNow.sendFrame();
}

int ESPDMXNowFrameHandler::getFrameRate() const
{
  // Not implemented; return a default value
  return _defaultFrameRate;
}
