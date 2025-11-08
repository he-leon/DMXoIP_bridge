#ifndef MOCK_DMX_OUTPUT_H
#define MOCK_DMX_OUTPUT_H

#include "interfaces/IDMXOutput.h"
#include <cstring>
#include <cstdint>

class MockDMXOutput : public IDMXOutput {
public:
    int callCount = 0;
    uint16_t lastLength = 0;
    uint8_t lastData[512] = {0};

    void writeFrame(uint16_t length, uint8_t* data) override {
        callCount++;
        lastLength = length;

        size_t bytesToCopy = (length > 512) ? 512 : length;
        if (data != nullptr) {
            memcpy(lastData, data, bytesToCopy);
        } else {
            memset(lastData, 0, bytesToCopy);
        }
    }

    bool wasCalled(int expectedCount) const { return callCount == expectedCount; }
};

#endif // MOCK_DMX_OUTPUT_H
