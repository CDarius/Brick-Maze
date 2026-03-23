#include "HighScore.hpp"

bool HighScore::begin(GameConfig config) {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        return false;
    }

    // Load per-level scores
    const size_t expectedTodaySize = sizeof(scores);
    if (preferences.getBytesLength(NVS_KEY_TODAY) == expectedTodaySize &&
        preferences.getBytes(NVS_KEY_TODAY, scores, expectedTodaySize) == expectedTodaySize) {
        for (uint8_t level = 0; level < GAME_LEVEL_COUNT; ++level) {
            for (uint8_t rank = 0; rank < SCORES_PER_LEVEL; ++rank) {
                normalizeScore(scores[level][rank]);
            }
        }
    } else {
        loadHardcodedScores(config);
        if (!persistLevelScores()) return false;
    }

    // Load all-time scores
    const size_t expectedAllTimeSize = sizeof(allTimeScores);
    if (preferences.getBytesLength(NVS_KEY_ALLTIME) == expectedAllTimeSize &&
        preferences.getBytes(NVS_KEY_ALLTIME, allTimeScores, expectedAllTimeSize) == expectedAllTimeSize) {
        for (uint8_t rank = 0; rank < SCORES_PER_LEVEL; ++rank) {
            normalizeAllTimeScore(allTimeScores[rank]);
        }
    } else {
        loadDefaultAllTimeScores();
        if (!persistAllTimeScores()) return false;
    }

    initialized = true;
    return true;
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

    // A score that doesn't make the today list cannot make the all-time list
    const int8_t rank = updateTodayScores(level, score);
    if (rank < 0) {
        return -1;
    }

    updateAllTimeScores(level, score);
    persistAll();

    return rank;
}

int8_t HighScore::updateTodayScores(GameLevel level, const Score& score) {
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

    return rank;
}

void HighScore::updateAllTimeScores(GameLevel level, const Score& score) {
    // Get the rank the score would achieve on the all-time list (if it were to be added)
    int8_t allTimeRank = -1;
    for (uint8_t r = 0; r < SCORES_PER_LEVEL; ++r) {
        if (score.timeMs < allTimeScores[r].timeMs) {
            allTimeRank = static_cast<int8_t>(r);
            break;
        }
    }

    // If the score doesn't make the all-time list, we can return early without modifying the list
    if (allTimeRank < 0) {
        return;
    }

    // Shift down lower scores to make room for the new score at the correct all-time rank
    for (int8_t i = static_cast<int8_t>(SCORES_PER_LEVEL) - 1; i > allTimeRank; --i) {
        allTimeScores[i] = allTimeScores[i - 1];
    }

    // Prepare the new all-time score entry with the player's name, time, and level
    AllTimeScore allTimeScore;
    memcpy(allTimeScore.name, score.name, sizeof(allTimeScore.name));
    allTimeScore.timeMs = score.timeMs;
    allTimeScore.level = level;
    normalizeAllTimeScore(allTimeScore);

    // Insert the new score at the correct all-time rank position
    allTimeScores[allTimeRank] = allTimeScore;
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

bool HighScore::readAllTime(uint8_t rank, AllTimeScore& outScore) const {
    if (!initialized || rank >= SCORES_PER_LEVEL) {
        return false;
    }

    outScore = allTimeScores[rank];
    return true;
}

bool HighScore::overwriteWithDefaultScores(GameConfig config) {
    if (!initialized) {
        return false;
    }

    // Only reset per-level scores; all-time scores are preserved
    loadHardcodedScores(config);
    return persistLevelScores();
}

void HighScore::loadDefaultAllTimeScores() {
    for (uint8_t i = 0; i < SCORES_PER_LEVEL; ++i) {
        allTimeScores[i] = {{'B', 'M', 'Z', '\0'}, UINT16_MAX, GameLevel::EASY};
    }
}

void HighScore::loadHardcodedScores(GameConfig config) {    
    for (uint8_t level = 0; level < GAME_LEVEL_COUNT; ++level) {
        for (uint8_t rank = 0; rank < SCORES_PER_LEVEL; ++rank) {
            scores[level][rank] = {{'B', 'M', 'Z', '\0'}, config.easyTimeLimitMs};
        }
    }
}

bool HighScore::persistLevelScores() {
    const size_t written = preferences.putBytes(NVS_KEY_TODAY, scores, sizeof(scores));
    return written == sizeof(scores);
}

bool HighScore::persistAllTimeScores() {
    const size_t written = preferences.putBytes(NVS_KEY_ALLTIME, allTimeScores, sizeof(allTimeScores));
    return written == sizeof(allTimeScores);
}

bool HighScore::persistAll() {
    return persistLevelScores() && persistAllTimeScores();
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

void HighScore::normalizeAllTimeScore(AllTimeScore& score) {
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
