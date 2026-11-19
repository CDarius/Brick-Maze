#include "HighScore.hpp"

bool HighScore::begin(GameConfig config) {
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        return false;
    }

    // Load today scores. If missing or with an unexpected size, reset them to defaults.
    const size_t expectedTodaySize = sizeof(scores);
    if (preferences.getBytesLength(NVS_KEY_TODAY) == expectedTodaySize &&
        preferences.getBytes(NVS_KEY_TODAY, scores, expectedTodaySize) == expectedTodaySize) {
        for (uint8_t rank = 0; rank < SCORE_COUNT; ++rank) {
            normalizeScore(scores[rank]);
        }
    } else {
        loadDefaultScores(config);
        if (!persistTodayScores()) return false;
    }

    // Load all-time scores. If missing or with an unexpected size (e.g. saved by older firmware with a
    // different layout), reset them to defaults.
    const size_t expectedAllTimeSize = sizeof(allTimeScores);
    if (preferences.getBytesLength(NVS_KEY_ALLTIME) == expectedAllTimeSize &&
        preferences.getBytes(NVS_KEY_ALLTIME, allTimeScores, expectedAllTimeSize) == expectedAllTimeSize) {
        for (uint8_t rank = 0; rank < SCORE_COUNT; ++rank) {
            normalizeScore(allTimeScores[rank]);
        }
    } else {
        loadDefaultAllTimeScores(config);
        if (!persistAllTimeScores()) return false;
    }

    initialized = true;
    return true;
}

bool HighScore::read(uint8_t rank, Score& outScore) const {
    if (!initialized || rank >= SCORE_COUNT) {
        return false;
    }

    outScore = scores[rank];
    return true;
}

int8_t HighScore::write(const Score& score) {
    if (!initialized) {
        return -1;
    }

    // A score that doesn't make the today list cannot make the all-time list
    const int8_t rank = updateTodayScores(score);
    if (rank < 0) {
        return -1;
    }

    updateAllTimeScores(score);
    persistAll();

    return rank;
}

int8_t HighScore::updateTodayScores(const Score& score) {
    const int8_t rank = getHighScoreRank(score.timeMs);
    if (rank < 0) {
        return -1;
    }

    // Shift down lower scores to make room for the new score at the correct rank
    for (int8_t i = static_cast<int8_t>(SCORE_COUNT) - 1; i > rank; --i) {
        scores[i] = scores[i - 1];
    }

    // Insert the new score at the correct rank position
    Score normalized = score;
    normalizeScore(normalized);
    scores[rank] = normalized;

    return rank;
}

void HighScore::updateAllTimeScores(const Score& score) {
    // Get the rank the score would achieve on the all-time list (if it were to be added)
    int8_t allTimeRank = -1;
    for (uint8_t r = 0; r < SCORE_COUNT; ++r) {
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
    for (int8_t i = static_cast<int8_t>(SCORE_COUNT) - 1; i > allTimeRank; --i) {
        allTimeScores[i] = allTimeScores[i - 1];
    }

    // Insert the new score at the correct all-time rank position
    Score normalized = score;
    normalizeScore(normalized);
    allTimeScores[allTimeRank] = normalized;
}

int8_t HighScore::getHighScoreRank(uint16_t time) const {
    if (!initialized) {
        return -1;
    }

    for (uint8_t rank = 0; rank < SCORE_COUNT; ++rank) {
        if (time < scores[rank].timeMs) {
            return static_cast<int8_t>(rank);
        }
    }

    return -1;
}

bool HighScore::readAllTime(uint8_t rank, Score& outScore) const {
    if (!initialized || rank >= SCORE_COUNT) {
        return false;
    }

    outScore = allTimeScores[rank];
    return true;
}

bool HighScore::overwriteWithDefaultScores(GameConfig config) {
    if (!initialized) {
        return false;
    }

    // Only reset today scores; all-time scores are preserved
    loadDefaultScores(config);
    return persistTodayScores();
}

void HighScore::loadDefaultAllTimeScores(GameConfig config) {
    for (uint8_t i = 0; i < SCORE_COUNT; ++i) {
        allTimeScores[i] = {{'B', 'M', 'Z', '\0'}, config.timeLimitMs};
    }
}

void HighScore::loadDefaultScores(GameConfig config) {
    for (uint8_t rank = 0; rank < SCORE_COUNT; ++rank) {
        scores[rank] = {{'B', 'M', 'Z', '\0'}, config.timeLimitMs};
    }
}

bool HighScore::persistTodayScores() {
    const size_t written = preferences.putBytes(NVS_KEY_TODAY, scores, sizeof(scores));
    return written == sizeof(scores);
}

bool HighScore::persistAllTimeScores() {
    const size_t written = preferences.putBytes(NVS_KEY_ALLTIME, allTimeScores, sizeof(allTimeScores));
    return written == sizeof(allTimeScores);
}

bool HighScore::persistAll() {
    return persistTodayScores() && persistAllTimeScores();
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
