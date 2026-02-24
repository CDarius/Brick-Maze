#include "Game.hpp"

namespace {
    constexpr uint16_t kCenterPulseUs = 1500;
}

Game::Game(HardwareServo& xServo, HardwareServo& yServo, uint8_t ballDropPin) 
    : xServo(xServo), yServo(yServo), ballDropPin(ballDropPin) {
        xServoRamp = SlewRateLimiter<uint16_t>(kCenterPulseUs, 200.0f); // Default max rate: 200us/sec
        yServoRamp = SlewRateLimiter<uint16_t>(kCenterPulseUs, 200.0f); // Default max rate: 200us/sec
    }

void Game::begin(const GameConfig config) {
    this->config = config;
    xServoRamp = SlewRateLimiter<uint16_t>(kCenterPulseUs, config.maxServoPulseRate); // Set max rate from config
    yServoRamp = SlewRateLimiter<uint16_t>(kCenterPulseUs, config.maxServoPulseRate); // Set max rate from config

    // Initialize ball drop pin input
    pinMode(ballDropPin, INPUT_PULLUP);

    // Initialize game state
    status = GameStatus::NOT_RUNNING;
    currentTimeLimitMs = 0;
    startTimeMs = 0;
    lastUpdateMs = millis();

    // Initialize last game results
    lastGameResult = GameResult::NONE;
    lastGameCompletionTimeMs = 0;
    lastGameLevel = GameLevel::EASY;
}

void Game::start(GameLevel level) {
    if (!isReadyToStart()) {
        return;
    }

    currentLevel = level;
    currentTimeLimitMs = getTimeLimitMs(level);
    startTimeMs = millis();
    status = GameStatus::RUNNING;

    xServoRamp.setTarget(kCenterPulseUs);
    yServoRamp.setTarget(kCenterPulseUs);
}

void Game::stop() {
    if (status != GameStatus::RUNNING) {
        return;
    }

    status = GameStatus::NOT_RUNNING;

    // Reset servos to center position
    xServoRamp.setTarget(kCenterPulseUs);
    yServoRamp.setTarget(kCenterPulseUs);

    // Clear last game results
    lastGameResult = GameResult::NONE;
    lastGameCompletionTimeMs = 0;
}

void Game::update(float controllerX, float controllerY) {
    unsigned long nowMs = millis();
    if (status == GameStatus::RUNNING) {
        // Check if the game time limit has been exceeded
        unsigned long elapsedMs = nowMs - startTimeMs;
        if (currentTimeLimitMs > 0 && elapsedMs >= currentTimeLimitMs) {
            status = GameStatus::NOT_RUNNING;
            xServoRamp.setTarget(kCenterPulseUs);
            yServoRamp.setTarget(kCenterPulseUs);
            lastGameResult = GameResult::LOST;
            lastGameCompletionTimeMs = currentTimeLimitMs;
            lastGameLevel = currentLevel;
            return;
        }

        // Check if the ball has been dropped
        if (digitalRead(ballDropPin) == LOW) {
            status == GameStatus::DROPPING_BALL;
            xServoRamp.setTarget(config.ballDropXPulseUs);
            yServoRamp.setTarget(config.ballDropYPulseUs);
            lastGameResult = GameResult::WON;
            lastGameCompletionTimeMs = elapsedMs;
            lastGameLevel = currentLevel;
            return;
        }

        // Clamp controller inputs to [-1, 1] and map to servo pulse widths
        float clampedX = constrain(controllerX, -1.0f, 1.0f);
        float clampedY = constrain(controllerY, -1.0f, 1.0f);

        float halfRangeUs = static_cast<float>(config.servoPulseRange) * 0.5f;
        float targetPulseXUs = kCenterPulseUs + (clampedX * halfRangeUs);
        float targetPulseYUs = kCenterPulseUs + (clampedY * halfRangeUs);
    }

    // Update servo positions with slew rate limiting
    unsigned long deltaMs = nowMs - lastUpdateMs;
    lastUpdateMs = nowMs;
    xServoRamp.update(deltaMs);
    yServoRamp.update(deltaMs);

    xServo.setPulseWidth(xServoRamp.getCurrentValue());
    yServo.setPulseWidth(yServoRamp.getCurrentValue());
}

bool Game::isRunning() const {
    return status == GameStatus::RUNNING;
}

bool Game::isReadyToStart() const {
    return status == GameStatus::NOT_RUNNING;
}

void Game::lastGameStats(GameLevel& level, GameResult& result, uint16_t& completionTime) const {
    level = lastGameLevel;
    result = lastGameResult;
    completionTime = lastGameCompletionTimeMs;
}

uint16_t Game::getTimeLimitMs(GameLevel level) const {
    switch (level) {
        case GameLevel::EASY:
            return config.easyTimeLimitMs;
        case GameLevel::MEDIUM:
            return config.mediumTimeLimitMs;
        case GameLevel::HARD:
            return config.hardTimeLimitMs;
    }
    return config.easyTimeLimitMs;
}
