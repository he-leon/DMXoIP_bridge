#include "StatusLED.h"

StatusLED::StatusLED(uint8_t pin, unsigned long blinkInterval, bool activeLow)
  : _pin(pin), _interval(blinkInterval), _activeLow(activeLow),
    _lastBlink(0), _ledOn(false) {}

void StatusLED::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, _activeLow ? HIGH : LOW); // off by default
}

void StatusLED::update(bool receiving) {
  if (receiving) {
    // Solid ON while receiving
    digitalWrite(_pin, _activeLow ? LOW : HIGH);
    return;
  }

  // Blink when idle
  unsigned long now = millis();
  if (now - _lastBlink >= _interval) {
    _lastBlink = now;
    _ledOn = !_ledOn;
    digitalWrite(_pin, _activeLow
      ? (_ledOn ? LOW : HIGH)
      : (_ledOn ? HIGH : LOW));
  }
}

