#pragma once

#include <Arduino.h>
#include <I2CDevice.hpp>

#define UNIT_PBHUB_I2C_ADDR 0x61

#define PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel) \
    if (channel <0 || channel > 7) return false; \
    if (channel == 5) channel++; // There is an empty row between ch4 and ch5, so we need to skip it.

#define PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index) \
    if (channel <0 || channel > 7) return false; \
    if (index < 0 ||index > 1) return false; \
    if (channel == 5) channel++; // There is an empty row between ch4 and ch5, so we need to skip it.

class M5UnitPbHub : public I2CDevice
{
public:
    M5UnitPbHub(TwoWire &wire, uint8_t addr = UNIT_PBHUB_I2C_ADDR) : I2CDevice(wire, addr) {}

    // -- SECTION IO

    /**
     * Write a value to a digital output pin on the specified channel and index.
     * @param channel The channel number (0-7) to write to.
     * @param index The index of the pin within the channel (0-1).
     * @param value The value to write to the pin (true for HIGH, false for LOW).
     * @return true if the write operation was successful, false otherwise.
     */
    bool digitalWrite(uint8_t channel, uint8_t index, bool value);

    /**
     * Read the state of a digital input pin on the specified channel and index.
     * @param channel The channel number (0-7) to read from.
     * @param index The index of the pin within the channel (0-1).
     * @return true if the pin is HIGH, false if the pin is LOW.
     */
    bool digitalRead(uint8_t channel, uint8_t index);

    /**
     * Read the value from an analog input pin on the specified channel and index.
     * @param channel The channel number (0-7) to read from.
     * @param index The index of the pin within the channel (0-1).
     * @return The 12-bit ADC value read from the pin (0-4095).
     */
    uint16_t analogRead(uint8_t channel);

    // -- SECTION PWM & SERVO
    /**
     * Set the PWM duty cycle for a pin on the specified channel and index.
     * @param channel The channel number (0-7) to write to.
     * @param index The index of the pin within the channel (0-1).
     * @param value The duty cycle value to set (0-255).
     * @return true if the write operation was successful, false otherwise.
     */
    bool setPWM(uint8_t channel, uint8_t index, uint8_t value);

    /**
     * Set the servo angle for a pin on the specified channel and index.
     * @param channel The channel number (0-7) to write to.
     * @param index The index of the pin within the channel (0-1).
     * @param value The angle to set the servo to (0 to 180 degrees).
     * @return true if the write operation was successful, false otherwise.
     */
    bool setServoAngle(uint8_t channel, uint8_t index, uint8_t value);

    /**
     * Set the servo pulse width for a pin on the specified channel and index.
     * @param channel The channel number (0-7) to write to.
     * @param index The index of the pin within the channel (0-1).
     * @param value The pulse width in microseconds to set the servo to (500-2500 microseconds).
     * @return true if the write operation was successful, false otherwise.
     */
    bool setServoPulse(uint8_t channel, uint8_t index, uint16_t value);

    // -- SECTION RGB LED
    /**
     * Set the number of RGB LEDs for a specified channel. Default is 74.
     * @param channel The channel number (0-7) to configure.
     * @param count The number of RGB LEDs to set for the channel.
     * @return true if the write operation was successful, false otherwise.
     */
    bool setLEDNum(uint8_t channel, uint16_t count);  // default 74

    /**
     * Set the RGB color for an LED on the specified channel and index.
     * @param channel The channel number (0-7) to write to.
     * @param index The index of the LED within the channel (0-1).
     * @param rgb888 The RGB color value in 0xRRGGBB format to set the LED to.
     * @return true if the write operation was successful, false otherwise.
     */
    bool setLEDColor(uint8_t channel, uint8_t index, uint32_t rgb888);

    /**
     * Fill a range of LEDs with a specific RGB color on the specified channel.
     * @param channel The channel number (0-7) to write to.
     * @param start The starting index of the LED range to fill.
     * @param count The number of LEDs to fill starting from the index.
     * @param rgb888 The RGB color value in 0xRRGGBB format
     * @return true if the write operation was successful, false otherwise.
     */
    bool fillLEDColor(uint8_t channel, uint8_t start, uint8_t count, uint32_t rgb888);

        /**
     * Set the brightness for all LEDs on the specified channel.
     * It's effective on the new LEDs write
     * @param channel The channel number (0-7) to configure.
     * @param value The brightness value to set (0-255).
     * @return true if the write operation was successful, false otherwise.
     */
    bool setLEDBrightness(uint8_t channel, uint8_t value);

    /**
     * Get the current LED show mode for the whole hub. The mode is shared across all channels and 
     * specifies what kind of LEDs are connected to the hub, which affects the timing of the LED data signal. 
     * The mode value is:
     *     0 for WS2812, WS2815, WS2816, SK6812
     *     1 for SK6822, APA106, PL9823
     * @return The LED show mode value (0 or 1).
     */
    uint8_t getLEDShowMode(void);

    /**
     * Set the LED show mode for the whole hub. The mode is shared across all channels and 
     * specifies what kind of LEDs are connected to the hub, which affects the timing of the LED data signal. 
     * The mode value is:
     *     0 for WS2812, WS2815, WS2816, SK6812
     *     1 for SK6822, APA106, PL9823
     * @param mode The LED show mode value to set (0 or 1).
     * @return true if the write operation was successful, false otherwise.
     */
    bool setLEDShowMode(uint8_t mode);

    /**
     * Get the firmware version of the PbHub unit.
     * @return The firmware version as an unsigned 8-bit integer.
     */
    uint8_t getFirmwareVersion(void);
};