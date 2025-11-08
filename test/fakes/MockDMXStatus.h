#pragma once
#include "interfaces/IDMXoIPStatus.h"

class MockDMXStatus : public IDMXoIPStatus {
public:
    bool receiving = false;
    int fps = 0;

    bool isReceiving() const override { return receiving; }
};
