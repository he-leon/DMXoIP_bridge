#ifndef STATUS_LED_H
#define STATUS_LED_H
#endif  

#ifdef UNIT_TEST
#include "fakes/ArduinoFake.h"
#else
#include <Arduino.h>
#endif
#include "interfaces/IDMXoIPStatus.h"

class StatusLED {
public:
  StatusLED(const IDMXoIPStatus& status, uint8_t pin = LED_BUILTIN,
            unsigned long blinkInterval = 500, bool activeLow = false);

  void begin();
  void update();

private:
  const IDMXoIPStatus& _status;
  uint8_t _pin;
  unsigned long _interval;
  bool _activeLow;
  unsigned long _lastBlink;
  bool _ledOn;
};
