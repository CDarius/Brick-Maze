#pragma once

#include <Arduino.h>

class SoftTimer {
private:
    uint32_t deadlineMs = 0;

public:
    void load(uint32_t timeSpanMs);
    bool isElapsed() const;
};
