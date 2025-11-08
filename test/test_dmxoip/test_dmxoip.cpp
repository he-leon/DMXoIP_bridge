#include <unity.h>
#include <cstring> // For memset/memcpy

#define UNIT_TEST // Required to trigger conditional compilation in DMXoIPHandler.cpp

#include "fakes/ArduinoFake.h"
#include "fakes/MockDMXFrameHandler.h"
#include "fakes/MockDMXoIPLibs.h" // Contains ArtnetWifi and ESPAsyncE131 MOCK CLASS DEFINITIONS
#include "DMXoIPHandler.h"

ArtnetWifi artnet;  
ESPAsyncE131 e131;  

MockDMXFrameHandler mockFrameHandler;
DMXoIPHandler* handler = nullptr;

// A constant needed for E1.31 setup (assuming it's not mocked)
// You might need to mock this definition if it exists in another header.
#define E131_MULTICAST 0x01

void setUp(void) {
    // Reset Fakes
    fakeMillis = 0;
    fakeMicros = 0;
    
    // Reset Mocks (The global instances used in tests)
    mockFrameHandler.callCount = 0;
    e131.beginCalled = false;
    e131.isEmptyValue = true;
    artnet.beginCalled = false;
    artnet.storedCallback = nullptr;

    handler = new DMXoIPHandler(mockFrameHandler, artnet, e131);
}

void tearDown(void) {
    delete handler;
    handler = nullptr;
}

// --- Status/Lifecycle Tests ---

void test_DMXoIPHandler_setupArtNet_calls_begin_and_sets_callback() {
    handler->setupArtNet();
    
    // Verify ArtnetWifi::begin was called (checks the global mock instance)
    TEST_ASSERT_TRUE(artnet.beginCalled);
    
    // Verify callback was set 
    TEST_ASSERT_NOT_NULL(artnet.storedCallback);
}

void test_DMXoIPHandler_setupE131_calls_begin() {
    handler->setupE131();
    
    // Verify E131::begin was called
    TEST_ASSERT_TRUE(e131.beginCalled);
}

void test_DMXoIPHandler_isReceiving_status() {
    // Initially, no packets received and timeout reached
    advance_time_ms(PACKET_TIMEOUT_MS * 2); 

    TEST_ASSERT_FALSE(handler->isReceiving()); 
    
    // Simulate a frame arrival (updates lastPacketTime = 0)
    uint8_t data[] = {1};
    handler->processFrame(0, 1, 1, data);
    
    // Advance time just under timeout (PACKET_TIMEOUT_MS = 5000)
    advance_time_ms(PACKET_TIMEOUT_MS - 1); // fakeMillis = 4999
    TEST_ASSERT_TRUE(handler->isReceiving());
    
    // Advance time past timeout
    advance_time_ms(2); // fakeMillis = 5001
    TEST_ASSERT_FALSE(handler->isReceiving());
}


// --- Frame Delegation Tests ---

void test_DMXoIPHandler_delegates_ArtNet_frame() {
    handler->setupArtNet(); // Ensures callback is set

    uint16_t universe = 42;
    uint16_t length = 10;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // Act: Trigger the static Art-Net callback via the global mock helper
    artnet.triggerDmxCallback(universe, length, 1, data);

    // Assert: Check if the mock handler was called and received the correct data
    TEST_ASSERT_TRUE(mockFrameHandler.wasCalled(1));
    TEST_ASSERT_EQUAL_UINT16(universe, mockFrameHandler.lastUniverse);
    TEST_ASSERT_EQUAL_UINT16(length, mockFrameHandler.lastLength);
    TEST_ASSERT_EQUAL_UINT8(data[0], mockFrameHandler.lastData[0]);
    TEST_ASSERT_EQUAL_UINT8(data[3], mockFrameHandler.lastData[3]);
}

void test_DMXoIPHandler_delegates_E131_frame() {
    // Arrange: Create a fake E1.31 packet
    e131_packet_t testPacket;
    memset(&testPacket, 0, sizeof(e131_packet_t));
    
    // Simulate Universe 1 (network byte order)
    testPacket.universe = htons(1); 
    // Simulate 4 DMX slots (start code + R, G, B)
    testPacket.property_value_count = htons(4); 
    testPacket.sequence_number = 5;
    
    uint8_t r_data = 100;
    uint8_t g_data = 200;
    uint8_t b_data = 50;
    
    // DMX data starts at index 1 of property_values (index 0 is start code)
    testPacket.property_values[1] = r_data;
    testPacket.property_values[2] = g_data;
    testPacket.property_values[3] = b_data;
    
    // Set the E131 mock to return this packet
    e131.nextPacket = testPacket;
    e131.isEmptyValue = false; // Make it non-empty
    
    // Act: Call readE131, which should pull the packet and call processFrame
    handler->readE131();

    // Assert: Check if the mock handler was called with the decoded data
    TEST_ASSERT_TRUE(mockFrameHandler.wasCalled(1));
    TEST_ASSERT_EQUAL_UINT16(1, mockFrameHandler.lastUniverse);
    TEST_ASSERT_EQUAL_UINT16(3, mockFrameHandler.lastLength);
    TEST_ASSERT_EQUAL_UINT8(r_data, mockFrameHandler.lastData[0]);
    TEST_ASSERT_EQUAL_UINT8(g_data, mockFrameHandler.lastData[1]);
    TEST_ASSERT_EQUAL_UINT8(b_data, mockFrameHandler.lastData[2]);
}


void setup_test_runner() {
    UNITY_BEGIN();
    
    RUN_TEST(test_DMXoIPHandler_setupArtNet_calls_begin_and_sets_callback);
    RUN_TEST(test_DMXoIPHandler_setupE131_calls_begin);
    RUN_TEST(test_DMXoIPHandler_isReceiving_status);
    RUN_TEST(test_DMXoIPHandler_delegates_ArtNet_frame);
    RUN_TEST(test_DMXoIPHandler_delegates_E131_frame);
    
    UNITY_END();
}

// Standard Arduino setup/loop for integration with test runners like PlatformIO
#ifdef ARDUINO
void setup() {
    // Small delay for serial port to initialize
    delay(2000); 
    setup_test_runner();
}

void loop() {
    // If testing complete, loop can be empty
}
#else
int main() {
    setup_test_runner();
    return 0;
}
#endif
