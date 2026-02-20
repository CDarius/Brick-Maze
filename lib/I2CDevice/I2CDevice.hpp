#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * I2CDevice is a base class for communicating with I2C devices. It provides methods to initialize communication,
 * write bytes to a device register, and read bytes from a device register. Derived classes can implement specific
 * functionality for different I2C devices by utilizing these basic read/write operations.
 */
class I2CDevice {
    protected:
        uint8_t addr;
        TwoWire& wire;

        /**
         * Write multiple bytes to the device starting from a specific register.
         * @param reg The starting register address to write to.
         * @param buffer Pointer to the data buffer to be written.
         * @param length The number of bytes to write from the buffer.
         * @return true if the write operation was successful, false otherwise.
         */
        bool writeBytes(uint8_t reg, uint8_t *buffer, uint8_t length);

        /**
         * Read multiple bytes from the device starting from a specific register.
         * @param reg The starting register address to read from.
         * @param buffer Pointer to the data buffer where the read bytes will be stored.
         * @param length The number of bytes to read into the buffer.
         * @return true if the read operation was successful, false otherwise.
         */
        bool readBytes(uint8_t reg, uint8_t *buffer, uint8_t length);

   public:
        /** 
         * Constructor for I2CDevice.
         * @param wire Reference to the TwoWire instance (I2C bus) to use for communication.
         * @param addr The I2C address of the device.
        */
        I2CDevice(TwoWire &wire, uint8_t addr) : wire(wire), addr(addr) 
        {
        }

        /**
         * Initialize communication with the I2C device by checking if it acknowledges its address.
         * @return true if the device acknowledges its address, false otherwise.
         */
        bool begin();
};