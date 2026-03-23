#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>

#include <PinsDefinitions.h>
#include <Config.hpp>

#include <HardwareServo.hpp>
#include <M5UnitPbHub.hpp>
#include <MPU6886.hpp>
#include <Button.hpp>

#include <Controller.hpp>
#include <Game.hpp>

#include <AudioPlayer.hpp>
#include <PuzzleDisplay.hpp>
#include <MainDisplay.hpp>
#include <HighScore.hpp>

#include <CancelToken.hpp>

HardwareServo xServo(X_SERVO_PIN, 0, -180, 180, 500, 2500, true);
HardwareServo yServo(Y_SERVO_PIN, 1, -180, 180, 500, 2500);

M5UnitPbHub pbHub(Wire);
MPU6886 imu(Wire);

SerialComm controllerSerialComm(Serial1);
Controller controller(controllerSerialComm);
Game game(xServo, yServo);

AudioPlayer audioPlayer(1); // Use I2S port 1. Display uses I2S0 (ESP32) or LCD (ESP32-S3).

PuzzleDisplay display(PUZZLE_DISPLAY_PIXEL_PIN);
HighScore highScore;
MainDisplay mainDisplay(audioPlayer, display, highScore);
GameLevel nextGameLevel = GameLevel::EASY;
bool waitingForGameToStart = false;

GameLevel selectNextGameLevel(GameLevel level) {
    switch (level) {
        case GameLevel::EASY:
            return GameLevel::EASY;
    }

    return GameLevel::EASY;
}

uint16_t getCriticalThresholdMs(GameLevel level) {
    switch (level) {
        case GameLevel::EASY:
            return 10000;
    }

    return 10000;
}

void showInitFailed(const char* displayMessage, const char* serialMessage) {
    display.clear();
    String displayMessageStr(displayMessage);
    displayMessageStr.toUpperCase();
    display.drawString(0,0, displayMessageStr, COLOR_RED, FONT_4x6);
    display.show();
    while (true)
    {
        Serial.println(serialMessage);
        delay(500);
    }
}

bool isStopButtonPressed() {
    return pbHub.digitalRead(0, 1) == LOW;
}

bool isStartButtonPressed() {
    return pbHub.digitalRead(0, 0) == LOW;
}

void IRAM_ATTR onBallDropInterrupt() {
    game.setBallDropped();
}

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    // Initialize serial comunication with the remote controller
    Serial1.begin (115200, SERIAL_8N1, REMOTE_CONTROLLER_UART_RX, REMOTE_CONTROLLER_UART_TX, false);
    Serial1.setTimeout(0);

    // Initialize the display
    display.begin(); 
    
    // Initialize IO pins
    pinMode(LED_BUILTIN, OUTPUT);
    // Initialize ball drop pin input
    pinMode(BALL_DROP_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BALL_DROP_PIN), onBallDropInterrupt, FALLING);

    // Initialize I2C bus & devices
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!pbHub.begin()) {
        showInitFailed("PB Hub Init Fail", "Failed to initialize M5 Unit PB Hub");
    }
    if (!imu.begin(MPU6886::AccelScale::RANGE_2G)) {
        showInitFailed("IMU Init Fail", "Failed to initialize MPU6886 IMU");
    }

    // Initialize X & Y servos
    if (!xServo.begin(0)) {
        showInitFailed("X Servo Init Fail", "Failed to initialize X servo pin with LedC peripheral");
    }

    if (!yServo.begin(0)) {
        showInitFailed("Y Servo Init Fail", "Failed to initialize Y servo pin with LedC peripheral");
    }

    // Initialize the high score manager
    if (!highScore.begin(getDefaultGameConfig())) {
        showInitFailed("HScore Fail", "Failed to initialize high score manager");
    }
    // Overwrite high scores with default values if both stop and start buttons are pressed during startup as a way 
    // to reset high scores without needing to reflash the device
    if (isStopButtonPressed() && isStartButtonPressed()) {
        highScore.overwriteWithDefaultScores(getDefaultGameConfig());
    }

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        showInitFailed("SPIFFS Mount Fail", "SPIFFS Mount Failed");
        return;
    }

    // List files in SPIFFS
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("\nFiles in SPIFFS:");
    while (file) {
        Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
        file = root.openNextFile();
    }

    // Initialize Audio library with I2S pins
    audioPlayer.begin(I2S_BCLK, I2S_LRC, I2S_DOUT);

    // Initialized the controller and wait for serial communication to be established with it
    if (!controller.begin(getDefaultControllerConfig())) {
        showInitFailed("Ctrl Init Fail", "Failed to initialize controller");
    }

    // Initialize the game with default configuration
    game.begin(getDefaultGameConfig());

    // Create audio loop task on core 0 with high priority
    xTaskCreatePinnedToCore(
        [](void* param) {
            audioPlayer.audioLoop();
        },
        "AudioTask",        // Name
        8192,               // Stack size
        nullptr,            // Parameter
        5,                  // Priority (higher than default loop, but not max)
        nullptr,            // Task handle
        0                   // Core 0
    );

    // Create a task to run the HMI update loop on core 0
    xTaskCreatePinnedToCore(
        [](void* param) {
            mainDisplay.updateLoop();
        },
        "MainDisplayTask",  // Task name
        12 * 1024,          // Stack size
        nullptr,            // Parameter
        1,                  // Priority
        nullptr,            // Task handle
        1                   // Core 1
    );

    // Create a task to run controller update loop on core 1 with high priority to ensure responsive control
    xTaskCreatePinnedToCore(
        [](void* param) {
            controller.update();
        },
        "ControllerTask",   // Task name
        4096,               // Stack size
        nullptr,            // Parameter
        2,                  // Priority
        nullptr,            // Task handle
        1                   // Core 1
    );

    // Create a task to run the game update loop on core 1
    xTaskCreatePinnedToCore(
        [](void* param) {
            while (true) {
                float x, y;
                bool buttonPressed;
                if (controller.getStatus(x, y, buttonPressed)) {
                    game.update(x, y);
                    mainDisplay.updateControllerStatus(x, y, buttonPressed);
                }
                else {
                    mainDisplay.updateControllerStatus(0, 0, false);
                }

                if (buttonPressed && waitingForGameToStart) {
                    // If the button is pressed and the game is ready to start means that someoune
                    // is playing with the controller without asking for it, so we can set display
                    // the DON'T TOUCH message to warn
                    mainDisplay.setDontTouchMode();
                }
                delay(10);
            }
        },
        "GameTask",         // Task name
        4096,               // Stack size
        nullptr,            // Parameter
        1,                  // Priority
        nullptr,            // Task handle
        1                   // Core 1
    );

    // Create a task that listens for game stop button
    xTaskCreatePinnedToCore(
        [](void* param) {
            while (true) {
                if (game.isRunning() && isStopButtonPressed()) {
                    game.stop();
                    mainDisplay.setNoGameMode();
                }
                delay(100);
            }
        },
        "StopButtonTask",   // Task name
        2048,               // Stack size
        nullptr,            // Parameter
        1,                  // Priority
        nullptr,            // Task handle
        1                   // Core 1
    );

    // Calibrate servos by leveling the table before starting the game
    mainDisplay.setTableLevelingMode();
    game.servoCalibration(imu);

    audioPlayer.play(AUDIO_FILE_SYSTEM_READY);
    delay(2000);
    mainDisplay.setNoGameMode(true);
    Serial.println("Initialization complete. Entering main loop.");
}

void beforeGame();
void startGame();
void gameEnd();

void loop() {
    waitingForGameToStart = true;
    beforeGame();
    waitingForGameToStart = false;
    startGame();
    Serial.println("Game started. Waiting for it to end...");

    // Wait for game to end
    while (game.isRunning()) {
        delay(100);
    }

    Serial.println("Game ended. Showing results...");
    gameEnd();

    // Wait for game ready to start again
    while (!game.isReadyToStart()) {
        delay(100);
    }
}

void displayNextGameLevel() {
    pbHub.setLEDBrightness(1, 127);

    uint32_t ledColor[3] = {0};

    if (nextGameLevel == GameLevel::EASY) {
        ledColor[2] = 0x00FF00; // Green for easy
    }

    for (int i = 0; i < 3; i++) {
        pbHub.setLEDColor(1, i, ledColor[i]);
    }
}

void beforeGame() {
    Button startButton;

    mainDisplay.setNoGameMode();
    controllerSerialComm.sendControllerHMIMode(SerialComm::ControllerHMIMode::NO_GAME);
    displayNextGameLevel();

    while (true) {
        startButton.setRawState(millis(), isStartButtonPressed());

        if (startButton.wasSingleClicked()) {
            if (isStopButtonPressed()) {
                nextGameLevel = selectNextGameLevel(nextGameLevel);
                displayNextGameLevel();
            } else {
                break;
            }
        }

        delay(10);
    }
}

void startGame() {
    game.prepareGame();
    mainDisplay.setReadySetGoMode();
    while (!mainDisplay.isModeDone()) {
        delay(50);
    }

    game.start(nextGameLevel);

    unsigned long gameEndTimeMs = game.currentGameEndTimeMs();
    uint16_t gameTimeLimitMs = game.currentGameTimeLimitMs();
    uint16_t criticalThresholdMs = getCriticalThresholdMs(nextGameLevel);
    mainDisplay.setCountdownMode(gameEndTimeMs, gameTimeLimitMs, criticalThresholdMs);
    controllerSerialComm.sendControllerHMIMode(SerialComm::ControllerHMIMode::IN_GAME);
}

void gameEnd() {
    GameLevel lastGameLevel;
    GameResult lastGameResult;
    uint16_t lastGameCompletionTimeMs;

    game.lastGameStats(lastGameLevel, lastGameResult, lastGameCompletionTimeMs);

    if (lastGameResult == GameResult::NONE) {
        // Game was stopped without a win or loss (e.g. by pressing stop button)
        // Nothing to show on display, just return to before game state
        return;
    }

    controllerSerialComm.sendControllerHMIMode(SerialComm::ControllerHMIMode::END_GAME);

    // Update the display to reflect the game result
    if (lastGameResult == GameResult::WON) {
        mainDisplay.setGameWinMode();
    } else if (lastGameResult == GameResult::LOST) {
        mainDisplay.setGameOverMode();
    }

    // Wait for the display to finish its animation before proceeding
    while (!mainDisplay.isModeDone()) {        
        delay(100);
    }

    if (lastGameResult == GameResult::WON) {
        // Check if the completion time is a new high score and update if so
        int8_t rank = highScore.getHighScoreRank(lastGameLevel, lastGameCompletionTimeMs);
        if (rank >= 0) {
            // Enter player name and show high score celebration animation on the display
            // Press the stop button to cancel the high score submission.
            controllerSerialComm.sendControllerHMIMode(SerialComm::ControllerHMIMode::WRITE_PLAYER_NAME);
            mainDisplay.setEndGameHighScoreMode(lastGameCompletionTimeMs, lastGameLevel, rank);
            bool highScoreCancel = false;
            while (!mainDisplay.isModeDone() && !highScoreCancel) {
                highScoreCancel = isStopButtonPressed();
                delay(100);
            }

            if (!highScoreCancel) {
                // Save the new high score
                const String playerName = mainDisplay.getEndGamePlayerName();
                HighScore::Score newScore = {{' ', ' ', ' ', '\0'}, lastGameCompletionTimeMs};
                for (uint8_t i = 0; i < 3 && i < playerName.length(); ++i) {
                    newScore.name[i] = playerName[i];
                }
                highScore.write(lastGameLevel, newScore);

                // Small delay to show the new high score on the high score table before returning to idle state
                delay(2000);
            }
        } else {
            // If not an high score just show the completion time without entering a name
            mainDisplay.setEndGameTimeMode(lastGameCompletionTimeMs);
            while (!mainDisplay.isModeDone()) {        
                delay(100);
            }

            // Small pause to let the player see their completion time before returning to idle state
            controllerSerialComm.sendControllerHMIMode(SerialComm::ControllerHMIMode::END_GAME);
            delay(2000);
        }
    } else if (lastGameResult == GameResult::LOST) {
        // Delay a couple of seconds to keep the game over screen visible before returning to idle state
        delay(2000);
    }
}

