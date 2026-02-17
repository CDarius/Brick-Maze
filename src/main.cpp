#include <Arduino.h>
#include <SPIFFS.h>

#include <PinsDefinitions.h>

#include <AudioPlayer.hpp>
#include <PuzzleDisplay.hpp>
#include <TextAnimation.hpp>
#include <ImageTransitionAnimation.hpp>
#include <MainDisplay.hpp>

#include <CancelToken.hpp>

AudioPlayer audioPlayer(1); // Use I2S port 1. Display uses I2S0 (ESP32) or LCD (ESP32-S3).

PuzzleDisplay display(PUZZLE_DISPLAY_PIXEL_PIN);
TextAnimation textAnimation(display);
ImageTransitionAnimation imageTransitionAnimation(display);
MainDisplay mainDisplay(audioPlayer, display, textAnimation, imageTransitionAnimation);

void setup() {
    Serial.begin(115200);

    // Initialize the display
    display.begin();

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    Serial.println("SPIFFS mounted successfully");

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

    // Create audio loop task on core 1 with high priority
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

    // Create a task to run mainDisplay.updateLoop() on core 1
    xTaskCreatePinnedToCore(
        [](void* param) {
            mainDisplay.updateLoop();
        },
        "MainDisplayTask", // Task name
        8192,              // Stack size
        nullptr,           // Parameter
        1,                 // Priority
        nullptr,           // Task handle
        1                  // Core 1
    );
}


void loop() {
    unsigned long startTime = millis();
    unsigned long endTime = startTime + 30000; // Run for 30 seconds
    unsigned long currentTime = startTime;
    
    uint16_t textWidth = display.getStringWidth("99.9", FONT_6x8, true); // Pre-calculate width for centering
    uint8_t xPos = (display.getWidth() - textWidth) / 2; // Center the text



    Serial.println("In loop");
    //mainDisplay.setNoGameMode();
    //delay(10000);
    mainDisplay.setCountdownMode(millis() + 6000, 6000, 5000); // Set a countdown of 6 seconds
    delay(7000); // Wait a moment before starting the countdown
    mainDisplay.setGameOverMode();
    delay(3000);

    /*
    RgbColor gradientColors[8];
    display.mirroredColorGradient(RgbColor(255, 0, 0), RgbColor(255, 255, 0), gradientColors, 8);
    
    while (currentTime < endTime) {
        unsigned long remainingTime = endTime - currentTime;
        float remainingSeconds = remainingTime / 1000.0;
        // Display the countdown timer
        display.fill(RgbColor(0, 127, 127)); // Clear the canvas azure
        String timerText = String(remainingSeconds, 1); // Show 1 decimal place
        if (timerText.length() < 4) {
            timerText = " " + timerText; // Pad with space for alignment
        }
        display.drawString(xPos, 0, timerText, gradientColors, FONT_6x8, true); // Yellow text
        display.show();

        delay(50); // Update every 50ms
        currentTime = millis();
    }
    */
}