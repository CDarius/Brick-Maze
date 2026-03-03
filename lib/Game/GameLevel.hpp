#pragma once

enum class GameLevel {
    EASY,
    MEDIUM,
    HARD
};

constexpr uint8_t GAME_LEVEL_COUNT = 3;

constexpr bool isValidGameLevel(GameLevel level) {
    return level == GameLevel::EASY || level == GameLevel::MEDIUM || level == GameLevel::HARD;
}

constexpr uint8_t gameLevelToIndex(GameLevel level) {
    return level == GameLevel::EASY ? 0 : (level == GameLevel::MEDIUM ? 1 : 2);
}