#ifndef ESP_DMX_NOW_FRAME_HANDLER_H
#define ESP_DMX_NOW_FRAME_HANDLER_H

#include "interfaces/IDMXFrameHandler.h"
#ifndef UNIT_TEST
#include "DMX_ESPNOW.h" // Include the external library header
#else
#include "fakes/MockDMXoIPLibs.h" // Include the mock version for unit tests
#endif

/**
 * @brief An implementation of IDMXFrameHandler that uses the DMX_ESPNOW library
 * to buffer DMX data for transmission over ESP-NOW.
 * This class acts as the DMX sender.
 */
class ESPDMXNowFrameHandler : public IDMXFrameHandler {
public:
    /**
     * @brief Constructor.
     * @param dmxEspNow An initialized instance of the DMX_ESPNOW sender.
     */
    ESPDMXNowFrameHandler(DMX_ESPNOW& dmxEspNow);

    /**
     * @brief Buffers the incoming DMX data into the DMX_ESPNOW library's buffer.
     * @param universeIn The universe number (ignored, as DMX_ESPNOW supports one).
     * @param length The length of the DMX data (up to 512).
     * @param data Pointer to the DMX data array.
     */
    void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override;

    /**
     * @brief Returns the frame rate set for the ESP-NOW sender.
     * Note: DMX_ESPNOW library doesn't expose a getter for refreshRate,
     * so we'll store and return a default/fixed rate.
     */
    int getFrameRate() const override;

private:
    DMX_ESPNOW& _dmxEspNow;
    const int _defaultFrameRate = 40; // Assuming a typical ESP-NOW target rate (Hz)
};

#endif // ESP_DMX_NOW_FRAME_HANDLER_H
