#include "SoftTimer.hpp"

void SoftTimer::load(uint32_t timeSpanMs) {
    deadlineMs = millis() + timeSpanMs;
}

bool SoftTimer::isElapsed() const {
    return millis() >= deadlineMs;
}
