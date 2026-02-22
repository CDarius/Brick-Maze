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
     */
    HardwareServo(uint8_t pin, uint8_t channel, int16_t minAngle = 0, int16_t maxAngle = 180, uint16_t minPulseWidthUs = 1000, uint16_t maxPulseWidthUs = 2000)
        : pin(pin), channel(channel), minAngle(minAngle), maxAngle(maxAngle), minPulseWidthUs(minPulseWidthUs), maxPulseWidthUs(maxPulseWidthUs) {
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
    void setAngle(int angle);

    /**
     * Sets the servo to a specific pulse width in microseconds.
     * @param pulseWidthUs The desired pulse width in microseconds to set the servo to
     */
    inline void setPulseWidth(uint16_t pulseWidthUs) {
        // Limit the pulse width to the specified range
        pulseWidthUs = constrain(pulseWidthUs, minPulseWidthUs, maxPulseWidthUs);

        // Map the pulse width to the corresponding duty cycle
        uint32_t duty = (((uint32_t)pulseWidthUs) * MAX_DUTY_CYCLE) / PWM_PERIOD_US;

        // Write the duty cycle value to the PWM channel
        ledcWrite(channel, duty);
    }

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
    uint16_t minDuty;
    uint16_t maxDuty;
};