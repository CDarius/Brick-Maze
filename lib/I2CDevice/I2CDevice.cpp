#include "I2CDevice.hpp"

bool I2CDevice::begin() {
    wire.beginTransmission(addr);
    uint8_t error = wire.endTransmission();
    if (error == 0) {
        return true;
    } else {
        return false;
    }
}

bool I2CDevice::writeBytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
    wire.beginTransmission(addr);
    wire.write(reg);
    for (uint8_t i = 0; i < length; i++) {
        wire.write(*(buffer + i));
    }
    if (wire.endTransmission() == 0) return true;
    return false;
}

bool I2CDevice::readBytes(uint8_t reg, uint8_t *buffer, uint8_t length) {
    uint8_t index = 0;
    wire.beginTransmission(addr);
    wire.write(reg);
    wire.endTransmission(false);
    if (wire.requestFrom(addr, length)) {
        for (uint8_t i = 0; i < length; i++) {
            buffer[index++] = wire.read();
        }
        return true;
    }
    return false;
}
