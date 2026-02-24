#pragma once

#include <Arduino.h>
#include <HardwareServo.hpp>
#include <GameConfig.h>
#include <SlewRateLimiter.hpp>

enum class GameLevel {
    EASY,
    MEDIUM,
    HARD
};

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
        Game(HardwareServo& xServo, HardwareServo& yServo, uint8_t ballDropPin);

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
         * Updates the game state based on controller input and ball drop status. This should be called regularly
         * (e.g., in a loop or timer) to process game logic and update servo positions.
         * @param controllerX The X axis input from the controller, expected to be in the range [-1, 1].
         * @param controllerY The Y axis input from the controller, expected to be in the range [-1, 1].
         */
        void update(float controllerX, float controllerY);

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
        uint8_t ballDropPin;

        GameConfig config;
        SlewRateLimiter<uint16_t> xServoRamp;
        SlewRateLimiter<uint16_t> yServoRamp;

        // Current game state
        GameStatus status = GameStatus::NOT_RUNNING;
        GameLevel currentLevel = GameLevel::EASY;    
        uint16_t currentTimeLimitMs = 0;
        unsigned long startTimeMs = 0;
        unsigned long lastUpdateMs = 0;

        // Last game results
        GameResult lastGameResult = GameResult::NONE;
        uint16_t lastGameCompletionTimeMs = 0;
        GameLevel lastGameLevel = GameLevel::EASY;

        uint16_t getTimeLimitMs(GameLevel level) const;
};