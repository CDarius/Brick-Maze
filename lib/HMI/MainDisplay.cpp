#include "MainDisplay.hpp"

namespace {
    /**
     * Helper function to format a time span in milliseconds into a string with seconds 
     * and centiseconds (e.g., "12.34")
     */
    String formatTimeSpan(uint32_t timeSpanMs) {
        // Format time as seconds with 2 decimal places
        uint32_t centiseconds = (timeSpanMs + 5) / 10; // Round milliseconds to 2 decimals
        uint32_t wholeSeconds = centiseconds / 100;
        uint32_t fractionalPart = centiseconds % 100;

        String wholeSecondsText = String(wholeSeconds);
        if (wholeSeconds < 10) {
            wholeSecondsText = "0" + wholeSecondsText;
        }

        String fractionalText = String(fractionalPart);
        if (fractionalPart < 10) {
            fractionalText = "0" + fractionalText;
        }

        String gameTimeText = wholeSecondsText + "." + fractionalText;
        return gameTimeText;
    }

    /**
     * Draws the trophy icon with a shining effect by alternating between the base gold color 
     * and the shine color in a diagonal pattern
     */
    void drawShiningThropy(PuzzleDisplay& display, uint16_t xOffset, uint16_t yOffset, uint16_t frame) {
        uint16_t displayWidth = display.getWidth();
        uint16_t displayHeight = display.getHeight();
        
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 8; x++) {
                RgbColor pixelColor = ICON_TROPHY_8x8[y * 8 + x];

                // Se il pixel è parte del corpo dorato della coppa, applichiamo il riflesso
                if (pixelColor == C_GOLD || pixelColor == C_SHINE) {
                    
                    // Creiamo una diagonale: se x + y è uguale al frame attuale, illumina il pixel
                    // Usiamo un range (es. frame, frame-1) per rendere la scia più spessa
                    if ((x + y) == frame || (x + y) == (frame - 1)) {
                        pixelColor = C_SHINE; 
                    } else {
                        pixelColor = C_GOLD; // Torna al colore base
                    }
                }

                int targetX = x + xOffset;
                int targetY = y + yOffset;
                
                if (targetX >= 0 && targetX < displayWidth && targetY >= 0 && targetY < displayHeight) {
                    display.drawPixel(targetX, targetY, pixelColor);
                }
            }
        }
    }
}

void MainDisplay::setNoGameMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_NO_GAME;
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;
    modeDone = false;

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
    modeDone = false;

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
    modeDone = false;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setGameWinMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_GAME_WIN;
    if (newMode == currentMode) 
        return; // No change
        
    currentMode = newMode;
    modeDone = false;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }   
}

void MainDisplay::setTableLevelingMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_TABLE_LEVELING;
    if (newMode == currentMode)
        return; // No change

    currentMode = newMode;
    modeDone = false;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setShowEndGameTimeMode(uint32_t timeSpanMs) {
    uint8_t newMode = MAIN_DISPLAY_MODE_SHOW_GAME_TIME;
    bool modeChanged = newMode != currentMode;

    if (!modeChanged) {
        return; // No change
    }

    currentMode = newMode;
    modeDone = false;
    endGameTimeSpanMs = timeSpanMs;
    endGameTimeIsNewRecord = false;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setShowEndGameHighScoreMode(uint32_t timeSpanMs, GameLevel level, uint8_t rank) {
    uint8_t newMode = MAIN_DISPLAY_MODE_SHOW_GAME_TIME;
    bool modeChanged = newMode != currentMode;

    if (!modeChanged) {
        return; // No change
    }

    currentMode = newMode;
    modeDone = false;
    endGameTimeSpanMs = timeSpanMs;
    endGameTimeIsNewRecord = true;
    endGameTimeGameLevel = level;
    endGameTimeRank = rank;

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

            case MAIN_DISPLAY_MODE_GAME_WIN:
                gameWinUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_TABLE_LEVELING:
                tableLevelingUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_SHOW_GAME_TIME:
                showEndGameTimeUpdateLoop();
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
        IF_CANCELLED(localCancelToken, break;)
        delayCancellable(1000, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("USE THE CONTROLLER", redYelloMirrorGradient, TEXT_POSITION_LEFT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("TO MOVE THE MAZE.", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("REACH THE END", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("BEFORE THE TIMER", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("RUNS OUT", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        textAnimation.verticalScrollIn("", redYelloMirrorGradient, TEXT_POSITION_RIGHT, 2, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
        delayCancellable(2000, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
    };

    modeDone = true;
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

            if (remainingTimeMs == 0) {
                modeDone = true; // Signal that countdown has finished
            }
        }

        delay(MAIN_DISPLAY_MAX_FPS_MS); // Limit update rate to max FPS
    }

    modeDone = true;
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
        if (!modeDone) {
            if (!audioPlayer.isPlaying()) {
                modeDone = true; // Signal that game win animation has finished when audio finishes playing
            }
        }
        delay(50);
    }

    modeDone = true; // Ensure modeDone is set to true when exiting the loop in case of cancellation
    cancelToken = nullptr; // Clear cancel token reference when exiting function
}

void MainDisplay::gameWinUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Crate color gradients for texts
    RgbColor greenYellowMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_GREEN, COLOR_YELLOW, greenYellowMirrorGradient, ANIM_TEXT_FONT_HEIGHT);

    // Draw "YOU WIN!" on the display and copy it to buffer2
    display.fill(COLOR_MAGENTA.Dim(64)); // Dimmed magenta background for better contrast with red/yellow text
    display.drawCenteredString(0, "YOU WIN!", greenYellowMirrorGradient, FONT_6x8);
    display.copyCanvasTo(_buffer2);

    audioPlayer.play(AUDIO_FILE_GAME_WIN);
    imageTransitionAnimation.horizontalCenterTransition(_buffer1, _buffer2, COLOR_GREEN, 300, localCancelToken);

    // Wait in this loop until the mode changes to avoid to repeat the animation
    while (!localCancelToken.isCancelled()) {
        // Slowly cycle colors of the "YOU WIN!" text to create a dynamic effect while waiting for mode change
        RgbColor last = greenYellowMirrorGradient[ANIM_TEXT_FONT_HEIGHT - 1];
        RgbColor newColor;
        for(int i = ANIM_TEXT_FONT_HEIGHT - 1; i > 0; i--) {        
            greenYellowMirrorGradient[i] = greenYellowMirrorGradient[i - 1];
        }
        greenYellowMirrorGradient[0] = last;
        display.fill(COLOR_MAGENTA.Dim(64)); // Dimmed magenta background for better contrast with red/yellow text
        display.drawCenteredString(0, "YOU WIN!", greenYellowMirrorGradient, FONT_6x8);
        display.show();

        if (!modeDone) {
            if (!audioPlayer.isPlaying()) {
                modeDone = true; // Signal that game win animation has finished when audio finishes playing
            }
        }
        delay(50);
    }

    modeDone = true; // Ensure modeDone is set to true when exiting the loop in case of cancellation
    cancelToken = nullptr; // Clear cancel token reference when exiting function
}

void MainDisplay::tableLevelingUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    const char* frames[] = {"|", "/", "-", "\\"};
    const uint8_t frameCount = sizeof(frames) / sizeof(frames[0]);
    uint8_t frameIndex = 0;

    RgbColor cyanBlueMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_CYAN, COLOR_BLUE, cyanBlueMirrorGradient, ANIM_TEXT_FONT_HEIGHT);

    while (!localCancelToken.isCancelled()) {
        String animChar = String(frames[frameIndex]);
        String animText = "LEVELING " + animChar;

        display.clear();
        display.drawString(1, 0, animText, cyanBlueMirrorGradient, FONT_6x8);
        display.show();

        frameIndex = (frameIndex + 1) % frameCount;
        delay(160);
    }

    modeDone = true;
    cancelToken = nullptr; // Clear cancel token reference when exiting function
}

void MainDisplay::showEndGameTimeUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    const float TOTAL_COUNTUP_TIME_S = 2.0; // Total time for the count-up animation in seconds
    const uint16_t TOTAL_COUNTUP_FRAMES = static_cast<uint16_t>(TOTAL_COUNTUP_TIME_S * MAIN_DISPLAY_MAX_FPS); // Total frames for the count-up animation based on max FPS
    const float SLOW_DOWN_FACTOR  = 0.8; // 80% of the count-up animation before slowing down

    uint32_t displayedTimeMs = 0;
    uint16_t glintFrame = 0;

    // Split frames: half for the sprint, half for the suspense
    const uint16_t frameSprint = TOTAL_COUNTUP_FRAMES * 0.5; 
    const uint16_t frameSuspense = TOTAL_COUNTUP_FRAMES - frameSprint;

    uint32_t timeForSprintMs = static_cast<uint32_t>(endGameTimeSpanMs * SLOW_DOWN_FACTOR);
    uint32_t residualTimeMs = endGameTimeSpanMs - timeForSprintMs;

    // --- PHASE 1: SPRINT (0% -> 80%) ---
    RgbColor goldYellowMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_GOLD, COLOR_YELLOW, goldYellowMirrorGradient, ANIM_TEXT_FONT_HEIGHT);
    for (uint16_t f = 0; f <= frameSprint; f++) {
        IF_CANCELLED(localCancelToken, break;)

        // Calculate the time to display for this frame using a quadratic easing for a more dynamic effect
        displayedTimeMs = (timeForSprintMs * f) / frameSprint;
        String gameTimeText = formatTimeSpan(displayedTimeMs);

        // Update display with the current time
        display.clear();
        display.drawCenteredString(0, gameTimeText, goldYellowMirrorGradient, FONT_6x8, true);
        
        // Show left and right trophy icons
        drawShiningThropy(display, 8, 0, glintFrame);
        drawShiningThropy(display, TOTAL_WIDTH - 16, 0, glintFrame);
        glintFrame = (++glintFrame) % 16; // Loop glint frame for shining effect
        
        display.show();
        delay(MAIN_DISPLAY_MAX_FPS_MS);
    }

    // --- PHASE 2: SUSPENSE (80% -> 100%) ---
    for (uint16_t f = 1; f <= frameSuspense; f++) {
        // Slower easing for suspense effect as we approach the final time
        IF_CANCELLED(localCancelToken, break;)
        displayedTimeMs = timeForSprintMs + (residualTimeMs * f) / frameSuspense;
        String gameTimeText = formatTimeSpan(displayedTimeMs);

        // Update display with the current time
        display.clear();
        display.drawCenteredString(0, gameTimeText, goldYellowMirrorGradient, FONT_6x8, true);
        
        // Show left and right trophy icons
        drawShiningThropy(display, 8, 0, glintFrame);
        drawShiningThropy(display, TOTAL_WIDTH - 16, 0, glintFrame);
        glintFrame = (++glintFrame) % 16; // Loop glint frame for shining effect
        
        // Gradually increase delay to create a slowing down effect as we approach the final time
        display.show();
        delay(MAIN_DISPLAY_MAX_FPS_MS + (f * 3));
    }

    // Final display with the actual game time to ensure we end exactly on the correct time 
    // in case of any rounding issues during the animation. It also remove the shining effect 
    // on the trophy for a more static and celebratory final screen.
    display.clear();
    String finalTimeText = formatTimeSpan(endGameTimeSpanMs);
    display.drawImage(8, 0, ICON_TROPHY_8x8, 8, 8);
    display.drawImage(TOTAL_WIDTH - 16, 0, ICON_TROPHY_8x8, 8, 8);
    display.drawCenteredString(0, finalTimeText, goldYellowMirrorGradient, FONT_6x8, true);
    display.show();
    
    modeDone = true;
    while (!localCancelToken.isCancelled()) {
        delay(100); // Wait for mode change
    }

    cancelToken = nullptr;
}