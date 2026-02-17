#include "MainDisplay.hpp"

void MainDisplay::setNoGameMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_NO_GAME;
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setCountdownMode(unsigned long endTimeMs, uint32_t durationMs, uint32_t criticalThresholdMs) {
    uint8_t newMode = MAIN_DISPLAY_MODE_COUNTDOWN;
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;

    // Store countdown parameters
    countdownEndTimeMs = endTimeMs;
    countdownDurationMs = durationMs;
    countdownCriticalThresholdMs = criticalThresholdMs;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setGameOverMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_GAME_OVER;
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::updateLoop() {
    // This will be called in the main loop to update the display based on the current mode
    while (true)
    {
        switch (currentMode) {
            case MAIN_DISPLAY_MODE_NO_GAME:
                noGameUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_COUNTDOWN:
                countdownUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_GAME_OVER:
                gameOverUpdateLoop();
                break;
        }
    }   
}

void MainDisplay::noGameUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    // Crate color gradients for texts
    RgbColor redYelloMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_RED, COLOR_YELLOW, redYelloMirrorGradient, ANIM_TEXT_FONT_HEIGHT);

    while (!localCancelToken.isCancelled()) {
        textAnimation.showText("HOW TO PLAY:", redYelloMirrorGradient, TEXT_POSITION_CENTER);
        delay(1000);
        textAnimation.verticalScrollIn("USE THE CONTROLLER", redYelloMirrorGradient, TEXT_POSITION_LEFT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        textAnimation.verticalScrollIn("TO MOVE THE MAZE.", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        textAnimation.verticalScrollIn("REACH THE END", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        textAnimation.verticalScrollIn("BEFORE THE TIMER", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        textAnimation.verticalScrollIn("RUNS OUT", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        textAnimation.verticalScrollIn("", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        delay(2000);
    };

    cancelToken = nullptr; // Clear cancel token reference when exiting loop
}

void MainDisplay::countdownUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;
    int16_t stripeOffset = 0;
    int16_t seconds = -1;

    while (!localCancelToken.isCancelled()) {
        // Calculate remaining time
        unsigned long currentTime = millis();
        uint32_t remainingTimeMs;
        if (currentTime < countdownEndTimeMs) {
            remainingTimeMs = countdownEndTimeMs - currentTime;
        } else {
            remainingTimeMs = 0;
        }

        if (remainingTimeMs > countdownCriticalThresholdMs) {
            // Draw a progress bar on the display based on remaining time
            float progress = (float)remainingTimeMs / (float)countdownDurationMs;
            int16_t numCols = (int16_t)(display.getWidth() * progress);

            float colorProgress = (float)(remainingTimeMs - countdownCriticalThresholdMs) / (float)(countdownDurationMs - countdownCriticalThresholdMs);
            RgbColor barColor = RgbColor::LinearBlend(COLOR_RED, COLOR_GREEN, colorProgress);
            RgbColor currentBarColor = barColor;

            display.clear();
            int16_t dw = display.getWidth();
            int16_t l = display.getHeight() - 1;
            for (int16_t x = 0; x < dw; x++) {                    
                if (x == numCols) {
                    // Dim bar color for elapsed bars
                    barColor = currentBarColor.Dim(64);
                }
                
                if (x == (numCols - 1)) {
                    // This is the last lit bar. Progressively dim it based on how close we are to the next column to light up 
                    float dimFactor = (display.getWidth() * progress) - numCols; // Calculate how far we are into the current bar
                    uint8_t dimAmount = (uint8_t)(64 + (255 - 64) * dimFactor); // Dim between 64 (fully dimmed) and 255 (full brightness)

                    barColor = currentBarColor.Dim(dimAmount);
                }

                display.drawLine(x, 0, x, l, barColor);
            }
            display.show();
        } else {
            uint16_t lineWidth = 4;
            bool blink = millis() % 400 < 200; // Alternate every 200ms for blinking effect
            RgbColor textColor = blink ? COLOR_RED : COLOR_ORANGE;

            // Draw crittical time indication
            display.fill(COLOR_RED);

            stripeOffset -= 2; // Move stripes by 2 pixel on each blink change to create a dynamic pattern
            if (stripeOffset <= (-2 * lineWidth)) {
                stripeOffset = 0; // Reset offset to create a looping diagonal pattern
            }

            // Draw diagonal lines to create a warning pattern (first screen half)
            int16_t h = display.getHeight();
            int16_t w = display.getWidth();
            int16_t halfW = w / 2;
            for (int16_t y = 0; y < h; y++) {
                int16_t xs = y < lineWidth ? y : y - 2 * lineWidth; // Start x position, creating a diagonal effect
                xs += stripeOffset; // Apply dynamic offset for blinking effect
                for (int16_t x = xs; x < halfW; x += 2 * lineWidth) {
                    display.drawLine(x, y, x + lineWidth - 1, y, COLOR_ORANGE);
                }
            }

            // Draw diagonal lines to create a warning pattern (second screen half)
            for (int16_t y = 0; y < h; y++) {
                int16_t xs = w - y - 1;
                if (y >= lineWidth)
                    xs += 2 * lineWidth; // Start x position, creating a diagonal effect
                xs -= stripeOffset; // Apply dynamic offset for blinking effect
                for (int16_t x = xs; x > halfW; x -= 2 * lineWidth) {
                    display.drawLine(x - lineWidth + 1, y, x, y, COLOR_ORANGE);
                }
            }

            // Draw remaining time in seconds at the center of the display
            String timerText = String(remainingTimeMs / 1000.0, 2); // Show 2 decimal places
            uint16_t textWidth = display.getStringWidth(timerText, FONT_6x8, true);
            int16_t xPos = (w - textWidth) / 2; // Center the text
            int16_t xBackgroundStart = xPos - 1;
            int16_t xBackgroundEnd = xPos + textWidth + 1;
            for (int16_t x = xBackgroundStart; x < xBackgroundEnd; x++) {
                display.drawLine(x, 0, x, h - 1, COLOR_ORANGE.Dim(64));
            }
            display.drawLine(0, 0, 0, h - 1, COLOR_RED);
            display.drawLine(xBackgroundStart - 1, 0, xBackgroundStart - 1, h - 1, COLOR_RED);
            display.drawLine(xBackgroundEnd, 0, xBackgroundEnd, h - 1, COLOR_RED);
            display.drawString(xPos, 0, timerText, textColor, FONT_6x8, true);
            display.drawLine(w - 1, 0, w - 1, h - 1, COLOR_RED);

            display.show();

            uint16_t remainingSeconds = remainingTimeMs / 1000;
            if (seconds != remainingSeconds) {
                seconds = remainingSeconds;
                audioPlayer.play(AUDIO_FILE_WARNING_BEEP);
            }
        }

        delay(MAIN_DISPLAY_MAX_FPS_MS); // Limit update rate to max FPS
    }

    cancelToken = nullptr; // Clear the token when exiting the loop
}

void MainDisplay::gameOverUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    display.copyCanvasTo(_buffer1);

    // Crate color gradients for texts
    RgbColor redYelloMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_RED, COLOR_YELLOW, redYelloMirrorGradient, ANIM_TEXT_FONT_HEIGHT);

    // Draw "GAME OVER" on the display and copy it to buffer2
    display.clear();
    display.drawCenteredString(0, "GAME OVER", redYelloMirrorGradient, FONT_6x8);
    display.copyCanvasTo(_buffer2);

    //audioPlayer.play(AUDIO_FILE_GAME_OVER);
    audioPlayer.play(AUDIO_FILE_GAME_OVER);
    imageTransitionAnimation.horizontalCenterTransition(_buffer1, _buffer2, COLOR_RED, 300, localCancelToken);

    // Wait in this loop until the mode changes to avoid to repeat the animation
    while (!localCancelToken.isCancelled()) {        
        delay(20);
    }
    cancelToken = nullptr; // Clear cancel token reference when exiting function
}