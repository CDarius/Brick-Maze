#pragma once

#include <AudioPlayer.hpp>
#include <PuzzleDisplay.hpp>
#include <Icons.hpp>
#include <TextAnimation.hpp>
#include <ImageTransitionAnimation.hpp>
#include <GameLevel.hpp>
#include <HighScore.hpp>
#include <CancelToken.hpp>

constexpr unsigned long TITLE_AUDIO_INTERVAL_MS = 10 * 60 * 1000;

class MainDisplay {
public:
    MainDisplay(AudioPlayer& audioPlayer, PuzzleDisplay& display, HighScore& highScore)
        : audioPlayer(audioPlayer), display(display), highScore(highScore),textAnimation(display), imageTransitionAnimation(display) {
        setNoGameMode();
    }

    void setNoGameMode(bool playTitleAudio = false);
    void setDontTouchMode();
    void setReadySetGoMode();
    void setCountdownMode(unsigned long endTimeMs, uint32_t durationMs, uint32_t criticalThresholdMs);
    void setGameOverMode();
    void setGameWinMode();
    void setTableLevelingMode();
    void setEndGameTimeMode(uint32_t timeSpanMs);
    void setEndGameHighScoreMode(uint32_t timeSpanMs, GameLevel level, uint8_t rank);
    void updateLoop();
    void updateControllerStatus(float x, float y, bool buttonPressed) {
        controllerX = x;
        controllerY = y;
        controllerButtonPressed = buttonPressed;
    }

    bool isModeDone() const {
        return modeDone;
    }

    String getEndGamePlayerName() const {
        return endGamePlayerName;
    }

private:
    AudioPlayer& audioPlayer;
    PuzzleDisplay& display;
    HighScore& highScore;
    TextAnimation textAnimation;
    ImageTransitionAnimation imageTransitionAnimation;

    uint8_t currentMode;
    bool modeDone;
    CancelToken* cancelToken;

    unsigned long nextTitleAudioTimeMs;

    // Countdown mode properties
    unsigned long countdownEndTimeMs;
    uint32_t countdownDurationMs;
    uint32_t countdownCriticalThresholdMs;

    // Show end game time mode properties
    uint32_t endGameTimeSpanMs;
    bool endGameTimeIsNewRecord;
    GameLevel endGameTimeGameLevel;
    uint8_t endGameTimeRank;
    String endGamePlayerName;

    // Controller status
    float controllerX;
    float controllerY;
    bool controllerButtonPressed;

    // Modes update loops
    void noGameUpdateLoop();
    void dontTouchUpdateLoop();
    void readySetGoUpdateLoop();
    void countdownUpdateLoop();
    void gameOverUpdateLoop();
    void gameWinUpdateLoop();
    void tableLevelingUpdateLoop();
    void endGameTimeUpdateLoop();
    void endGameHighScoreUpdateLoop();

    void drawHighScroreLine(uint32_t timeSpanMs, String name, uint8_t rank);
    void showHighScoreList(GameLevel level, CancelToken& cancelToken);
    void showBrickMazeTitleScreen(CancelToken& cancelToken, bool playTitleAudio);
    void showReadySetGoAnimation(CancelToken& cancelToken);
};