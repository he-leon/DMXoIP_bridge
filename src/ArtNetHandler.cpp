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
        //Serial.println("Color Mode: SINGLE");
        uint8_t brightness = data[startAddress - 1];
        RgbColor color(data[startAddress], data[startAddress + 1], data[startAddress + 2]);
        bool needsUpdate = false;

        if (brightness != currentBrightness) { currentBrightness = brightness; needsUpdate = true; }
        if (color != currentColor) { currentColor = color; needsUpdate = true; }

        //Serial.printf("Received color R:%d G:%d B:%d at brightness %d\n", color.R, color.G, color.B, brightness);

        if (needsUpdate) {
            RgbColor scaledColor = color.Dim(currentBrightness);
            Serial.printf("Updating all LEDs to R:%d G:%d B:%d at brightness %d\n", scaledColor.R, scaledColor.G, scaledColor.B, currentBrightness);
            for (int i = 0; i < numLeds; i++) strip->SetPixelColor(i, scaledColor);
            strip->Show();
        }

    } else { // COLOR_MODE_MULTIPLE
        for (int i = 0; i < numLeds; i++) {
            int j = i * 4;
            uint8_t brightness = data[j + startAddress - 1];
            RgbColor color(data[j + startAddress], data[j + startAddress + 1], data[j + startAddress + 2]);
            RgbColor scaledColor = color.Dim(brightness);
            Serial.printf("LED %d - R:%d G:%d B:%d at brightness %d\n", i, scaledColor.R, scaledColor.G, scaledColor.B, brightness);
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

