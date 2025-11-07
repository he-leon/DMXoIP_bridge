#include <ArtnetWifi.h>
#include "ConfigParameters.h"
#include "ArtNetHandler.h"
#include "LEDConfig.h"


ArtnetWifi artnet;
ESPAsyncE131 e131;

RgbColor currentColor(0);
uint8_t currentBrightness = DEFAULT_BRIGHTNESS;
unsigned long lastPacketTime = 0;

void onDmxFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data) {
    if (universeIn != ::universe) return;
    lastPacketTime = millis();
    Serial.printf("SEQ:%d\n", data[0]);
    if (startAddress - 1 < 0 || (startAddress - 1 + 2) >= length) return;

    static unsigned long lastPowerCalc = 0;

    if (colorMode == COLOR_MODE_SINGLE) {
        //Serial.println("Color Mode: SINGLE");
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

        //Serial.printf("Received color R:%d G:%d B:%d at brightness %d\n", color.R, color.G, color.B, brightness);

        if (needsUpdate) {
            RgbColor scaledColor = color.Dim(currentBrightness);
            //Serial.printf("Updating all LEDs to R:%d G:%d B:%d at brightness %d\n", scaledColor.R, scaledColor.G, scaledColor.B, currentBrightness);
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
            //Serial.printf("LED %d - R:%d G:%d B:%d at brightness %d\n", i, scaledColor.R, scaledColor.G, scaledColor.B, brightness);
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

bool isReceiving() {
    return (millis() - lastPacketTime) < PACKET_TIMEOUT_MS;
}

