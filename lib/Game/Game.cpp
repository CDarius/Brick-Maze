#include "Game.hpp"

namespace {
    constexpr uint16_t kDefaultCenterPulseUs = 1500;
}

Game::Game(HardwareServo& xServo, HardwareServo& yServo) 
    : xServo(xServo), yServo(yServo) {
        xServoRamp = SlewRateLimiter<int16_t>(kDefaultCenterPulseUs, 200.0f); // Default max rate: 200us/sec
        yServoRamp = SlewRateLimiter<int16_t>(kDefaultCenterPulseUs, 200.0f); // Default max rate: 200us/sec
    }

void IRAM_ATTR Game::setBallDropped() {
    portENTER_CRITICAL_ISR(&ballDroppedMux);
    ballDropped = true;
    portEXIT_CRITICAL_ISR(&ballDroppedMux);
}

void Game::resetBallDroppedFlag() {
    portENTER_CRITICAL(&ballDroppedMux);
    ballDropped = false;
    portEXIT_CRITICAL(&ballDroppedMux);
}

bool Game::consumeBallDroppedFlag() {
    bool dropped;
    portENTER_CRITICAL(&ballDroppedMux);
    dropped = ballDropped;
    ballDropped = false;
    portEXIT_CRITICAL(&ballDroppedMux);
    return dropped;
}

void Game::begin(const GameConfig config) {
    this->config = config;
    xServoRamp.setMaxRate(config.maxServoPulseRate);
    yServoRamp.setMaxRate(config.maxServoPulseRate);

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

void Game::prepareGame() {
    if (!isReadyToStart()) {
        return;
    }

    status = GameStatus::PREPARING;
    // Move servos to center position before starting the game
    xServoRamp.setMaxRate(config.prepareGamePulseRate);
    yServoRamp.setMaxRate(config.prepareGamePulseRate);
    xServoRamp.setTarget(static_cast<int16_t>(xCenterPulseUs) + config.prepareGameXPulseus);
    yServoRamp.setTarget(static_cast<int16_t>(yCenterPulseUs) + config.prepareGameYPulseus);
}

void Game::start(GameLevel level) {
    if (status != GameStatus::PREPARING) {
        return;
    }

    // Move the servos quickly in the opposite direction of the prepareGame to kick the ball away from the holder
    xServoRamp.setMaxRate(config.maxServoPulseRate);
    yServoRamp.setMaxRate(config.maxServoPulseRate);
    xServoRamp.setTarget(static_cast<int16_t>(xCenterPulseUs) - config.prepareGameXPulseus);
    yServoRamp.setTarget(static_cast<int16_t>(yCenterPulseUs) - config.prepareGameYPulseus);
    delay(config.prepareKickBackDelayMs);
    
    // Let's the player control the servos after the kick back
    currentLevel = level;
    currentTimeLimitMs = getTimeLimitMs(level);
    startTimeMs = millis();
    status = GameStatus::RUNNING;
    resetBallDroppedFlag();
}

void Game::stop() {
    if (status != GameStatus::RUNNING && status != GameStatus::PREPARING) {
        return;
    }

    status = GameStatus::NOT_RUNNING;

    // Reset servos to center position
    xServoRamp.setTarget(static_cast<int16_t>(xCenterPulseUs));
    yServoRamp.setTarget(static_cast<int16_t>(yCenterPulseUs));

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
            xServoRamp.setTarget(static_cast<int16_t>(xCenterPulseUs));
            yServoRamp.setTarget(static_cast<int16_t>(yCenterPulseUs));
            lastGameResult = GameResult::LOST;
            lastGameCompletionTimeMs = currentTimeLimitMs;
            lastGameLevel = currentLevel;
            return;
        }

        // Check if the ball has been dropped
        if (consumeBallDroppedFlag()) {
            status = GameStatus::DROPPING_BALL;
            xServoRamp.setTarget(config.ballDropXDeltaPulseUs + static_cast<int16_t>(xCenterPulseUs));
            yServoRamp.setTarget(config.ballDropYDeltaPulseUs + static_cast<int16_t>(yCenterPulseUs));
            timer.load(config.ballDropTimeMs);
            lastGameResult = GameResult::WON;
            lastGameCompletionTimeMs = elapsedMs;
            lastGameLevel = currentLevel;
            return;
        }

        // Clamp controller inputs to [-1, 1] and map to servo pulse widths
        float clampedX = constrain(controllerX, -1.0f, 1.0f);
        float clampedY = constrain(controllerY, -1.0f, 1.0f);

        float halfRangeUs = static_cast<float>(config.servoPulseRange) * 0.5f;
        float targetPulseXUs = xCenterPulseUs + (clampedX * halfRangeUs);
        float targetPulseYUs = yCenterPulseUs + (clampedY * halfRangeUs);
        xServoRamp.setTarget(static_cast<int16_t>(targetPulseXUs));
        yServoRamp.setTarget(static_cast<int16_t>(targetPulseYUs));
    }
    if (status == GameStatus::DROPPING_BALL) {
        // During ball dropping controller is not used and wait for the ball to reach
        // the back collection box
        if (timer.isElapsed()) {
            // Ball has reached the back collection box, ready for next game
            status = GameStatus::NOT_RUNNING;
            xServoRamp.setTarget(static_cast<int16_t>(xCenterPulseUs));
            yServoRamp.setTarget(static_cast<int16_t>(yCenterPulseUs));
        }
    }

    // Update servo positions with slew rate limiting
    unsigned long deltaMs = nowMs - lastUpdateMs;
    lastUpdateMs = nowMs;
    xServoRamp.update(deltaMs);
    yServoRamp.update(deltaMs);

    // Apply updated servo positions to the hardware except when calibration is in progress
    if (!calibrationInProgress) {
        xServo.setPulseWidth(static_cast<uint16_t>(xServoRamp.getCurrentValue()));
        yServo.setPulseWidth(static_cast<uint16_t>(yServoRamp.getCurrentValue()));
    }
}

bool Game::isRunning() const {
    return status == GameStatus::RUNNING || status == GameStatus::PREPARING;
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
    }
    return config.easyTimeLimitMs;
}

void Game::servoCalibration(MPU6886& imu) {
    calibrationInProgress = true;
    Serial.println("Starting servo calibration...");

    // Move servos to center position
    xServo.setPulseWidth(kDefaultCenterPulseUs);
    yServo.setPulseWidth(kDefaultCenterPulseUs);
    delay(1000); // Wait for servos to stabilize

    // Read IMU data to determine current orientation
    float accelX, accelY, accelZ;
    imu.getAccel(&accelX, &accelY, &accelZ);

    int xCalibrationOk = 0;
    int yCalibrationOk = 0;

    while (true) {
        imu.getAccel(&accelX, &accelY, &accelZ);

        // Invert X and Y accelation to match servo directions
        accelX = -accelX;
        accelY = -accelY;

        // Calculate angles from accelerometer data
        float angleX_deg = asin(accelX) * 180.0 / M_PI;
        float angleY_deg = asin(accelY) * 180.0 / M_PI;

        // Apply a simple proportional control to move servos towards leveling the table
        float kP = 0.5f; // Proportional gain (adjust as needed)
        float errorX = config.servoCalibrationTargetAngleXDeg - angleX_deg; // Desired angle is targetAngleXDeg
        float errorY = config.servoCalibrationTargetAngleYDeg - angleY_deg; // Desired angle is targetAngleYDeg
        if (abs(errorX) < config.servoCalibrationErrorThresholdDeg) {
            xCalibrationOk++;
        } else {
            xCalibrationOk = 0;
            float controlX = kP * errorX;
            xServo.changeAngle(controlX);
        }
        if (abs(errorY) < config.servoCalibrationErrorThresholdDeg) {
            yCalibrationOk++;
        } else {
            yCalibrationOk = 0;
            float controlY = kP * errorY;
            yServo.changeAngle(controlY);
        }

        delay(200);

        if (xCalibrationOk >= 5 && yCalibrationOk >= 5) {
            // Consider calibration successful if both axes are within the error threshold for 5 consecutive readings
            break;
        }
    }

    // Save calibration data
    xCenterPulseUs = xServo.getLastPulseWitdth();
    yCenterPulseUs = yServo.getLastPulseWitdth();
    xServoRamp.reset(static_cast<int16_t>(xCenterPulseUs));
    yServoRamp.reset(static_cast<int16_t>(yCenterPulseUs));

    Serial.println("Servo calibration complete.");
    calibrationInProgress = false;
}