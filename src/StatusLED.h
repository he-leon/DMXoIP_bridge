#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

class StatusLED {
public:
  StatusLED(uint8_t pin = LED_BUILTIN, unsigned long blinkInterval = 500, bool activeLow = false);

  void begin();
  void update(bool receiving);

private:
  uint8_t _pin;
  unsigned long _interval;
  bool _activeLow;
  unsigned long _lastBlink;
  bool _ledOn;
};

#endif

