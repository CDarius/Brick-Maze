#pragma once

#include <Arduino.h>

/**
 * HardwareServo class provides an interface to control a servo motor using the ESP32's LEDC PWM functionality.
 */
class HardwareServo {
public:
    /**
     * Constructor for the HardwareServo class.
     * @param pin The GPIO pin number to which the servo is connected.
     * @param channel The LEDC channel to use for PWM (0-15). (Note: 0-7 for ESP32-S3)
     * @param minAngle The minimum angle of the servo (default is 0 degrees).
     * @param maxAngle The maximum angle of the servo (default is 180 degrees).
     * @param minPulseWidthUs The pulse width in microseconds corresponding to the minimum angle (default is 1000us).
     * @param maxPulseWidthUs The pulse width in microseconds corresponding to the maximum angle (default is 2000us).
     * @param invertOutputPulseWidth If true, inverts angle-to-pulse mapping (minAngle -> maxPulseWidthUs, maxAngle -> minPulseWidthUs).
     */
    HardwareServo(uint8_t pin, uint8_t channel, int16_t minAngle = 0, int16_t maxAngle = 180, uint16_t minPulseWidthUs = 1000, uint16_t maxPulseWidthUs = 2000, bool invertOutputPulseWidth = false)
        : pin(pin), channel(channel), minAngle(minAngle), maxAngle(maxAngle), minPulseWidthUs(minPulseWidthUs), maxPulseWidthUs(maxPulseWidthUs), invertOutputPulseWidth(invertOutputPulseWidth) {
        // Calculate min and max duty cycle based on pulse widths and PWM frequency
        minDuty = (minPulseWidthUs * MAX_DUTY_CYCLE) / PWM_PERIOD_US;
        maxDuty = (maxPulseWidthUs * MAX_DUTY_CYCLE) / PWM_PERIOD_US;
    }

    /**
     * Initializes the servo by configuring the LEDC channel and attaching the pin.
     * @param initialAngle The initial angle to set the servo to (default is 90 degrees).
     * @return true if initialization was successful, false otherwise.
     */
    bool begin(int16_t initialAngle = 90);

    /**
     * Sets the servo to a specific angle.
     * @param angle The desired angle to set the servo to.
     */
    void setAngle(float angle);

    /**
     * Changes the servo angle by a delta from the last commanded angle.
     * @param deltaAngle The angle delta to apply.
     */
    void changeAngle(float deltaAngle);

    /**
     * Gets the most recently commanded angle.
     * @return Last angle setpoint.
     */
    inline float getLastAngle() const {
        if (maxPulseWidthUs == minPulseWidthUs) {
            return (float)minAngle;
        }
        float normalized = (float)(lastPulseWidthUs - minPulseWidthUs) / (float)(maxPulseWidthUs - minPulseWidthUs);
        return (float)minAngle + normalized * (float)(maxAngle - minAngle);
    }

    /**
     * Gets the most recently commanded pulse width in microseconds.
     * @return Last pulse width setpoint in microseconds.
     */
    inline uint16_t getLastPulseWitdth() const {
        return lastPulseWidthUs;
    }

    /**
     * Sets the servo to a specific pulse width in microseconds.
     * @param pulseWidthUs The desired pulse width in microseconds to set the servo to
     */
    void setPulseWidth(uint16_t pulseWidthUs);

private:
    // Common PWM parameters for servos
    static const int PWM_FREQUENCY = 50;      // Standard frequency for servos (50 Hz)
    #if defined(CONFIG_IDF_TARGET_ESP32S3)
        static const int PWM_RESOLUTION = 14;     // 14-bit resolution for fine control
    #else
        static const int PWM_RESOLUTION = 16;     // 16-bit resolution for fine control
    #endif
    static const int MAX_DUTY_CYCLE = (1 << PWM_RESOLUTION) - 1; // Maximum duty cycle value based on resolution
    static const int PWM_PERIOD_US = 1000000 / PWM_FREQUENCY; // Period in microseconds (20ms for 50Hz)

    uint8_t pin;
    uint8_t channel;
    int16_t minAngle;
    int16_t maxAngle;
    uint16_t minPulseWidthUs;
    uint16_t maxPulseWidthUs;
    bool invertOutputPulseWidth;
    uint16_t minDuty;
    uint16_t maxDuty;
    uint16_t lastPulseWidthUs = 0;
};