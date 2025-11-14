#ifndef ESP_DMX_NOW_FRAME_HANDLER_H
#define ESP_DMX_NOW_FRAME_HANDLER_H

#include "DMX_ESPNOW.h"
#include "interfaces/IDMXFrameHandler.h"

class ESPDMXNowFrameHandler : public IDMXFrameHandler
{
 public:
  ESPDMXNowFrameHandler(DMX_ESPNOW& dmxEspNow);
  void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override;
  int getFrameRate() const override;

 private:
  DMX_ESPNOW& _dmxEspNow;
  const int _defaultFrameRate = 40;
};

#endif  // ESP_DMX_NOW_FRAME_HANDLER_H
