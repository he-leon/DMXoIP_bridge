#include "HardwareSerialDMXOutput.h"

#include <SPIFFS.h>
#include <string.h>  // For memcpy

static const char* DMX_STATE_FILE = "/dmx_state.bin";

HardwareSerialDMXOutput::HardwareSerialDMXOutput(HardwareSerial& serialPort)
    : _serialPort(serialPort)
{
  _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N2);
  memset(_dmxBuffer, 0, sizeof(_dmxBuffer));
  memset(_lastPersistedBuffer, 0, sizeof(_lastPersistedBuffer));
  loadDMXState();
}

void HardwareSerialDMXOutput::writeFrame(uint16_t length, uint8_t* data)
{
  if (length > MAX_DMX_SLOTS)
  {
    length = MAX_DMX_SLOTS;
  }
  memcpy(_dmxBuffer, data, length);
  _dmxLength = length;
}

void HardwareSerialDMXOutput::sendDMX()
{
  if (_dmxLength == 0)
  {
    return;  // Nothing to send
  }

  // The following will take ~23ms and will block execution during that time.
  // 1. Generate the DMX BREAK condition
  // Flush any pending outgoing data
  _serialPort.flush();

  // Change to a lower baud rate (or simply 90000) to force a BREAK condition
  _serialPort.begin(90000, SERIAL_8N2);
  // Send a dummy byte (0) at the low rate to ensure the break is long enough
  _serialPort.write(0);
  // Wait for the BREAK to finish
  _serialPort.flush();

  // Return to the DMX baud rate for data transmission
  _serialPort.begin(DMX_BAUD_RATE, SERIAL_8N2);

  // Send the Start Code (0x00)
  _serialPort.write(0x00);

  // Send the DMX data slots
  for (int i = 0; i < _dmxLength; i++)
  {
    _serialPort.write(_dmxBuffer[i]);
  }

  // Wait for all data to be transmitted
  _serialPort.flush();
}

void HardwareSerialDMXOutput::updatePersistence()
{
  if (hasStateChanged())
  {
    if (!_stateChanged)
    {
      // State just changed, start the timer
      _stateChanged     = true;
      _lastChangeMillis = millis();
    }
    else if (millis() - _lastChangeMillis >= PERSIST_DELAY_MS)
    {
      // State has been stable for 10 seconds, persist it
      saveDMXState();
      _stateChanged = false;
    }
  }
  else
  {
    // State matches persisted state, reset flag
    _stateChanged = false;
  }
}

bool HardwareSerialDMXOutput::hasStateChanged() const
{
  if (_dmxLength != _lastPersistedLength)
  {
    return true;
  }
  return memcmp(_dmxBuffer, _lastPersistedBuffer, _dmxLength) != 0;
}

void HardwareSerialDMXOutput::loadDMXState()
{
  File stateFile = SPIFFS.open(DMX_STATE_FILE, FILE_READ);
  if (!stateFile)
  {
    Serial.println("No DMX state file found, starting with empty state");
    return;
  }

  // Read length (2 bytes)
  if (stateFile.available() >= 2)
  {
    uint8_t lengthBytes[2];
    stateFile.read(lengthBytes, 2);
    _dmxLength = (lengthBytes[0] << 8) | lengthBytes[1];

    if (_dmxLength > MAX_DMX_SLOTS)
    {
      Serial.printf("Invalid DMX length in state file: %u, resetting\n", _dmxLength);
      _dmxLength = 0;
      stateFile.close();
      return;
    }

    // Read DMX data
    if (stateFile.available() >= _dmxLength)
    {
      stateFile.read(_dmxBuffer, _dmxLength);
      memcpy(_lastPersistedBuffer, _dmxBuffer, _dmxLength);
      _lastPersistedLength = _dmxLength;
      Serial.printf("Loaded DMX state from file: %u channels\n", _dmxLength);
    }
    else
    {
      Serial.println("DMX state file corrupted, resetting");
      _dmxLength = 0;
    }
  }

  stateFile.close();
}

void HardwareSerialDMXOutput::saveDMXState()
{
  File stateFile = SPIFFS.open(DMX_STATE_FILE, FILE_WRITE);
  if (!stateFile)
  {
    Serial.println("Failed to open DMX state file for writing");
    return;
  }

  // Write length (2 bytes)
  uint8_t lengthBytes[2];
  lengthBytes[0] = (_dmxLength >> 8) & 0xFF;
  lengthBytes[1] = _dmxLength & 0xFF;
  stateFile.write(lengthBytes, 2);

  // Write DMX data
  stateFile.write(_dmxBuffer, _dmxLength);
  stateFile.close();

  // Update last persisted state
  memcpy(_lastPersistedBuffer, _dmxBuffer, _dmxLength);
  _lastPersistedLength = _dmxLength;

  Serial.printf("Saved DMX state to file: %u channels\n", _dmxLength);
}
