#pragma once

#include <Arduino.h>
#include <I2CDevice.hpp>

#define M5_UNIT_8SERVO_DEFAULT_ADDR         0x25
#define M5_UNIT_8SERVO_NUM_PINS             8

#define M5_UNIT_8SERVO_MODE_REG             0x00
#define M5_UNIT_8SERVO_OUTPUT_CTL_REG       0x10
#define M5_UNIT_8SERVO_DIGITAL_INPUT_REG    0x20
#define M5_UNIT_8SERVO_ANALOG_INPUT_8B_REG  0x30
#define M5_UNIT_8SERVO_ANALOG_INPUT_12B_REG 0x40
#define M5_UNIT_8SERVO_SERVO_ANGLE_8B_REG   0x50
#define M5_UNIT_8SERVO_SERVO_PULSE_16B_REG  0x60
#define M5_UNIT_8SERVO_RGB_24B_REG          0x70
#define M5_UNIT_8SERVO_PWM_8B_REG           0x90
#define M5_UNIT_8SERVO_CURRENT_REG          0xA0

#define M5_UNIT_8SERVO_FW_VERSION_REG       0xFE
#define M5_UNIT_8SERVO_ADDRESS_REG          0xFF

typedef enum {
    DIGITAL_INPUT_MODE = 0,
    DIGITAL_OUTPUT_MODE,
    ADC_INPUT_MODE,
    SERVO_CTL_MODE,
    RGB_LED_MODE,
    PWM_MODE
} extio_io_mode_t;

typedef enum { _8bit = 0, _12bit } extio_anolog_read_mode_t;

class M5_Unit8Servos : public I2CDevice {
public:
    M5_Unit8Servos(TwoWire &wire, uint8_t addr = M5_UNIT_8SERVO_DEFAULT_ADDR) : I2CDevice(wire, addr) {}

    /**
     * Set the I2C address of the M5 Unit 8 Servos. This allows you to change the default 
     * address (0x25) to avoid conflicts with other devices on the same I2C bus.
     * @param addr The new I2C address to set for the device.
     * @return true if the address was successfully set, false otherwise.
     */
    bool setDeviceAddr(uint8_t addr);

    /**
     * Get the firmware version of the M5 Unit 8 Servos. This can be useful for debugging or ensuring 
     * compatibility with certain features.
     * @return The firmware version as an 8-bit unsigned integer.
     */
    uint8_t getVersion();

    /**
     * Set the mode for all pins on the M5 Unit 8 Servos. The mode can be:
     * - DIGITAL_INPUT_MODE
     * - DIGITAL_OUTPUT_MODE
     * - ADC_INPUT_MODE
     * - SERVO_CTL_MODE
     * - RGB_LED_MODE
     * - PWM_MODE
     * @param mode The mode to set for all pins.
     * @return true if the operation was successful, false otherwise.
     */
    bool setAllPinMode(extio_io_mode_t mode);

    /**
     * Set the mode for a specific pin on the M5 Unit 8 Servos. The mode can be:
     * - DIGITAL_INPUT_MODE
     * - DIGITAL_OUTPUT_MODE
     * - ADC_INPUT_MODE
     * - SERVO_CTL_MODE
     * - RGB_LED_MODE
     * - PWM_MODE
     * @param pin The pin number (0-7) to set the mode for.
     * @param mode The mode to set for the specified pin.
     * @return true if the operation was successful, false otherwise.
     */
    bool setOnePinMode(uint8_t pin, extio_io_mode_t mode);

    /**
     * Set the angle of a servo connected to a specific pin. The angle should be between 0 and 180 degrees. 
     * If the angle exceeds 180, it will be limited to 180.
     * @param pin The pin number (0-7) where the servo is connected.
     * @param angle The desired angle for the servo (0-180 degrees).
     * @return true if the operation was successful, false otherwise.
     */
    bool setServoAngle(uint8_t pin, uint8_t angle);

    /**
     * Set the pulse width (16-bit) for a servo connected to a specific pin. The pulse width is typically 
     * between 500 and 2500 microseconds, depending on the servo specifications.
     * @param pin The pin number (0-7) where the servo is connected.
     * @param pulse The desired pulse width in microseconds (e.g., 500-2500).
     * @return true if the operation was successful, false otherwise.
     */
    bool setServoPulse(uint8_t pin, uint16_t pulse);

    /**
     * Set the PWM value (8-bit) for a specific pin. This can be used to control the brightness of an LED 
     * or the speed of a motor.
     * @param pin The pin number (0-7) to set the PWM value for.
     * @param value The PWM value (0-255) to set for the specified pin.
     * @return true if the operation was successful, false otherwise.
     */
    bool setPWM(uint8_t pin, uint8_t value);

    /**
     * Set the RGB color for a specific pin. The color is represented as a 24-bit integer (0xRRGGBB).
     * @param pin The pin number (0-7) to set the RGB color for.
     * @param color The RGB color as a 24-bit integer (0xRRGGBB).
     * @return true if the operation was successful, false otherwise.
     */
    bool setLEDColor(uint8_t pin, uint32_t color);

    /**
     * Set the digital output state for a specific pin. The state can be either HIGH (1) or LOW (0).
     * @param pin The pin number (0-7) to set the digital output for.
     * @param state The desired state for the digital output (HIGH or LOW).
     * @return true if the operation was successful, false otherwise.
     */
    bool setDigitalOutput(uint8_t pin, uint8_t state);

    /**
     * Read the digital input state from a specific pin. The state will be either HIGH (1) or LOW (0).
     * @param pin The pin number (0-7) to read the digital input from.
     * @return The digital input state (HIGH or LOW) if the operation was successful, LOW otherwise.
     */
    uint8_t getDigitalInput(uint8_t pin);

    /**
     * Read the analog input value from a specific pin. The value can be read in either 8-bit or 12-bit mode.
     * @param pin The pin number (0-7) to read the analog input from.
     * @param bit The resolution for reading the analog input (8-bit or 12-bit).
     * @return The analog input value as a 16-bit unsigned integer if the operation was successful, 0 otherwise.
     */
    uint16_t getAnalogInput(uint8_t pin, extio_anolog_read_mode_t bit = _8bit);

    /**
     * Get the current consumption of the servo module.
     * @return The current in amperes as a floating-point value.
     */
    float getServoCurrent(void);
};