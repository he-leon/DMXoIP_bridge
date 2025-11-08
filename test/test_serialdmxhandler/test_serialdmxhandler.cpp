#include <unity.h>
#include <cstring>

#include "fakes/MockDMXOutput.h"
#include "SerialDMXFrameHandler.h"

// Test fixtures
MockDMXOutput mockOutput;
SerialDMXFrameHandler* handler = nullptr;
const uint16_t TARGET_UNIVERSE = 5;

void setUp(void) {
    mockOutput.callCount = 0;
    mockOutput.lastLength = 0;
    
    // Instantiate the handler with the mock output driver
    handler = new SerialDMXFrameHandler(mockOutput, TARGET_UNIVERSE);
}

void tearDown(void) {
    delete handler;
    handler = nullptr;
}

// --- Test Cases ---

void test_SerialDMXFrameHandler_delegates_correct_universe() {
    uint8_t testData[] = {10, 20, 30};
    uint16_t length = 3;

    handler->handleFrame(TARGET_UNIVERSE, length, testData);

    TEST_ASSERT_TRUE(mockOutput.wasCalled(1));
    TEST_ASSERT_EQUAL_UINT16(length, mockOutput.lastLength);
    TEST_ASSERT_EQUAL_UINT8(testData[0], mockOutput.lastData[0]);
    TEST_ASSERT_EQUAL_UINT8(testData[2], mockOutput.lastData[2]);
}

void test_SerialDMXFrameHandler_filters_incorrect_universe() {
    uint8_t testData[] = {1, 1, 1};
    uint16_t length = 3;

    handler->handleFrame(6, length, testData);

    TEST_ASSERT_TRUE(mockOutput.wasCalled(0));
}

void test_SerialDMXFrameHandler_filters_zero_length_frame() {
    uint8_t testData[] = {};
    
    handler->handleFrame(TARGET_UNIVERSE, 0, testData);

    TEST_ASSERT_TRUE(mockOutput.wasCalled(0));
}

void test_SerialDMXFrameHandler_filters_over_512_length_frame() {
    uint8_t testData[513] = {0};
    
    handler->handleFrame(TARGET_UNIVERSE, 513, testData);

    TEST_ASSERT_TRUE(mockOutput.wasCalled(0));
}


void setup_test_runner() {
    UNITY_BEGIN();
    
    RUN_TEST(test_SerialDMXFrameHandler_delegates_correct_universe);
    RUN_TEST(test_SerialDMXFrameHandler_filters_incorrect_universe);
    RUN_TEST(test_SerialDMXFrameHandler_filters_zero_length_frame);
    RUN_TEST(test_SerialDMXFrameHandler_filters_over_512_length_frame);
    
    UNITY_END();
}

// Standard main function for native testing
int main() {
    setup_test_runner();
    return 0;
}
