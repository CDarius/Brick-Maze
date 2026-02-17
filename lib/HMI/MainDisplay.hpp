#pragma once

#include <AudioPlayer.hpp>
#include <PuzzleDisplay.hpp>
#include <TextAnimation.hpp>
#include <ImageTransitionAnimation.hpp>
#include <CancelToken.hpp>

#define MAIN_DISPLAY_MAX_FPS    20
#define MAIN_DISPLAY_MAX_FPS_MS (1000 / MAIN_DISPLAY_MAX_FPS)

#define MAIN_DISPLAY_MODE_COUNTDOWN 1
#define MAIN_DISPLAY_MODE_NO_GAME   2
#define MAIN_DISPLAY_MODE_GAME_OVER 3

class MainDisplay {
private:
    AudioPlayer& audioPlayer;
    PuzzleDisplay& display;
    TextAnimation& textAnimation;
    ImageTransitionAnimation& imageTransitionAnimation;

    uint8_t currentMode;
    CancelToken* cancelToken;

    // Countdown mode properties
    unsigned long countdownEndTimeMs;
    uint32_t countdownDurationMs;
    uint32_t countdownCriticalThresholdMs;

    // Modes update loops
    void noGameUpdateLoop();
    void countdownUpdateLoop();
    void gameOverUpdateLoop();
public:
    MainDisplay(AudioPlayer& audioPlayer, PuzzleDisplay& display, TextAnimation& textAnimation, ImageTransitionAnimation& imageTransitionAnimation) 
        : audioPlayer(audioPlayer), display(display), textAnimation(textAnimation), imageTransitionAnimation(imageTransitionAnimation) {
        setNoGameMode();
    }

    void setNoGameMode();
    void setCountdownMode(unsigned long endTimeMs, uint32_t durationMs, uint32_t criticalThresholdMs);
    void setGameOverMode();

    void updateLoop();
};