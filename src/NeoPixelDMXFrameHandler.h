#ifndef NEOPIXELDMXFRAMEHANDLER_H
#define NEOPIXELDMXFRAMEHANDLER_H

#include <stdint.h>
#include <NeoPixelBus.h>
#include "interfaces/IDMXFrameHandler.h" // <-- Include the new interface

/**
 * @brief Handles DMX frame processing specifically for NeoPixelBus (WS2812B) strips.
 */
class NeoPixelDMXFrameHandler : public IDMXFrameHandler { // <-- Implement the interface
public:
    NeoPixelDMXFrameHandler();
    
    /**
     * @brief Processes the DMX data and updates the LED strip.
     */
    void handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) override;

private:
    // Tracking for single color mode to optimize updates
    RgbColor currentColor = RgbColor(0);
    uint8_t currentBrightness = 0; // Default brightness (0-255)
    
    // Power calculation tracking
    unsigned long lastPowerCalc = 0;
};

#endif // NEOPIXELDMXFRAMEHANDLER_H
