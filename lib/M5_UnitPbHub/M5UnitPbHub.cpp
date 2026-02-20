#include "M5UnitPbHub.hpp"

bool M5UnitPbHub::digitalWrite(uint8_t channel, uint8_t index, bool value)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index)
    uint8_t reg = ((channel + 4) << 4) | (0x00 + index);
    return writeByte(reg, (uint8_t)value);
}

bool M5UnitPbHub::digitalRead(uint8_t channel, uint8_t index)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index)
    uint8_t reg = ((channel + 4) << 4) | (0x04 + index);
    uint8_t value;
    return readByte(reg, &value) && (value != 0);
}

uint16_t M5UnitPbHub::analogRead(uint8_t channel)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel)
    uint8_t reg = ((channel + 4) << 4) | (0x06);
    uint16_t value;
    if (readUint16(reg, &value)) {
        return value;
    }
    return 0;
}

bool M5UnitPbHub::setPWM(uint8_t channel, uint8_t index, uint8_t value)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index)
    uint8_t reg = ((channel + 4) << 4) | (0x02 + index);
    return writeByte(reg, value);
}

bool M5UnitPbHub::setServoAngle(uint8_t channel, uint8_t index, uint8_t value)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index)
    uint8_t reg = ((channel + 4) << 4) | (0x0C + index);
    return writeByte(reg, value);
}

bool M5UnitPbHub::setServoPulse(uint8_t channel, uint8_t index, uint16_t value)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL_INDEX(channel, index)
    uint8_t reg = ((channel + 4) << 4) | (0x0E + index);

    uint8_t data[2];
    data[0] = value & 0xff;
    data[1] = value >> 8;
    return writeBytes(reg, data, 2);
}

bool M5UnitPbHub::setLEDNum(uint8_t channel, uint16_t count)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel)
    uint8_t reg = ((channel + 4) << 4) | (0x08);
    uint8_t data[2];
    data[0] = count & 0xff;
    data[1] = count >> 8;
    return writeBytes(reg, data, 2);
}

bool M5UnitPbHub::setLEDColor(uint8_t channel, uint8_t index, uint32_t rgb888)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel)
    uint8_t reg = ((channel + 4) << 4) | (0x09);
    uint8_t data[5];
    data[0] = index & 0xff;
    data[1] = index >> 8;
    data[2] = (rgb888 >> 16) & 0xff;
    data[3] = (rgb888 >> 8) & 0xff;
    data[4] = rgb888 & 0xff;
    return writeBytes(reg, data, 5);
}

bool M5UnitPbHub::fillLEDColor(uint8_t channel, uint8_t start, uint8_t count, uint32_t rgb888)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel)
    uint8_t reg = ((channel + 4) << 4) | (0x0A);
    uint8_t data[7];
    data[0] = start & 0xff;
    data[1] = start >> 8;
    data[2] = count & 0xff;
    data[3] = count >> 8;
    data[4] = (rgb888 >> 16) & 0xff;
    data[5] = (rgb888 >> 8) & 0xff;
    data[6] = rgb888 & 0xff;
    return writeBytes(reg, data, 7);
}

bool M5UnitPbHub::setLEDBrightness(uint8_t channel, uint8_t value)
{
    PB_HUB_VALIDATE_AND_FIX_CHANNEL(channel)
    uint8_t reg = ((channel + 4) << 4) | (0x0B);
    return writeByte(reg, value);
}

bool M5UnitPbHub::setLEDShowMode(uint8_t mode)
{
    return writeByte(0xFA, mode);
}

uint8_t M5UnitPbHub::getLEDShowMode(void)
{
    uint8_t value;
    if (readByte(0xFA, &value)) {
        return value;
    }
    return 0;
}

uint8_t M5UnitPbHub::getFirmwareVersion(void)
{
    uint8_t value;
    if (readByte(0xFE, &value)) {
        return value;
    }
    return 0;
}