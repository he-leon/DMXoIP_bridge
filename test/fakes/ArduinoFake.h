#ifndef FAKE_ARDUINO_H
#define FAKE_ARDUINO_H

#include <cstdint>
#include <cstring>  // For memcpy/memset in structs
#include <map>

// --- FAKE GLOBALS ---
inline unsigned long fakeMillis = 0;
inline unsigned long fakeMicros = 0;
inline std::map<int, int> fakePinStates;

// --- FAKE CONSTANTS ---
#define HIGH 1
#define LOW 0
#define OUTPUT 1
#define LED_BUILTIN 13

// --- FAKE FUNCTIONS (from existing code) ---
inline void pinMode(int pin, int mode) { /* no-op */ }
inline void digitalWrite(int pin, int val) { fakePinStates[pin] = val; }
inline unsigned long millis() { return fakeMillis; }

// --- NEW FAKE FUNCTIONS ---

// DMXoIPHandler uses micros() for FPS calculation
inline unsigned long micros() { return fakeMicros; }

// DMXoIPHandler uses Serial.printf. We provide a stub for compilation.
namespace
{
struct SerialClass
{
  void printf(const char* format, ...) {}
  void println(const char* s) {}
  void begin(int baud) {}
} Serial;
}  // namespace

// Utility to advance time in tests
inline void advance_time_ms(unsigned long ms)
{
  fakeMillis += ms;
  fakeMicros += ms * 1000UL;
}

// --- FAKE NETWORKING/LIBRARY TYPES ---

// DMXoIPHandler uses htons for E1.31
inline uint16_t htons(uint16_t hostshort)
{
  // Basic implementation of host to network short (for little endian systems)
  return (hostshort >> 8) | (hostshort << 8);
}

// Minimal struct required for DMXoIPHandler::handleE131Packet
typedef struct e131_packet_t
{
  uint16_t universe;              // Universe ID (Network Byte Order)
  uint16_t property_value_count;  // Length (Network Byte Order)
  uint8_t sequence_number;
  uint8_t property_values[513];  // Data (Slot 0 + 512 DMX slots)
} e131_packet_t;

#endif  // FAKE_ARDUINO_H
