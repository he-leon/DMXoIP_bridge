#include "ESPDMXNowFrameHandler.h"
#include <algorithm> // For std::min

/**
 * @brief Constructor.
 * @param dmxEspNow An initialized instance of the DMX_ESPNOW sender.
 */
ESPDMXNowFrameHandler::ESPDMXNowFrameHandler(DMX_ESPNOW& dmxEspNow)
    : _dmxEspNow(dmxEspNow) {
    // The DMX_ESPNOW instance should be initialized as a sender externally.
}

/**
 * @brief Buffers the incoming DMX data into the DMX_ESPNOW library's buffer.
 * DMX_ESPNOW takes care of slicing this buffer into ESP-NOW packets 
 * when its update/sendFrame methods are called.
 * * @param universeIn The universe number (ignored).
 * @param length The length of the DMX data (up to 512).
 * @param data Pointer to the DMX data array.
 */
void ESPDMXNowFrameHandler::handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) {
    // 1. Determine the number of channels to set (clamp to DMX_UNIVERSE_SIZE, which is 512)
    uint16_t count = std::min((uint16_t)DMX_UNIVERSE_SIZE, length);
    
    _dmxEspNow.setChannels(
        0,       // startChannel: DMX channels start at 1
        data,    // values: The source data array
        count    // count: Number of channels to update
    );
    _dmxEspNow.sendFrame();
}

/**
 * @brief Returns the frame rate set for the ESP-NOW sender.
 * @return int The assumed frame rate in Hz.
 */
int ESPDMXNowFrameHandler::getFrameRate() const {
    return _defaultFrameRate;
}
