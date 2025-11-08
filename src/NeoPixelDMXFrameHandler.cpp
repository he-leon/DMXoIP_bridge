#include "NeoPixelDMXFrameHandler.h" // <-- Include the new header
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include <Arduino.h>

static uint16_t frameCount = 0;
static uint32_t lastFpsMicros = 0;
static uint16_t currentFps = 0;  // Integer FPS

NeoPixelDMXFrameHandler::NeoPixelDMXFrameHandler() 
    : currentBrightness(DEFAULT_BRIGHTNESS)
{}

// ----------------- IDMXFrameHandler Implementation -----------------
void NeoPixelDMXFrameHandler::handleFrame(uint16_t universeIn, uint16_t length, uint8_t* data) {
    if (universeIn != ::universe) return;
    updateFrameRate();  // Update FPS counter
    Serial.printf("CH1:%d, FPS:%d\n", data[0], getFrameRate());
    
    // Check if enough data is available for the simplest case (R, G, B channels)
    if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length) return;

    if (colorMode == COLOR_MODE_SINGLE) {
        RgbColor color;
        bool needsUpdate = false;
        
        if (USE_DIM_CHANNEL) {
            uint8_t brightness = data[startAddress - 1];
            color = RgbColor(data[startAddress], data[startAddress + 1], data[startAddress + 2]);

            if (brightness != currentBrightness) { currentBrightness = brightness; needsUpdate = true; }
            if (color != currentColor) { currentColor = color; needsUpdate = true; }
        } else {
            // No dim channel: R, G, B start at startAddress-1
            color = RgbColor(data[startAddress - 1], data[startAddress], data[startAddress + 1]);
            if (color != currentColor) { currentColor = color; needsUpdate = true; }
        }

        if (needsUpdate) {
            RgbColor scaledColor = currentColor.Dim(currentBrightness);
            for (int i = 0; i < numLeds; i++) strip->SetPixelColor(i, scaledColor);
            strip->Show();
        }

    } else { // COLOR_MODE_MULTIPLE
        for (int i = 0; i < numLeds; i++) {
            RgbColor scaledColor;
            int offset = i * (USE_DIM_CHANNEL ? 4 : 3);
            
            if (offset + startAddress - 1 + (USE_DIM_CHANNEL ? 3 : 2) >= length) {
                // Not enough data for this LED, stop processing
                break;
            }

            if (USE_DIM_CHANNEL)
            {
                uint8_t brightness = data[offset + startAddress - 1];
                RgbColor color(data[offset + startAddress], data[offset + startAddress + 1], data[offset + startAddress + 2]);
                scaledColor = color.Dim(brightness);
            }
            else
            {
                RgbColor color(data[offset + startAddress - 1], data[offset + startAddress], data[offset + startAddress + 1]);
                // Use global/default brightness if no dim channel is configured
                scaledColor = color.Dim(currentBrightness); 
            }
            strip->SetPixelColor(i, scaledColor);
        }
        strip->Show();
    }

    // Power calculation (moved from DMXoIPHandler)
    if (millis() - lastPowerCalc > 1000) { 
        calculatePowerUsage(); 
        lastPowerCalc = millis(); 
    }
}

// ----------------- IDMXoIPStatus Implementation -----------------
int NeoPixelDMXFrameHandler::getFrameRate() const {
    return currentFps;
}

// ----------------- Frame rate counter -----------------
void NeoPixelDMXFrameHandler::updateFrameRate() {
    frameCount++;
    uint32_t now = micros();
    uint32_t elapsed = now - lastFpsMicros;
    
    if (elapsed >= 100000) {
        currentFps = (frameCount * 1000000UL) / elapsed;  // Pure integer math
        frameCount = 0;
        lastFpsMicros = now;
    }
}
