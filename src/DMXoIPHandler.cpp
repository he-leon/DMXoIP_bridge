#include "DMXoIPHandler.h"

static uint16_t frameCount    = 0;
static uint32_t lastFpsMicros = 0;
static uint16_t currentFps    = 0;  // Integer FPS

// Need a static pointer to the current instance for the C-style callbacks
// This is a common pattern for Arduino libraries that don't support class methods
static DMXoIPHandler* currentInstance = nullptr;

// --- Constructor ---
DMXoIPHandler::DMXoIPHandler(IDMXFrameHandler& frameHandler,
                             ArtnetWifi& artnet,
                             ESPAsyncE131& e131,
                             DMX_ESPNOW& dmxEspNow)  // <-- MODIFIED
    : _frameHandler(frameHandler)
    , _artnet(artnet)
    , _e131(e131)
    , _dmxEspNow(dmxEspNow)  // <-- NEW
{
  // Set the static instance pointer when the object is created
  currentInstance = this;
}

bool DMXoIPHandler::isReceiving() const
{
  return (millis() - lastPacketTime) < PACKET_TIMEOUT_MS;
}

void DMXoIPHandler::processFrame(uint16_t universeIn,
                                 uint16_t length,
                                 uint8_t sequence,
                                 uint8_t* data)
{
  lastPacketTime = millis();

  // Delegate the call to the handler via the interface
  _frameHandler.handleFrame(universeIn, length, data);
}

// ----------------- Art-Net -----------------

// Static proxy function required by the ArtnetWifi library
static void artnetDmxCallback(uint16_t universe,
                              uint16_t length,
                              uint8_t sequence,
                              uint8_t* data)
{
  if (currentInstance)
  {
    currentInstance->processFrame(universe, length, sequence, data);
  }
}

void DMXoIPHandler::setupArtNet()
{
  _artnet.begin();
  _artnet.setArtDmxCallback(artnetDmxCallback);  // <-- Use the static proxy
  Serial.println("Art-Net initialized and ready.");
}

void DMXoIPHandler::readArtNet() { _artnet.read(); }

// ----------------- E1.31 -----------------
void DMXoIPHandler::handleE131Packet(e131_packet_t* packet)
{
  uint16_t universe = htons(packet->universe);
  uint16_t length   = htons(packet->property_value_count) - 1;
  uint8_t* data     = packet->property_values + 1;
  // E1.31 doesn't include sequence number in the core data, pass a dummy one if needed
  processFrame(universe, length, packet->sequence_number, data);
}

void DMXoIPHandler::setupE131()
{
  if (_e131.begin(E131_MULTICAST))
    Serial.println("E1.31 receiver initialized");
  else
    Serial.println("E1.31 failed to start!");
}

void DMXoIPHandler::readE131()
{
  if (!_e131.isEmpty())
  {
    e131_packet_t packet;
    while (!_e131.isEmpty())
    {
      _e131.pull(&packet);
      handleE131Packet(&packet);
    }
  }
}

// ----------------- DMX_ESPNOW -----------------

// Static proxy function for the DMX_ESPNOW library
void DMXoIPHandler::espNowDmxFrameCallback(uint8_t universeId)
{
  Serial.println("DMX_ESPNOW frame received callback triggered.");
  if (currentInstance)
  {
    // DMX_ESPNOW frame is complete. We can now retrieve the full DMX buffer.
    uint8_t* dmxData = currentInstance->_dmxEspNow.getDMXBuffer();

    // DMX_ESPNOW typically handles 512 slots.
    currentInstance->processFrame(universeId,
                                  DMX_UNIVERSE_SIZE,  // length (full 512 slots)
                                  0,                  // sequence (dummy)
                                  dmxData             // data pointer
    );
  }
}

void DMXoIPHandler::setupEspNowReceiver()
{
  if (_dmxEspNow.beginReceiver(espNowDmxFrameCallback))
  {
    Serial.println("DMX_ESPNOW receiver initialized and ready.");
  }
  else
  {
    Serial.println("ERROR: Failed to initialize DMX_ESPNOW receiver!");
  }
}

void DMXoIPHandler::readEspNow() { _dmxEspNow.update(); }
