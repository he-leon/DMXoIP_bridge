#include "DMXoIPHandler.h"

static uint16_t frameCount = 0;
static uint32_t lastFpsMicros = 0;
static uint16_t currentFps = 0;  // Integer FPS

// Need a static pointer to the current instance for the C-style callbacks
// This is a common pattern for Arduino libraries that don't support class methods
static DMXoIPHandler* currentInstance = nullptr; 

// --- Constructor ---
DMXoIPHandler::DMXoIPHandler(IDMXFrameHandler& frameHandler,
                             ArtnetWifi& artnet,
                             ESPAsyncE131& e131)
    : _frameHandler(frameHandler), 
      _artnet(artnet), // Initialize references
      _e131(e131)     // Initialize references
{
    // Set the static instance pointer when the object is created
    currentInstance = this;
}

// ----------------- Frame rate counter -----------------
void DMXoIPHandler::updateFrameRate() {
    frameCount++;
    uint32_t now = micros();
    uint32_t elapsed = now - lastFpsMicros;
    
    if (elapsed >= 100000) {
        currentFps = (frameCount * 1000000UL) / elapsed;  // Pure integer math
        frameCount = 0;
        lastFpsMicros = now;
    }
}

// ----------------- IDMXoIPStatus Implementation -----------------
int DMXoIPHandler::getFrameRate() const {
    return currentFps;
}

bool DMXoIPHandler::isReceiving() const {
    return (millis() - lastPacketTime) < PACKET_TIMEOUT_MS;
}

// ----------------- DMX Frame Processing Proxy -----------------
// This method is called by the Art-Net and E1.31 receivers
void DMXoIPHandler::processFrame(uint16_t universeIn, uint16_t length, uint8_t sequence, uint8_t* data) {
    updateFrameRate();
    lastPacketTime = millis();
    
    Serial.printf("CH1:%d, FPS:%d\n", data[0], getFrameRate());
    
    // Delegate the call to the handler via the interface
    _frameHandler.handleFrame(universeIn, length, data);
}

// ----------------- Art-Net -----------------

// Static proxy function required by the ArtnetWifi library
static void artnetDmxCallback(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
    if (currentInstance) {
        currentInstance->processFrame(universe, length, sequence, data);
    }
}

void DMXoIPHandler::setupArtNet() {
    _artnet.begin();
    _artnet.setArtDmxCallback(artnetDmxCallback); // <-- Use the static proxy
    Serial.println("Art-Net initialized and ready.");
}

void DMXoIPHandler::readArtNet() { _artnet.read(); }

// ----------------- E1.31 -----------------
void DMXoIPHandler::handleE131Packet(e131_packet_t* packet) {
    uint16_t universe = htons(packet->universe);
    uint16_t length = htons(packet->property_value_count) - 1;
    uint8_t* data = packet->property_values + 1;
    // E1.31 doesn't include sequence number in the core data, pass a dummy one if needed
    processFrame(universe, length, packet->sequence_number, data); 
}

void DMXoIPHandler::setupE131() {
    if (_e131.begin(E131_MULTICAST)) Serial.println("E1.31 receiver initialized");
    else Serial.println("E1.31 failed to start!");
}

void DMXoIPHandler::readE131() {
    if (!_e131.isEmpty()) {
        e131_packet_t packet;
        while (!_e131.isEmpty()) { 
            _e131.pull(&packet); 
            handleE131Packet(&packet); 
        }
    }
}
