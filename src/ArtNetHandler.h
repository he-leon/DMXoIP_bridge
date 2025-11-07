#ifndef ARTNETHANDLER_H
#define ARTNETHANDLER_H
#include <ESPAsyncE131.h>

#define PACKET_TIMEOUT_MS 5000

void setupArtNet();
void readArtNet();
void setupE131();
void readE131();
void handleE131Packet(e131_packet_t* packet);
bool isReceiving();

// --- Frame rate counter ---
void updateFrameRate();
uint16_t getFrameRate();

extern unsigned long lastPacketTime;

#endif  // ARTNETHANDLER_H

