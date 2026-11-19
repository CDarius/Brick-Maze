#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include <GameConfig.h>

class HighScore {
public:
    static constexpr uint8_t SCORE_COUNT = 9;

    struct Score {
        char name[4];
        uint16_t timeMs;
    };

    bool begin(GameConfig config);

    bool read(uint8_t rank, Score& outScore) const;
    int8_t write(const Score& score);
    int8_t getHighScoreRank(uint16_t time) const;

    bool readAllTime(uint8_t rank, Score& outScore) const;

    bool overwriteWithDefaultScores(GameConfig config);

private:
    static constexpr const char* NVS_NAMESPACE = "brickmaze";
    static constexpr const char* NVS_KEY_TODAY = "today";
    static constexpr const char* NVS_KEY_ALLTIME = "alltime";

    Preferences preferences;
    Score scores[SCORE_COUNT];
    Score allTimeScores[SCORE_COUNT];
    bool initialized = false;

    void loadDefaultScores(GameConfig config);
    void loadDefaultAllTimeScores(GameConfig config);
    bool persistTodayScores();
    bool persistAllTimeScores();
    bool persistAll();
    int8_t updateTodayScores(const Score& score);
    void updateAllTimeScores(const Score& score);
    static void normalizeScore(Score& score);
};
