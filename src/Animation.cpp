#include "Animation.h"

// LED sector configurations
const int sector1[] = {5, 6, 7, 13, 14};     // Top
const int sector2[] = {8, 9, 10, 16};        // Top-right
const int sector3[] = {17, 18, 19, 20};      // Right
const int sector4[] = {21, 22, 23, 32};      // Bottom-right
const int sector5[] = {29, 30, 31, 37, 38};  // Bottom
const int sector6[] = {28, 34, 35, 36};      // Bottom-left
const int sector7[] = {24, 25, 26, 27, 33};  // Left
const int sector8[] = {11, 12, 1, 2, 3, 4};  // Top-left (wrap-around)

// Array of all sectors for easy iteration
const int *sectors[]
    = {sector1, sector2, sector3, sector4, sector5, sector6, sector7, sector8};
// Number of LEDs in each sector
const int sectorSizes[] = {5, 4, 4, 4, 5, 4, 5, 6};

// Variables for non-blocking timing
unsigned long previousMillis = 0;  // Stores the last time the LEDs were updated
int currentSector            = 0;  // Tracks the current sector for the spinner

// Spinner effect settings
const int interval = 100;  // Time in milliseconds between sector updates

// Function definition
void displaySpinnerEffect(CRGB color)
{
  unsigned long currentMillis = millis();  // Get the current time

  // Check if it's time to update the spinner (non-blocking)
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;  // Update the last time LEDs were changed

    // Clear all LEDs before lighting up the current sector
    FastLED.clear();

    // Light up LEDs in the current sector with the given color
    for (int j = 0; j < sectorSizes[currentSector]; j++)
    {
      int ledIndex
          = sectors[currentSector][j] - 1;  // -1 because array index is zero-based
      if (ledIndex >= 0 && ledIndex < numLeds)
      {
          leds[ledIndex] = color;
      }
    }

    // Show the current sector on LEDs
    FastLED.show();

    // Move to the next sector, wrapping around if needed
    currentSector = (currentSector + 1) % 8;
  }
}

void displaySectorColors()
{
  // Clear all LEDs before lighting up the current sector
  FastLED.clear();

  // Define colors for each sector
  CRGB colors[] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow,
                   CRGB::Magenta, CRGB::Cyan, CRGB::Orange, CRGB::Purple};

  // Light up LEDs in each sector with a different color
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < sectorSizes[i]; j++)
    {
      int ledIndex = sectors[i][j] - 1;  // -1 because array index is zero-based
      if (ledIndex >= 0 && ledIndex < numLeds)
      {
        leds[ledIndex] = colors[i];
      }
    }
  }

  // Show the current sector on LEDs
  FastLED.show();
}
