#include <HardwareServo.hpp>

bool HardwareServo::begin(int16_t initialAngle) {
    // Configure the LEDC channel
    if (ledcSetup(channel, PWM_FREQUENCY, PWM_RESOLUTION) == 0) {
        // ledcSetup returns 0 in case of error
        return false;
    }

    // Attach the pin to the configured channel
    ledcAttachPin(pin, channel);

    setAngle(initialAngle);
    return true;
}

void HardwareServo::setAngle(int angle) {
    // Limit the angle to the specified range
    angle = constrain(angle, minAngle, maxAngle);

    // Map the angle to the corresponding duty cycle
    long duty = map(angle, minAngle, maxAngle, minDuty, maxDuty);

    // Write the duty cycle value to the PWM channel
    ledcWrite(channel, (int32_t)duty);
}

