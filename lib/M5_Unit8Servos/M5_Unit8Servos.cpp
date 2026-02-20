#include "M5_Unit8Servos.hpp"

bool M5_Unit8Servos::setAllPinMode(extio_io_mode_t mode) {
    uint8_t data[8];
    memset(data, mode, 8);

    return writeBytes(M5_UNIT_8SERVO_MODE_REG, data, 8);
}

bool M5_Unit8Servos::setOnePinMode(uint8_t pin, extio_io_mode_t mode) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    uint8_t data = mode;
    return writeBytes(pin, &data, 1);
}

bool M5_Unit8Servos::setDeviceAddr(uint8_t addr) {
    if (writeBytes(M5_UNIT_8SERVO_ADDRESS_REG, &addr, 1)) {
        this->addr = addr;
        return true;
    } else {
        return false;
    }
}

uint8_t M5_Unit8Servos::getVersion() {
    uint8_t data = 0;
    readBytes(M5_UNIT_8SERVO_FW_VERSION_REG, &data, 1);
    return data;
}

bool M5_Unit8Servos::setDigitalOutput(uint8_t pin, uint8_t state) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    uint8_t reg = pin + M5_UNIT_8SERVO_OUTPUT_CTL_REG;
    return writeBytes(reg, &state, 1);
}

bool M5_Unit8Servos::setLEDColor(uint8_t pin, uint32_t color) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    uint8_t data[3];
    data[0] = (color >> 16) & 0xff;
    data[1] = (color >> 8) & 0xff;
    data[2] = color & 0xff;
    uint8_t reg = pin * 3 + M5_UNIT_8SERVO_RGB_24B_REG;
    return writeBytes(reg, data, 3);
}

bool M5_Unit8Servos::setServoAngle(uint8_t pin, uint8_t angle) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    if (angle > 180) 
        angle = 180; // Limit angle to 180 degrees

    uint8_t reg = pin + M5_UNIT_8SERVO_SERVO_ANGLE_8B_REG;
    return writeBytes(reg, &angle, 1);
}

bool M5_Unit8Servos::setPWM(uint8_t pin, uint8_t value) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    uint8_t reg = pin + M5_UNIT_8SERVO_PWM_8B_REG;
    return writeBytes(reg, &value, 1);
}

bool M5_Unit8Servos::setServoPulse(uint8_t pin, uint16_t pulse) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return false;

    uint8_t reg = pin * 2 + M5_UNIT_8SERVO_SERVO_PULSE_16B_REG;
    uint8_t data[2];
    data[1] = (pulse >> 8) & 0xff;
    data[0] = pulse & 0xff;
    return writeBytes(reg, data, 2);
}

uint8_t M5_Unit8Servos::getDigitalInput(uint8_t pin) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return LOW;

    uint8_t data;
    uint8_t reg = pin + M5_UNIT_8SERVO_DIGITAL_INPUT_REG;
    if (readBytes(reg, &data, 1)) {
        return data;
    }

    return LOW;
}

uint16_t M5_Unit8Servos::getAnalogInput(uint8_t pin, extio_anolog_read_mode_t bit) {
    if (pin < 0 || pin >= M5_UNIT_8SERVO_NUM_PINS) 
        return 0;

    if (bit == _8bit) {
        uint8_t data;
        uint8_t reg = pin + M5_UNIT_8SERVO_ANALOG_INPUT_8B_REG;
        if (readBytes(reg, &data, 1)) {
            return data;
        }
    } else {
        uint8_t data[2];
        uint8_t reg = pin * 2 + M5_UNIT_8SERVO_ANALOG_INPUT_12B_REG;
        if (readBytes(reg, data, 2)) {
            return (data[1] << 8) | data[0];
        }
    }
    return 0;
}

float M5_Unit8Servos::getServoCurrent(void) {
    uint8_t data[4];
    float c;
    uint8_t *p;

    readBytes(M5_UNIT_8SERVO_CURRENT_REG, data, 4);
    p = (uint8_t *)&c;
    memcpy(p, data, 4);

    return c;
}
