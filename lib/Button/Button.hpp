// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <Arduino.h>

class Button
{
public:
    enum button_state_t : uint8_t
    {
        state_nochange,
        state_clicked,
        state_hold,
        state_decide_click_count
    };

    /// Returns true when the button is pressed briefly and released.
    bool wasClicked(void) const { return _currentState == state_clicked; }

    /// Returns true when the button has been held pressed for a while.
    bool wasHold(void) const { return _currentState == state_hold; }

    /// Returns true when some time has passed since the button was single clicked.
    bool wasSingleClicked(void) const { return _currentState == state_decide_click_count && _clickCount == 1; }

    /// Returns true when some time has passed since the button was double clicked.
    bool wasDoubleClicked(void) const { return _currentState == state_decide_click_count && _clickCount == 2; }

    /// Returns true when some time has passed since the button was multiple clicked.
    bool wasDecideClickCount(void) const { return _currentState == state_decide_click_count; }

    uint8_t getClickCount(void) const { return _clickCount; }

    /// Returns true if the button is currently held pressed.
    bool isHolding(void) const { return _press == 2; }
    bool wasChangePressed(void) const { return ((bool)_press) != ((bool)_oldPress); }

    bool isPressed(void) const { return _press; }
    bool isReleased(void) const { return !_press; }
    bool wasPressed(void) const { return !_oldPress && _press; }
    bool wasReleased(void) const { return _oldPress && !_press; }
    bool wasReleasedAfterHold(void) const { return !_press && _oldPress == 2; }
    bool wasReleaseFor(uint32_t ms) const { return _oldPress && !_press && _lastHoldPeriod >= ms; }

    bool pressedFor(uint32_t ms) const { return (_press && _lastMsec - _lastChange >= ms); }
    bool releasedFor(uint32_t ms) const { return (!_press && _lastMsec - _lastChange >= ms); }

    void setDebounceThresh(uint32_t msec) { _msecDebounce = msec; }
    void setHoldThresh(uint32_t msec) { _msecHold = msec; }

    void setRawState(uint32_t msec, bool press);
    void setState(uint32_t msec, button_state_t state);
    button_state_t getState(void) const { return _currentState; }
    uint32_t lastChange(void) const { return _lastChange; }

    uint32_t getDebounceThresh(void) const { return _msecDebounce; }
    uint32_t getHoldThresh(void) const { return _msecHold; }

    uint32_t getUpdateMsec(void) const { return _lastMsec; }

private:
    uint32_t _lastMsec = 0;
    uint32_t _lastChange = 0;
    uint32_t _lastRawChange = 0;
    uint32_t _lastClicked = 0;
    uint16_t _msecDebounce = 10;
    uint16_t _msecHold = 500;
    uint16_t _lastHoldPeriod = 0;
    button_state_t _currentState = state_nochange; // 0:nochange  1:click  2:hold
    bool _raw_press = false;
    uint8_t _press = 0; // 0:release  1:click  2:holding
    uint8_t _oldPress = 0;
    uint8_t _clickCount = 0;
};