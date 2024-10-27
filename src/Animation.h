// Animation.h
#ifndef ANIMATION_H
#define ANIMATION_H

#include <FastLED.h>

extern int numLeds;
extern CRGB *leds;

void displaySpinnerEffect(CRGB color);
void displaySectorColors();

#endif
