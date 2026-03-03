#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include <GameLevel.hpp>

class HighScore {
public:
    static constexpr uint8_t SCORES_PER_LEVEL = 9;

    struct Score {
        char name[4];
        uint16_t timeMs;
    };

    bool begin();

    bool read(GameLevel level, uint8_t rank, Score& outScore) const;
    int8_t write(GameLevel level, const Score& score);
    int8_t getHighScoreRank(GameLevel level, uint16_t time) const;
    bool overwriteWithDefaultScores();

private:
    static constexpr const char* NVS_NAMESPACE = "brickmaze";
    static constexpr const char* NVS_KEY = "hscores";

    Preferences preferences;
    Score scores[GAME_LEVEL_COUNT][SCORES_PER_LEVEL];
    bool initialized = false;

    void loadHardcodedScores();
    bool persistAll();
    static void normalizeScore(Score& score);
};
