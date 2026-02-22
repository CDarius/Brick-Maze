#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>

#include <PinsDefinitions.h>

#include <HardwareServo.hpp>
#include <M5UnitPbHub.hpp>
#include <Button.hpp>

#include <AudioPlayer.hpp>
#include <PuzzleDisplay.hpp>
#include <TextAnimation.hpp>
#include <ImageTransitionAnimation.hpp>
#include <MainDisplay.hpp>

#include <CancelToken.hpp>

HardwareServo xServo(X_SERVO_PIN, 0, -180, 180, 500, 2500);
HardwareServo yServo(Y_SERVO_PIN, 1, -180, 180, 500, 2500);

M5UnitPbHub pbHub(Wire);

AudioPlayer audioPlayer(1); // Use I2S port 1. Display uses I2S0 (ESP32) or LCD (ESP32-S3).

PuzzleDisplay display(PUZZLE_DISPLAY_PIXEL_PIN);
TextAnimation textAnimation(display);
ImageTransitionAnimation imageTransitionAnimation(display);
MainDisplay mainDisplay(audioPlayer, display, textAnimation, imageTransitionAnimation);

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

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    // Initialize serial comunication with the remote controller
    Serial1.begin (115200, SERIAL_8N1, REMOTE_CONTROLLER_UART_RX, REMOTE_CONTROLLER_UART_TX, false);
    Serial1.setTimeout(0);
    // Initialize the display
    display.begin(); 
    
    // Initialize I2C bus & devices
    Wire.begin(I2C_SDA, I2C_SCL);
    if (!pbHub.begin()) {
        showInitFailed("PB Hub Init Fail", "Failed to initialize M5 Unit PB Hub");
    }

    // Initialize X & Y servos
    if (!xServo.begin()) {
        showInitFailed("X Servo Init Fail", "Failed to initialize X servo pin with LedC peripheral");
    }

    if (!yServo.begin()) {
        showInitFailed("Y Servo Init Fail", "Failed to initialize Y servo pin with LedC peripheral");
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
    //audioPlayer.setVolume(2); // Set initial volume (0-21)

    // Create audio loop task on core 0 with high priority
    xTaskCreatePinnedToCore(
        [](void* param) {
            audioPlayer.audioLoop();
        },
        "AudioTask",        // Name
        4096,               // Stack size
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
        "MainDisplayTask", // Task name
        8192,              // Stack size
        nullptr,           // Parameter
        1,                 // Priority
        nullptr,           // Task handle
        0                  // Core 0
    );

    Serial.println("Initialization complete. Entering main loop.");
}


uint16_t counter = 0;
void loop() {
    Serial.println("Main looop running...");
    xServo.setPulseWidth(1000);
    delay(1000);
    xServo.setPulseWidth(1500);
    delay(1000);
    xServo.setPulseWidth(2000);
    delay(1000);
    /*
    mainDisplay.setCountdownMode(millis() + 10000, 10000, 5000); // Example: 10 second countdown with critical threshold at 3 seconds
    delay(10000);
    audioPlayer.play(AUDIO_FILE_GAME_WIN);
    delay(10000);
    
    delay(5000);
    */
}