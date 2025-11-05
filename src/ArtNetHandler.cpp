#include <ArtnetWifi.h>
#include "ConfigParameters.h"
#include "LEDConfig.h"
#include <ESPAsyncE131.h>

ArtnetWifi artnet;
ESPAsyncE131 e131;

RgbColor currentColor(0);
uint8_t currentBrightness = DEFAULT_BRIGHTNESS;

void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data) {
    if (universeIn != ::universe) return;
    if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length) return;

    static unsigned long lastPowerCalc = 0;

    if (colorMode == COLOR_MODE_SINGLE) {
        uint8_t brightness = data[startAddress - 1];
        RgbColor color(data[startAddress], data[startAddress + 1], data[startAddress + 2]);
        bool needsUpdate = false;

        if (brightness != currentBrightness) { currentBrightness = brightness; needsUpdate = true; }
        if (color != currentColor) { currentColor = color; needsUpdate = true; }

        if (needsUpdate) {
            RgbColor scaledColor = currentColor;
            scaledColor.Dim(currentBrightness);
            for (int i = 0; i < numLeds; i++) strip->SetPixelColor(i, scaledColor);
            strip->Show();
        }

    } else { // COLOR_MODE_MULTIPLE
        for (int i = 0; i < numLeds; i++) {
            uint8_t brightness = data[i + startAddress - 1];
            RgbColor color(data[i + startAddress], data[i + startAddress + 1], data[i + startAddress + 2]);
            RgbColor scaledColor = color;
            scaledColor.Dim(brightness);
            strip->SetPixelColor(i, scaledColor);
        }
        strip->Show();
    }

    if (millis() - lastPowerCalc > 1000) { calculatePowerUsage(); lastPowerCalc = millis(); }
}

// ----------------- Art-Net -----------------
void setupArtNet() {
    artnet.begin();
    artnet.setArtDmxCallback(onDmxFrame);
    Serial.println("Art-Net initialized and ready.");
}
void readArtNet() { artnet.read(); }

// ----------------- E1.31 -----------------
void setupE131() {
    if (e131.begin(E131_MULTICAST)) Serial.println("E1.31 receiver initialized");
    else Serial.println("E1.31 failed to start!");
}

void handleE131Packet(e131_packet_t* packet) {
    Serial.println("E1.31 packet received");
    uint16_t universe = htons(packet->universe);
    uint16_t length = htons(packet->property_value_count) - 1;
    uint8_t* data = packet->property_values + 1;
    onDmxFrame(universe, length, packet->sequence_number, data);
}

void readE131() {
    if (!e131.isEmpty()) {
        e131_packet_t packet;
        while (!e131.isEmpty()) { e131.pull(&packet); handleE131Packet(&packet); }
    }
}

