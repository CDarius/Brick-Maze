#pragma once

#include <Arduino.h>
#include <GameLevel.hpp>
#include <SoftTimer.hpp>
#include <HardwareServo.hpp>
#include <GameConfig.h>
#include <SlewRateLimiter.hpp>
#include <MPU6886.hpp>

enum class GameResult {
    NONE,
    WON,
    LOST
};

enum class GameStatus {
    NOT_RUNNING,
    RUNNING,
    DROPPING_BALL
};

/**
 * Game class manages the state and logic of the game. It handles starting, stopping, and updating the game
 * based on controller input and ball drop status. The update method should be called regularly 
 * (e.g., in a loop or timer) to process game logic and update servo positions.
 */
class Game {
    public:
        /**
         * Constructor for the Game class.
         * @param xServo Reference to the HardwareServo controlling the X axis.
         * @param yServo Reference to the HardwareServo controlling the Y axis.
         * @param ballDropPin GPIO pin number where the ball drop sensor is connected.
         */
        Game(HardwareServo& xServo, HardwareServo& yServo);

        /**
         * Initializes the game with the provided configuration.
         * @param config The configuration parameters for the game.
         */
        void begin(const GameConfig config);

        /**
         * Starts the game at the specified difficulty level. The game will run until the time limit
         * is reached or the ball reaches the goal.
         * @param level The difficulty level to start the game at.
         */
        void start(GameLevel level);

        /**
         * Stops a running game and resets the state. The servos will return to the center position.
         */
        void stop();

        /**
         * Sets the ball drop status to true, indicating that the ball has been dropped. This should be called
         * when the ball drop sensor detects that the ball has been dropped.
         */
        void IRAM_ATTR setBallDropped();

        /**
         * Updates the game state based on controller input and ball drop status. This should be called regularly
         * (e.g., in a loop or timer) to process game logic and update servo positions.
         * @param controllerX The X axis input from the controller, expected to be in the range [-1, 1].
         * @param controllerY The Y axis input from the controller, expected to be in the range [-1, 1].
         */
        void update(float controllerX, float controllerY);

        /** 
         * Performs servo calibration by moving the servos to level the game table, using the IMU 
         * to measure the current orientation. This can help ensure that the game starts with the table level.
         * @param imu Reference to the MPU6886 IMU used for measuring orientation during calibration.
        */
        void servoCalibration(MPU6886& imu);

        /**
         * Checks if the game is currently running.
         * When status is DROPPING_BALL, isRunning() will return false.
         * @return true if the game is running, false otherwise.
         */
        bool isRunning() const;

        /**
         * Checks if the game is ready to start.
         * The game is ready to start when it is not currently running or dropping the ball.
         * @return true if the game is ready to start, false otherwise.
         */
        bool isReadyToStart() const;

        /**
         * Gets the results of the last completed game, including the level, result (win/loss), and completion time.
         * @param level Last game level.
         * @param result Last game result.
         * @param completionTime Last game completion time in milliseconds.
         */
        void lastGameStats(GameLevel& level, GameResult& result, uint16_t& completionTime) const;

        /**
         * Gets end time for the current game. If the goal is not reached within the time limit, the game will end 
         * and the player will lose.
         * @return The end time for the current game in milliseconds since epoch. If the game is not running, returns 0.
         */
        unsigned long currentGameEndTimeMs() const {
            if (status != GameStatus::RUNNING) {
                return 0;
            }
            return startTimeMs + currentTimeLimitMs;
        }

        /**
         * Gets the time span in which the current game must be completed. If the goal is not reached 
         * within this time span, the game will end and the player will lose.
         * @return The time span for the current game in milliseconds. If the game is not running, returns 0.
         */
        uint16_t currentGameTimeLimitMs() const {
            if (status != GameStatus::RUNNING) {
                return 0;
            }
            return currentTimeLimitMs;
        }

    private:
        HardwareServo& xServo;
        HardwareServo& yServo;

        GameConfig config;
        SlewRateLimiter<int16_t> xServoRamp;
        SlewRateLimiter<int16_t> yServoRamp;
        SoftTimer timer;

        // Current game state
        GameStatus status = GameStatus::NOT_RUNNING;
        GameLevel currentLevel = GameLevel::EASY;    
        uint16_t currentTimeLimitMs = 0;
        unsigned long startTimeMs = 0;
        unsigned long lastUpdateMs = 0;
        volatile bool ballDropped = false;
        portMUX_TYPE ballDroppedMux = portMUX_INITIALIZER_UNLOCKED;

        // Last game results
        GameResult lastGameResult = GameResult::NONE;
        uint16_t lastGameCompletionTimeMs = 0;
        GameLevel lastGameLevel = GameLevel::EASY;

        void resetBallDroppedFlag();
        bool consumeBallDroppedFlag();
        uint16_t getTimeLimitMs(GameLevel level) const;

        // Calibration
        bool calibrationInProgress = false;
        uint16_t xCenterPulseUs = 1500;
        uint16_t yCenterPulseUs = 1500;
};