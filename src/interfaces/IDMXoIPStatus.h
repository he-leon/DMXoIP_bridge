#pragma once

class IDMXoIPStatus
{
public:
    virtual bool isReceiving() const = 0;
    virtual int getFrameRate() const = 0;
    virtual ~IDMXoIPStatus() = default;
};

