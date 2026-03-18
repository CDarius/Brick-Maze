#include "MainDisplay.hpp"
#include "FallingChars.hpp"
#include "DemolitionCharsAnimation.hpp"
#include "CenterGrowAndFadeAnimation.hpp"
#include "AudioPlayer.hpp"

#define MAIN_DISPLAY_MAX_FPS    20
#define MAIN_DISPLAY_MAX_FPS_MS (1000 / MAIN_DISPLAY_MAX_FPS)

#define MAIN_DISPLAY_MODE_COUNTDOWN 1
#define MAIN_DISPLAY_MODE_NO_GAME   2
#define MAIN_DISPLAY_MODE_GAME_OVER 3
#define MAIN_DISPLAY_MODE_GAME_WIN  4
#define MAIN_DISPLAY_MODE_TABLE_LEVELING 5
#define MAIN_DISPLAY_MODE_END_GAME_TIME 6
#define MAIN_DISPLAY_MODE_END_GAME_HIGH_SCORE 7
#define MAIN_DISPLAY_MODE_READY_SET_GO 8
#define MAIN_DISPLAY_MODE_DONT_TOUCH 9

#define SKY_BLUE_GRADIENT_COLORS { \
    RgbColor(0, 50, 150), \
    RgbColor(0, 50, 150), \
    RgbColor(0, 50, 150), \
    RgbColor(130, 230, 255), \
    RgbColor(0, 180, 200), \
    RgbColor(0, 180, 200), \
    RgbColor(0, 180, 200), \
    RgbColor(0, 180, 200) \
}

#define NEON_GRADIENT_COLORS { \
    RgbColor(120, 0, 120), \
    RgbColor(120, 0, 120), \
    RgbColor(120, 0, 120), \
    RgbColor(255, 180, 230), \
    RgbColor(255, 0, 50), \
    RgbColor(255, 0, 50), \
    RgbColor(255, 0, 50), \
    RgbColor(255, 0, 50) \
}

#define GOLD_GRADIENT_COLORS { \
    RgbColor(139, 69, 19), \
    RgbColor(139, 69, 19), \
    RgbColor(139, 69, 19), \
    RgbColor(255, 230, 150), \
    RgbColor(255, 165, 0), \
    RgbColor(255, 165, 0), \
    RgbColor(255, 165, 0), \
    RgbColor(255, 165, 0) \
}

#define BRIGHT_GOLD_GRADIENT_COLORS { \
    RgbColor(119, 89, 0), \
    RgbColor(119, 89, 0), \
    RgbColor(119, 89, 0), \
    RgbColor(255, 230, 150), \
    RgbColor(255, 180, 0), \
    RgbColor(255, 180, 0), \
    RgbColor(255, 180, 0), \
    RgbColor(255, 180, 0), \
}

#define BRIGHT_RED_GRADIENT_COLORS { \
    RgbColor(100, 0, 0), \
    RgbColor(100, 0, 0), \
    RgbColor(100, 0, 0), \
    RgbColor(255, 180, 230), \
    RgbColor(255, 0, 0), \
    RgbColor(255, 0, 0), \
    RgbColor(255, 0, 0), \
    RgbColor(255, 0, 0), \
}

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

    /**
     * Helper function to draw the end game time with a shining trophy effect on the sides. 
     * The trophies will have a diagonal shine animation that loops every 16 frames.
     */
    void drawGameEndTime(PuzzleDisplay& display, RgbColor* textGradient, uint32_t timeMs, uint16_t glintFrame) {
        String gameTimeText = formatTimeSpan(timeMs);

        // Update display with the current time
        display.clear();
        display.drawCenteredString(0, gameTimeText, textGradient, FONT_6x8, true);
        
        // Show left and right trophy icons
        if (glintFrame >= 0) {
            drawShiningThropy(display, 8, 0, glintFrame);
            drawShiningThropy(display, TOTAL_WIDTH - 16, 0, glintFrame);
        } else {
            // If glintFrame is negative, draw static trophies without shine
            display.drawImage(8, 0, ICON_TROPHY_8x8, 8, 8);
            display.drawImage(TOTAL_WIDTH - 16, 0, ICON_TROPHY_8x8, 8, 8);
        }
    }    
}

void MainDisplay::setNoGameMode(bool playTitleAudio) {
    uint8_t newMode = MAIN_DISPLAY_MODE_NO_GAME;
    if (newMode == currentMode) 
        return; // No change

    currentMode = newMode;
    modeDone = false;

    if (playTitleAudio) {
        nextTitleAudioTimeMs = 0; // Play the title audio at the first iteration
    } else {
        nextTitleAudioTimeMs = millis() + TITLE_AUDIO_INTERVAL_MS; // Schedule next title audio in 10 minutes
    }
    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setDontTouchMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_DONT_TOUCH;
    if (newMode == currentMode)
        return; // No change

    currentMode = newMode;
    modeDone = false;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setReadySetGoMode() {
    uint8_t newMode = MAIN_DISPLAY_MODE_READY_SET_GO;
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

void MainDisplay::setEndGameTimeMode(uint32_t timeSpanMs) {
    uint8_t newMode = MAIN_DISPLAY_MODE_END_GAME_TIME;
    bool modeChanged = newMode != currentMode;

    if (!modeChanged) {
        return; // No change
    }

    currentMode = newMode;
    modeDone = false;
    endGameTimeSpanMs = timeSpanMs;

    // Cancel current ongoing mode loop
    if (cancelToken != nullptr) {
        cancelToken->cancel();
    }
}

void MainDisplay::setEndGameHighScoreMode(uint32_t timeSpanMs, GameLevel level, uint8_t rank) {
    uint8_t newMode = MAIN_DISPLAY_MODE_END_GAME_HIGH_SCORE;
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
    endGamePlayerName = "";

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

            case MAIN_DISPLAY_MODE_READY_SET_GO:
                readySetGoUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_DONT_TOUCH:
                dontTouchUpdateLoop();
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

            case MAIN_DISPLAY_MODE_END_GAME_TIME:
                endGameTimeUpdateLoop();
                break;

            case MAIN_DISPLAY_MODE_END_GAME_HIGH_SCORE:
                endGameHighScoreUpdateLoop();
                break;
        }
    }   
}

void MainDisplay::noGameUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    modeDone = true; // This mode doesn't have a defined end, so we can consider it done immediately    
    while (!localCancelToken.isCancelled()) {
        // ----------------------
        // -- GAME TITLE SCREEN --
        // ----------------------
        unsigned long now = millis();
        bool playTitleAudio = false;
        if (now >= nextTitleAudioTimeMs) {
            playTitleAudio = true;
            nextTitleAudioTimeMs = now + TITLE_AUDIO_INTERVAL_MS;
        }
        showBrickMazeTitleScreen(localCancelToken, playTitleAudio);
        IF_CANCELLED(localCancelToken, break;)

        // ----------------------
        // -- TODAY HIGH SCORES --
        // ----------------------
        showHighScoreList(GameLevel::EASY, localCancelToken);
        IF_CANCELLED(localCancelToken, break;)

        // --------------------------
        // -- ALL TIME HIGH SCORES --
        // --------------------------
        showHighScoreList(localCancelToken);
        IF_CANCELLED(localCancelToken, break;)
    }

    cancelToken = nullptr; // Clear cancel token reference when exiting loop
}

void MainDisplay::readySetGoUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    showReadySetGoAnimation(localCancelToken);

    if (!localCancelToken.isCancelled()) {
        modeDone = true;
    }

    while (!localCancelToken.isCancelled()) {
        delay(100); // Wait for mode change
    }

    cancelToken = nullptr; // Clear cancel token reference when exiting loop
}

void MainDisplay::dontTouchUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    audioPlayer.play(AUDIO_FILE_DONT_TOUCH);

    for (uint16_t i = 0; i < 14; i++) {
        if (localCancelToken.isCancelled()) {
            audioPlayer.stop();            
            break;
        }

        display.clear();
        display.drawCenteredString(0, "DON'T TOUCH", COLOR_RED, FONT_6x8);
        display.show();
        delay(100);

        display.clear();
        display.show();
        delay(60);
    }

    if (!localCancelToken.isCancelled()) {
        modeDone = true;
        setNoGameMode(); // Return to no game mode without replaying title audio immediately
    }

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

    cancelToken = nullptr; // Clear the token when exiting the loop
}

void MainDisplay::gameOverUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Crate color gradients for texts
    RgbColor redYelloMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_RED, COLOR_YELLOW, redYelloMirrorGradient, ANIM_TEXT_FONT_HEIGHT);

    // Draw "GAME OVER" on the display and copy it to buffer2
    display.clear();
    display.drawCenteredString(0, "GAME OVER", redYelloMirrorGradient, FONT_6x8);
    display.copyCanvasTo(_buffer2);

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
        delay(MAIN_DISPLAY_MAX_FPS_MS);
    }

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

    cancelToken = nullptr; // Clear cancel token reference when exiting function
}

void MainDisplay::endGameTimeUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Draw 0.00 game time on the display and copy it to buffer2
    RgbColor goldYellowMirrorGradient[ANIM_TEXT_FONT_HEIGHT];
    display.mirroredColorGradient(COLOR_GOLD, COLOR_YELLOW, goldYellowMirrorGradient, ANIM_TEXT_FONT_HEIGHT);
    drawGameEndTime(display, goldYellowMirrorGradient, 0, -1);
    display.copyCanvasTo(_buffer2);

    // Animate transition from previous screen to the end game time screen with a horizontal center inverse transition effect, 
    // using gold as the transition color for a celebratory feel
    imageTransitionAnimation.horizontalCenterInverseTransition(_buffer1, _buffer2, COLOR_GOLD.Dim(127), 300, localCancelToken);

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

    long startBeepFrequency = 300; // Low tone
    long endBeepFrequency = 900;   // High tone

    // --- PHASE 1: SPRINT (0% -> 80%) ---
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
        
        long pitchAttuale = map(f, 0, TOTAL_COUNTUP_FRAMES, startBeepFrequency, endBeepFrequency);
        audioPlayer.playTone(pitchAttuale, 25);
        
        delay(MAIN_DISPLAY_MAX_FPS_MS);
    }

    // --- PHASE 2: SUSPENSE (80% -> 100%) ---
    for (uint16_t f = 1; f < frameSuspense; f++) {
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
        display.show();
        
        long pitchAttuale = map(f + frameSprint, 0, TOTAL_COUNTUP_FRAMES, startBeepFrequency, endBeepFrequency);
        audioPlayer.playTone(pitchAttuale, 25);

        // Gradually increase delay to create a slowing down effect as we approach the final time
        delay(MAIN_DISPLAY_MAX_FPS_MS + (f * 3));
    }

    // Final display with the actual game time to ensure we end exactly on the correct time 
    // in case of any rounding issues during the animation. It also remove the shining effect 
    // on the trophy for a more static and celebratory final screen.
    display.clear();
    drawGameEndTime(display, goldYellowMirrorGradient, endGameTimeSpanMs, -1); // Pass -1 to disable glint effect for the final display
    display.show();

    // Signal mode completed only if the mode wasn't cancelled during the animation
    if (!localCancelToken.isCancelled()) {
        modeDone = true;
    }
    while (!localCancelToken.isCancelled()) {
        delay(100); // Wait for mode change
    }


    cancelToken = nullptr;
}

void MainDisplay::endGameHighScoreUpdateLoop() {
    CancelToken localCancelToken;
    cancelToken = &localCancelToken;

    RgbColor skyBlueGradient[ANIM_TEXT_FONT_HEIGHT] = SKY_BLUE_GRADIENT_COLORS;
    RgbColor neonGradient[ANIM_TEXT_FONT_HEIGHT] = NEON_GRADIENT_COLORS;

    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Draw "HIGH SCORE!" on the display and copy it to buffer2
    display.clear();
    display.drawCenteredString(0, "HIGH SCORE!", neonGradient, FONT_6x8, true);
    display.copyCanvasTo(_buffer2);

    audioPlayer.play(AUDIO_FILE_NEW_HIGHSCORE);
    // Animate transition from previous screen to the high score screen with a horizontal center inverse transition effect, using cyan as the transition color for a vibrant look
    imageTransitionAnimation.horizontalCenterInverseTransition(_buffer1, _buffer2, COLOR_CYAN, 300, localCancelToken);

    // Flash "HIGH SCORE!" text with alternating gradients to create a celebratory effect
    for (uint8_t i = 0; i < 12; i++) {
        IF_CANCELLED(localCancelToken, break;)

        display.clear();

        if (i % 2 == 0) {
            display.drawCenteredString(0, "HIGH SCORE!", skyBlueGradient, FONT_6x8, true);
        } else {
            display.drawCenteredString(0, "HIGH SCORE!", neonGradient, FONT_6x8, true);
        }

        display.show();
        delay(MAIN_DISPLAY_MAX_FPS_MS * 4);
    }

    // After flashing, show the final time, high score rank and let the player enter their name 
    // The player can change the character by moving the controller up or down, and confirm the character 
    // by pressing a button

    uint16_t frameCounter = 0;
    String playerName = String("");
    const String nameChars = String("ABCDEFGHIJKLMNOPQRSTUVWXYZ_.-+/") + String((char)DEL_FONT_CHAR) + String((char)END_FONT_CHAR);
    const int16_t nameCharsCount = nameChars.length();
    int16_t currentCharIndex = 0;
    float charStepAccumulator = 0.0f;
    uint32_t lastInputUpdateMs = millis();
    const float controllerDeadband = 0.2f;
    const float maxCharsPerSecond = 10.0f;
    bool buttonWasPressed = false;
    while (!localCancelToken.isCancelled() && playerName.length() < 3) {
        // Enter the char when the button is pressed
        if (controllerButtonPressed && !buttonWasPressed) {
            const char charToAdd = nameChars[currentCharIndex];
            if (charToAdd == DEL_FONT_CHAR) {
                if (playerName.length() > 0) {
                    playerName = playerName.substring(0, playerName.length() - 1); // Remove last character for delete
                }
            } else if (charToAdd == END_FONT_CHAR) {
                while (playerName.length() < 3) {
                    playerName += " "; // Pad with spaces if name is less than 3 characters when confirming
                }
            } else {
                playerName += charToAdd;
            }
        }
        buttonWasPressed = controllerButtonPressed;

        // Get delta time since last input update to calculate how many characters to move in the name selection based on controller input
        uint32_t nowMs = millis();
        float dtSeconds = (float)(nowMs - lastInputUpdateMs) / 1000.0f;
        lastInputUpdateMs = nowMs;

        // Read controller X axis and apply deadband to avoid unintentional character changes when the controller is near the center position. 
        // The effective input will be scaled to the range [-1, 1] after applying the deadband.
        float effectiveInput = 0.0f;
        if (controllerX > controllerDeadband) {
            effectiveInput = (controllerX - controllerDeadband) / (1.0f - controllerDeadband);
        } else if (controllerX < -controllerDeadband) {
            effectiveInput = (controllerX + controllerDeadband) / (1.0f - controllerDeadband);
        }

        charStepAccumulator += (effectiveInput * maxCharsPerSecond) * dtSeconds;

        if (controllerButtonPressed)
        {
            charStepAccumulator = 0; // Reset character step accumulator when the button is pressed to allow precise character selection without overshooting due to accumulated input
        }

        if (abs(controllerX) > controllerDeadband * 1.2f) {
            frameCounter = 0; // Don't blink while the player is actively changing the character to provide better feedback on the selection change
        }

        // Convert the accumulated character steps to an integer to determine how many characters to move in 
        // the name selection.
        int16_t charSteps = (int16_t)charStepAccumulator;
        if (charSteps != 0) {
            currentCharIndex = (currentCharIndex + charSteps + nameCharsCount) % nameCharsCount;
            charStepAccumulator -= static_cast<float>(charSteps);
        }

        // Display the current name selection with a blinking effect on the currently selected character to indicate that it's active. 
        String currentChar = String(nameChars[currentCharIndex]);
        bool blinkOn = frameCounter % 16 < 8;
        frameCounter++;

        if (blinkOn && playerName.length() < 3) {
            drawHighScroreLine(endGameTimeSpanMs, playerName + currentChar, endGameTimeRank);
        }
        else {
            drawHighScroreLine(endGameTimeSpanMs, playerName, endGameTimeRank); 
        }
        display.show();

        delay(MAIN_DISPLAY_MAX_FPS_MS);
    }

    // Signal mode completed and update endGamePlayerName only if the mode wasn't cancelled during the animation
    if (!localCancelToken.isCancelled()) {
        endGamePlayerName = playerName;
        modeDone = true;
    }

    // Wait for mode change to exit the loop and clear the cancel token reference
    while (!localCancelToken.isCancelled()) {
        delay(100);
    }

    cancelToken = nullptr;
}

void MainDisplay::drawHighScroreLine(uint32_t timeSpanMs, String name, uint8_t rank) {    
    RgbColor neonGradient[ANIM_TEXT_FONT_HEIGHT] = NEON_GRADIENT_COLORS;
    RgbColor goldGradient[ANIM_TEXT_FONT_HEIGHT] = GOLD_GRADIENT_COLORS;
    
    String timeText = formatTimeSpan(timeSpanMs);
    display.clear();
    display.drawRightString(0, timeText, goldGradient, FONT_6x8, true);
    display.drawString(0,0, String(rank + 1) + ".", goldGradient, FONT_6x8, true);
    display.drawString(15, 0, name, neonGradient, FONT_6x8, true);
}

void MainDisplay::showHighScoreList(GameLevel level, CancelToken& cancelToken) {
    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Draw high score level title on the display and copy it to buffer2
    display.clear();
    //String title = String("TOP ")  + gameLevelToString(level);
    String title = "TODAY TOP";
    RgbColor skyBlueGradient[ANIM_TEXT_FONT_HEIGHT] = SKY_BLUE_GRADIENT_COLORS;
    display.drawCenteredString(0, title, skyBlueGradient, FONT_6x8);
    display.copyCanvasTo(_buffer2);
    IF_CANCELLED(cancelToken, return;)

    // Animate transition from previous screen to the high score screen with a wipe transtion
    imageTransitionAnimation.horizontalWipeTransition(_buffer1, _buffer2, COLOR_CYAN, 2, 800, 30, cancelToken);
    IF_CANCELLED(cancelToken, return;)
    delayCancellable(2000, cancelToken, 100);
    IF_CANCELLED(cancelToken, return;)

    // Copy buffer2 back to buffer1 to use it as the new base for drawing the high score list
    memcpy(_buffer1, _buffer2, sizeof(_buffer1));

    // Draw high score entries with a slight delay between each to create a staggered reveal effect
    HighScore::Score score;
    for (uint8_t i = 0; i < highScore.SCORES_PER_LEVEL; i++) {
        // Draw the new score line and save it to buffer2, then animate a transition between buffer1 and buffer2 to create 
        // vertical page scroll effect for each new score line.
        highScore.read(level, i, score);
        display.clear();
        drawHighScroreLine(score.timeMs, score.name, i);
        display.copyCanvasTo(_buffer2);

        imageTransitionAnimation.verticalPageScrollTransition(_buffer1, _buffer2, 300, 30, cancelToken);
        IF_CANCELLED(cancelToken, return;)

        // Copy the current state of the display with the newly drawn score back to buffer1 for the next transition
        memcpy(_buffer1, _buffer2, sizeof(_buffer1));
        delayCancellable(1500, cancelToken, 100);
    }

    // Scroll out current content to leave a blank screen for the next animation
    display.copyCanvasTo(_buffer1);
    imageTransitionAnimation.verticalPageScrollOutTransition(_buffer1, 300, 30, cancelToken);
    IF_CANCELLED(cancelToken, break;)
    delayCancellable(1500, cancelToken, 100);
}

void MainDisplay::showHighScoreList(CancelToken& cancelToken) {
    RgbColor _buffer1[TOTAL_LEDS];
    RgbColor _buffer2[TOTAL_LEDS];

    // Capture current display state in buffer1 to use as starting point for the transition animation
    display.copyCanvasTo(_buffer1);

    // Draw all-time high score title on the display and copy it to buffer2
    display.clear();
    RgbColor skyBlueGradient[ANIM_TEXT_FONT_HEIGHT] = SKY_BLUE_GRADIENT_COLORS;
    display.drawCenteredString(0, "ALL TIME", skyBlueGradient, FONT_6x8);
    display.copyCanvasTo(_buffer2);
    IF_CANCELLED(cancelToken, return;)

    // Animate transition from previous screen to the all-time high score screen with a wipe transition
    imageTransitionAnimation.horizontalWipeTransition(_buffer1, _buffer2, COLOR_CYAN, 2, 800, 30, cancelToken);
    IF_CANCELLED(cancelToken, return;)
    delayCancellable(2000, cancelToken, 100);
    IF_CANCELLED(cancelToken, return;)

    // Copy buffer2 back to buffer1 to use it as the new base for drawing the high score list
    memcpy(_buffer1, _buffer2, sizeof(_buffer1));

    // Draw all-time high score entries with a slight delay between each to create a staggered reveal effect
    HighScore::AllTimeScore score;
    for (uint8_t i = 0; i < highScore.SCORES_PER_LEVEL; i++) {
        // Draw the new score line and save it to buffer2, then animate a transition between buffer1 and buffer2 to create
        // vertical page scroll effect for each new score line.
        highScore.readAllTime(i, score);
        display.clear();
        drawHighScroreLine(score.timeMs, score.name, i);
        display.copyCanvasTo(_buffer2);

        imageTransitionAnimation.verticalPageScrollTransition(_buffer1, _buffer2, 300, 30, cancelToken);
        IF_CANCELLED(cancelToken, return;)

        // Copy the current state of the display with the newly drawn score back to buffer1 for the next transition
        memcpy(_buffer1, _buffer2, sizeof(_buffer1));
        delayCancellable(1500, cancelToken, 100);
    }

    // Scroll out current content to leave a blank screen for the next animation
    display.copyCanvasTo(_buffer1);
    imageTransitionAnimation.verticalPageScrollOutTransition(_buffer1, 300, 30, cancelToken);
    IF_CANCELLED(cancelToken, break;)
    delayCancellable(1500, cancelToken, 100);
}

void MainDisplay::showBrickMazeTitleScreen(CancelToken& cancelToken, bool playTitleAudio) {
    RgbColor goldGradient[ANIM_TEXT_FONT_HEIGHT] = BRIGHT_GOLD_GRADIENT_COLORS;
    RgbColor redGradient[ANIM_TEXT_FONT_HEIGHT] = BRIGHT_RED_GRADIENT_COLORS;
    FallingCharsAnimation fallingAnimation(display, audioPlayer);

    // Get words widths to center them together on the display
    display.clear();
    uint16_t brickW = display.getStringWidth("BRICK", FONT_6x8);
    uint16_t mazeW = display.getStringWidth("MAZE", FONT_6x8);
    uint16_t totalTextWidth = brickW + 6 + mazeW;

    // Animate the fist word: BRICK
    uint16_t brickX = (display.getWidth() - totalTextWidth) / 2;
    fallingAnimation.InAnimation(brickX, "BRICK", redGradient, 600, cancelToken);
    IF_CANCELLED(cancelToken, return;)

    // Animate the second word: MAZE
    uint16_t mazeX = brickX + brickW + 6; // 6 pixels of spacing between the two words
    fallingAnimation.InAnimation(mazeX, "MAZE", goldGradient, 600, cancelToken);
    IF_CANCELLED(cancelToken, return;)

    if (!playTitleAudio) {
        delayCancellable(1500, cancelToken, 100);

        // Make the title blink
        for (uint8_t i = 0; i < 6; i++) {
            IF_CANCELLED(cancelToken, return;)

            display.clear();
            if (i % 2 == 0) {
                display.fillRect(0, 0, mazeX - 3, display.getHeight(), COLOR_RED);
                display.fillRect(mazeX - 3, 0, display.getWidth() - mazeX + 3, display.getHeight(), COLOR_GOLD);
                display.drawString(brickX, 0, "BRICK", COLOR_BLACK, FONT_6x8);
                display.drawString(mazeX, 0, "MAZE", COLOR_BLACK, FONT_6x8);            
            } else {
                display.drawString(brickX, 0, "BRICK", redGradient, FONT_6x8);
                display.drawString(mazeX, 0, "MAZE", goldGradient, FONT_6x8);
            }

            display.show();
            delay(50);
        } 
    } else {
        audioPlayer.play(AUDIO_FILE_BRICK_MAZE);

        // Wait for the laugh in the audio
        delayCancellable(2600, cancelToken, 100);
        IF_CANCELLED(cancelToken, { audioPlayer.stop(); return; })

        // Blink until the end of the audio to sync the title animation with the audio for a more impactful presentation
        uint16_t frame = 0;
        while (audioPlayer.isPlaying()) {
            IF_CANCELLED(cancelToken, { audioPlayer.stop(); return; })

            display.clear();
            if (frame % 2 == 0) {
                display.fillRect(0, 0, mazeX - 3, display.getHeight(), COLOR_RED);
                display.fillRect(mazeX - 3, 0, display.getWidth() - mazeX + 3, display.getHeight(), COLOR_GOLD);
                display.drawString(brickX, 0, "BRICK", COLOR_BLACK, FONT_6x8);
                display.drawString(mazeX, 0, "MAZE", COLOR_BLACK, FONT_6x8);            
            } else {
                display.drawString(brickX, 0, "BRICK", redGradient, FONT_6x8);
                display.drawString(mazeX, 0, "MAZE", goldGradient, FONT_6x8);
            }

            display.show();
            delay(50);
            frame++;
        }
    }

    // Run the demolition animation
    DemolitionCharsAnimation demolitionChars(display);
    demolitionChars.addText(brickX, "BRICK", redGradient);
    demolitionChars.addText(mazeX, "MAZE", goldGradient);
    demolitionChars.run(cancelToken);

    // Small pause after the demolition animation before starting the next screen to give a moment of visual rest
    delayCancellable(1500, cancelToken, 100);
}

void MainDisplay::showReadySetGoAnimation(CancelToken& cancelToken) {
    CenterGrowAndFadeAnimation centerGrowAndFade(display, 150, 700, 200, 33);

    RgbColor redGradient[ANIM_TEXT_FONT_HEIGHT];
    RgbColor ambraGradient[ANIM_TEXT_FONT_HEIGHT];
    RgbColor greenGradient[ANIM_TEXT_FONT_HEIGHT];

    display.linearColorGradient(RgbColor(255, 30, 30), RgbColor(100, 0, 0), redGradient, ANIM_TEXT_FONT_HEIGHT);
    display.linearColorGradient(RgbColor(255, 200, 0), RgbColor(150, 50, 0), ambraGradient, ANIM_TEXT_FONT_HEIGHT);
    display.linearColorGradient(RgbColor(150, 255, 0), RgbColor(0, 80, 0), greenGradient, ANIM_TEXT_FONT_HEIGHT);

    // Show "READY"
    audioPlayer.play(AUDIO_FILE_START_BEEP_SHORT);
    centerGrowAndFade.animate("READY", RgbColor(255, 30, 30), redGradient, cancelToken);
    audioPlayer.setVolume(0); // Hack to avoid the click sound when the audio end
    delay(50);
    audioPlayer.setVolume(21);
    IF_CANCELLED(cancelToken, return;)

    // Show "SET"
    audioPlayer.play(AUDIO_FILE_START_BEEP_SHORT);
    centerGrowAndFade.animate("SET", RgbColor(255, 200, 0), ambraGradient, cancelToken);
    audioPlayer.setVolume(0); // Hack to avoid the click sound when the audio end
    delay(50);
    audioPlayer.setVolume(21);
    IF_CANCELLED(cancelToken, return;)

    // Show "GO!"
    audioPlayer.play(AUDIO_FILE_START_BEEP_LONG);
    for (uint8_t i = 0; i < 6; i++) {
        IF_CANCELLED(cancelToken, {
            audioPlayer.stop();
            audioPlayer.setVolume(21);
            return;
        })

        display.clear();
        display.drawCenteredString(0, "GO!", greenGradient, FONT_6x8, true);
        display.show();
        delay(100);

        display.clear();
        display.show();
        delay(60);
    }
    audioPlayer.setVolume(0); // Hack to avoid the click sound when the audio end
    delay(50);
    audioPlayer.setVolume(21);
}