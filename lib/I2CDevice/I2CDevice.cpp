#include "I2CDevice.hpp"

SemaphoreHandle_t I2CDevice::busMutex() {
    static SemaphoreHandle_t mutex = xSemaphoreCreateRecursiveMutex();
    return mutex;
}

bool I2CDevice::begin() {
    BusLock lock;
    wire.beginTransmission(addr);
    return wire.endTransmission() == 0;
}
