#pragma once

#include <ControllerConfig.h>
#include <GameConfig.h>

ControllerConfig getDefaultControllerConfig() {
    ControllerConfig config {
        .maxAngle = 1.0f, // Max angle in range [0, 1]
        .updateRateMs = 100 // Update rate in milliseconds
    };

    return config;
}

GameConfig getDefaultGameConfig() {
    GameConfig config {
        .easyTimeLimitMs = 45000,   // Time limit for easy level in milliseconds
        .mediumTimeLimitMs = 30000, // Time limit for medium level in milliseconds
        .hardTimeLimitMs = 15000,   // Time limit for hard level in milliseconds
        .maxServoPulseRate = 1000,  // Maximum servo pulse change rate in pulses/second
        .servoPulseRange = 166,     // Range of servo pulse in \microseconds (e.g. 1000 for 1000-2000us)

        .ballDropXPulseUs = 1500,  // X servo pulse width when ball is dropped
        .ballDropYPulseUs = 1500   // Y servo pulse width when ball is dropped
    };

    return config;
}