#ifndef ARTNETHANDLER_H
#define ARTNETHANDLER_H

#include <NeoPixelBus.h>

void setupArtNet();
void readArtNet();
void setupE131();
void readE131();
void handleE131Packet(struct e131_packet_t* packet);

#endif  // ARTNETHANDLER_H
