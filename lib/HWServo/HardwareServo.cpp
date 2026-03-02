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

void HardwareServo::setAngle(float angle) {
    // Limit the angle to the specified range
    angle = constrain(angle, minAngle, maxAngle);

    float normalized = (maxAngle != minAngle)
                           ? (angle - (float)minAngle) / (float)(maxAngle - minAngle)
                           : 0.0f;
    normalized = constrain(normalized, 0.0f, 1.0f);

    uint16_t pulseWidthUs = (uint16_t)((float)minPulseWidthUs + normalized * (float)(maxPulseWidthUs - minPulseWidthUs));
    setPulseWidth(pulseWidthUs);
}

void HardwareServo::changeAngle(float deltaAngle) {
    setAngle(getLastAngle() + deltaAngle);
}

void HardwareServo::setPulseWidth(uint16_t pulseWidthUs) {
    // Limit the pulse width to the specified range
    pulseWidthUs = constrain(pulseWidthUs, minPulseWidthUs, maxPulseWidthUs);

    // Track the latest pulse width setpoint (before optional inversion)
    lastPulseWidthUs = pulseWidthUs;

    // Optionally invert pulse width mapping (min -> max, max -> min)
    if (invertOutputPulseWidth) {
        uint32_t pulseWidthSum = (uint32_t)minPulseWidthUs + (uint32_t)maxPulseWidthUs;
        pulseWidthUs = (uint16_t)(pulseWidthSum - pulseWidthUs);
    }

    // Map the pulse width to the corresponding duty cycle
    uint32_t duty = (((uint32_t)pulseWidthUs) * MAX_DUTY_CYCLE) / PWM_PERIOD_US;

    // Write the duty cycle value to the PWM channel
    ledcWrite(channel, duty);
}