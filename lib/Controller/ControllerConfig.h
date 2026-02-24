#pragma once

#include <Arduino.h>

struct ControllerConfig {
    float maxAngle;         // Maximum angle in range [0, 1]
    uint16_t updateRateMs;  // Update rate in milliseconds
};
