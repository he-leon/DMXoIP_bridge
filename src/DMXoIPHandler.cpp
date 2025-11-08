#include <ArtnetWifi.h>
#include "ConfigParameters.h"
#include "DMXoIPHandler.h"
#include "LEDConfig.h"

static ArtnetWifi artnet;
static ESPAsyncE131 e131;

static RgbColor currentColor(0);
static uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

static uint16_t frameCount = 0;
static uint32_t lastFpsMicros = 0;
static uint16_t currentFps = 0;  // Integer FPS


// ----------------- Frame rate counter -----------------
void DMXoIPHandler::updateFrameRate() {
    frameCount++;
    uint32_t now = micros();
    uint32_t elapsed = now - lastFpsMicros;
    
    if (elapsed >= 100000) {
        currentFps = (frameCount * 1000000UL) / elapsed;  // Pure integer math
        frameCount = 0;
        lastFpsMicros = now;
    }
}

// ----------------- IDMXoIPStatus Implementation -----------------
int DMXoIPHandler::getFrameRate() const {
    return currentFps;
}

bool DMXoIPHandler::isReceiving() const {
    return (millis() - lastPacketTime) < PACKET_TIMEOUT_MS;
}

// ----------------- DMX Frame Handler -----------------
void DMXoIPHandler::onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data) {
    updateFrameRate();  // <--- Update FPS counter
    if (universeIn != ::universe) return;
    lastPacketTime = millis();
    // Used for latency measurements with Python script
    Serial.printf("CH1:%d, FPS:%d\n", data[0], getFrameRate());
    if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length) return;

    static unsigned long lastPowerCalc = 0;

    if (colorMode == COLOR_MODE_SINGLE) {
        RgbColor color;
        bool needsUpdate = false;
        if (USE_DIM_CHANNEL) {
            uint8_t brightness = data[startAddress - 1];
            color = RgbColor(data[startAddress], data[startAddress + 1], data[startAddress + 2]);

            if (brightness != currentBrightness) { currentBrightness = brightness; needsUpdate = true; }
            if (color != currentColor) { currentColor = color; needsUpdate = true; }
        } else {
            color = RgbColor(data[startAddress - 1], data[startAddress], data[startAddress + 1]);
            if (color != currentColor) { currentColor = color; needsUpdate = true; }
        }

        if (needsUpdate) {
            RgbColor scaledColor = color.Dim(currentBrightness);
            for (int i = 0; i < numLeds; i++) strip->SetPixelColor(i, scaledColor);
            strip->Show();
        }

    } else { // COLOR_MODE_MULTIPLE
        for (int i = 0; i < numLeds; i++) {
            RgbColor scaledColor;
            if (USE_DIM_CHANNEL)
            {
                int j = i * 4;
                uint8_t brightness = data[j + startAddress - 1];
                RgbColor color(data[j + startAddress], data[j + startAddress + 1], data[j + startAddress + 2]);
                scaledColor = color.Dim(brightness);
            }
            else
            {
                int j = i * 3;
                RgbColor color(data[j + startAddress - 1], data[j + startAddress], data[j + startAddress + 1]);
                scaledColor = color.Dim(currentBrightness);
            }
            strip->SetPixelColor(i, scaledColor);
        }
        strip->Show();
    }

    if (millis() - lastPowerCalc > 1000) { calculatePowerUsage(); lastPowerCalc = millis(); }
}

// ----------------- Art-Net -----------------
void DMXoIPHandler::setupArtNet() {
    // Need a wrapper lambda/function to capture 'this' for the static callback
    // A C-style callback must be static, or a lambda that captures 'this'.
    // Since ArtnetWifi requires a C-style function pointer, we'll use a static
    // function and a global/static pointer to the instance if multiple instances
    // aren't needed, or a capture-less lambda if the library supports it.
    // For now, let's keep it simple with a C++ lambda (if the library supports C++ lambda as callback)
    // or a static proxy function. ArtnetWifi's setArtDmxCallback takes a C-style function.
    // For now, let's use a static proxy that assumes a single instance, which is common in Arduino.
    
    // ArtnetWifi doesn't directly support a class method as a callback, 
    // so we need a static proxy and a way to access the instance.
    // For simplicity in this refactor, let's assume one instance is used globally.
    // **NOTE**: In a more complex design, you'd use a singleton pattern or a global pointer.
    
    // Temporary static proxy for the callback to call the member function.
    static DMXoIPHandler* currentInstance = this;
    
    artnet.begin();
    artnet.setArtDmxCallback([](uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
        if (currentInstance) {
            currentInstance->onDmxFrame(universe, length, sequence, data);
        }
    });
    Serial.println("Art-Net initialized and ready.");
}

void DMXoIPHandler::readArtNet() { artnet.read(); }

// ----------------- E1.31 -----------------
void DMXoIPHandler::handleE131Packet(e131_packet_t* packet) {
    uint16_t universe = htons(packet->universe);
    uint16_t length = htons(packet->property_value_count) - 1;
    uint8_t* data = packet->property_values + 1;
    onDmxFrame(universe, length, packet->sequence_number, data);
}

void DMXoIPHandler::setupE131() {
    // Similar problem with E1.31 library's callback. ESPAsyncE131 uses a poll method.
    if (e131.begin(E131_MULTICAST, universe=universe)) Serial.println("E1.31 receiver initialized");
    else Serial.println("E1.31 failed to start!");
}

void DMXoIPHandler::readE131() {
    if (!e131.isEmpty()) {
        e131_packet_t packet;
        while (!e131.isEmpty()) { 
            e131.pull(&packet); 
            handleE131Packet(&packet); 
        }
    }
}
