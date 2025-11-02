#ifndef ARTNETHANDLER_H
#define ARTNETHANDLER_H

#include <NeoPixelBus.h>

void setupArtNet();
void readArtNet();
void setupE131();
void readE131();
void handleE131Packet();

#endif  // ARTNETHANDLER_H

