#pragma once

enum class GameLevel {
    EASY
};

constexpr uint8_t GAME_LEVEL_COUNT = 1;

constexpr bool isValidGameLevel(GameLevel level) {
    return level == GameLevel::EASY;
}

constexpr uint8_t gameLevelToIndex(GameLevel level) {
    return level == GameLevel::EASY ? 0 : 0;
}

constexpr const char* gameLevelToString(GameLevel level) {
    return level == GameLevel::EASY ? "EASY" : "EASY";
}