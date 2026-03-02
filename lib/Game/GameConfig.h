#pragma once 

#include <Arduino.h>

struct GameConfig {
    uint16_t easyTimeLimitMs;   // Time limit for easy level in milliseconds
    uint16_t mediumTimeLimitMs; // Time limit for medium level in milliseconds
    uint16_t hardTimeLimitMs;   // Time limit for hard level in milliseconds
    uint16_t maxServoPulseRate; // Maximum servo pulse change rate in pulses/second
    uint16_t servoPulseRange;   // Range of servo pulse in \microseconds (e.g. 1000 for 1000-2000us)

    uint16_t ballDropXPulseUs;  // X servo pulse width when ball is dropped
    uint16_t ballDropYPulseUs;  // Y servo pulse width when ball is dropped

    float servoCalibrationErrorThresholdDeg;    // Acceptable error threshold in degrees for servo calibration
    float servoCalibrationTargetAngleXDeg;      // Compensation for physical X axis misalignment in degrees for servo calibration to achieve level orientation
    float servoCalibrationTargetAngleYDeg;      // Compensation for physical Y axis misalignment in degrees for servo calibration to achieve level orientation
};