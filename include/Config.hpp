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
        .maxServoPulseRate = 2000,  // Maximum servo pulse change rate in pulses/second
        .servoPulseRange = 600,     // Range of servo pulse in microseconds (e.g. 1000 for 1000-2000us)

        .ballDropXDeltaPulseUs = 200,  // X servo pulse width delta from center when ball is dropped
        .ballDropYDeltaPulseUs = -250, // Y servo pulse width delta from center when ball is dropped
        .ballDropTimeMs = 4000,        // Time in milliseconds for the ball to reach the drop collection box after being dropped

        .servoCalibrationErrorThresholdDeg = 0.5f,  // Acceptable error threshold in degrees for servo calibration
        .servoCalibrationTargetAngleXDeg = 0.0f,    // Compensation for physical X axis misalignment in degrees for servo calibration to achieve level orientation
        .servoCalibrationTargetAngleYDeg = -1.7f    // Compensation for physical Y axis misalignment in degrees for servo calibration to achieve level orientation
    };

    return config;
}