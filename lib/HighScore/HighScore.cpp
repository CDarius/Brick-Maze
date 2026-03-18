#include "HighScore.hpp"

bool HighScore::begin(GameConfig config) {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        return false;
    }

    const size_t expectedSize = sizeof(scores);
    const size_t storedSize = preferences.getBytesLength(NVS_KEY);

    if (storedSize == expectedSize) {
        const size_t readSize = preferences.getBytes(NVS_KEY, scores, expectedSize);
        if (readSize == expectedSize) {
            for (uint8_t level = 0; level < GAME_LEVEL_COUNT; ++level) {
                for (uint8_t rank = 0; rank < SCORES_PER_LEVEL; ++rank) {
                    normalizeScore(scores[level][rank]);
                }
            }
            initialized = true;
            return true;
        }
    }

    loadHardcodedScores(config);
    initialized = persistAll();
    return initialized;
}

bool HighScore::read(GameLevel level, uint8_t rank, Score& outScore) const {
    if (!initialized || !isValidGameLevel(level) || rank >= SCORES_PER_LEVEL) {
        return false;
    }

    outScore = scores[gameLevelToIndex(level)][rank];
    return true;
}

int8_t HighScore::write(GameLevel level, const Score& score) {
    if (!initialized || !isValidGameLevel(level)) {
        return -1;
    }

    // Determine the rank for the new score. If it's not a high score, return -1 
    // and do not write.
    const int8_t rank = getHighScoreRank(level, score.timeMs);
    if (rank < 0) {
        return -1;
    }

    const uint8_t levelIndex = gameLevelToIndex(level);

    // Shift down lower scores to make room for the new score at the correct rank
    for (int8_t i = static_cast<int8_t>(SCORES_PER_LEVEL) - 1; i > rank; --i) {
        scores[levelIndex][i] = scores[levelIndex][i - 1];
    }

    // Insert the new score at the correct rank position
    Score normalized = score;
    normalizeScore(normalized);
    scores[levelIndex][rank] = normalized;

    // Persist the updated scores to NVS
    persistAll();

    return rank;
}

int8_t HighScore::getHighScoreRank(GameLevel level, uint16_t time) const {
    if (!initialized || !isValidGameLevel(level)) {
        return -1;
    }

    const uint8_t levelIndex = gameLevelToIndex(level);
    for (uint8_t rank = 0; rank < SCORES_PER_LEVEL; ++rank) {
        if (time < scores[levelIndex][rank].timeMs) {
            return static_cast<int8_t>(rank);
        }
    }

    return -1;
}

bool HighScore::overwriteWithDefaultScores(GameConfig config) {
    if (!initialized) {
        return false;
    }

    loadHardcodedScores(config);
    return persistAll();
}

void HighScore::loadHardcodedScores(GameConfig config) {
    Score defaultScores[GAME_LEVEL_COUNT][SCORES_PER_LEVEL] = {
        {
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
            {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs},
        },
    };

    memcpy(scores, defaultScores, sizeof(scores));
}

bool HighScore::persistAll() {
    const size_t written = preferences.putBytes(NVS_KEY, scores, sizeof(scores));
    return written == sizeof(scores);
}

void HighScore::normalizeScore(Score& score) {
    score.name[3] = '\0';

    for (uint8_t i = 0; i < 3; ++i) {
        const char c = score.name[i];
        if (c == '\0') {
            for (uint8_t j = i; j < 3; ++j) {
                score.name[j] = ' ';
            }
            break;
        }
    }
}
