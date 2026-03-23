#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include <GameLevel.hpp>
#include <GameConfig.h>

class HighScore {
public:
    static constexpr uint8_t SCORES_PER_LEVEL = 9;

    struct Score {
        char name[4];
        uint16_t timeMs;
    };

    struct AllTimeScore {
        char name[4];
        uint16_t timeMs;
        GameLevel level;
    };

    bool begin(GameConfig config);

    bool read(GameLevel level, uint8_t rank, Score& outScore) const;
    int8_t write(GameLevel level, const Score& score);
    int8_t getHighScoreRank(GameLevel level, uint16_t time) const;

    bool readAllTime(uint8_t rank, AllTimeScore& outScore) const;

    bool overwriteWithDefaultScores(GameConfig config);

private:
    static constexpr const char* NVS_NAMESPACE = "brickmaze";
    static constexpr const char* NVS_KEY_TODAY = "today";
    static constexpr const char* NVS_KEY_ALLTIME = "alltime";

    Preferences preferences;
    Score scores[GAME_LEVEL_COUNT][SCORES_PER_LEVEL];
    AllTimeScore allTimeScores[SCORES_PER_LEVEL];
    bool initialized = false;

    void loadDefaultScores(GameConfig config);
    void loadDefaultAllTimeScores(GameConfig config);
    bool persistLevelScores();
    bool persistAllTimeScores();
    bool persistAll();
    int8_t updateTodayScores(GameLevel level, const Score& score);
    void updateAllTimeScores(GameLevel level, const Score& score);
    static void normalizeScore(Score& score);
    static void normalizeAllTimeScore(AllTimeScore& score);
};
