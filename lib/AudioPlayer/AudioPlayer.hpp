#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <freertos/semphr.h>

#define AUDIO_MAX_VOLUME 21

#define AUDIO_FILE_GAME_OVER    "/game-over.wav"
#define AUDIO_FILE_WARNING_BEEP "/warning-beep.wav"

class AudioPlayer {
    private:
        Audio audio;
        String newFilenameToPlay;
        SemaphoreHandle_t _mutex;

    public:
        /**
         * @brief Construct a new Audio Player object.
         * 
         * @param i2s_port_number The I2S port number to use (0 or 1). Port 1 is recommended to avoid conflicts with other peripherals like RMT (for NeoPixels).
         */
        AudioPlayer(uint8_t i2s_port_number = 0) : audio(false, 0, i2s_port_number) {
            _mutex = xSemaphoreCreateMutex();
            newFilenameToPlay = "";
        }
        /**
         * Initializes the audio player with the specified I2S pin configuration. 
         * This should be called in the setup() function before attempting to play any audio.
         * @param bclk The GPIO pin number for the I2S bit clock (BCLK).
         * @param lrc The GPIO pin number for the I2S word select (LRC).
         * @param dout The GPIO pin number for the I2S data output (DOUT). 
         */
        void begin(int bclk, int lrc, int dout) {
            audio.setPinout(bclk, lrc, dout);
            audio.setVolume(AUDIO_MAX_VOLUME); // Set default volume to maximum
        }

        /**
         * Checks if audio is currently playing. Note that this will return false if the audio has finished playing, 
         * even if the file is still open. Use this in combination with the audio_info callback to detect when playback 
         * has actually stopped.
         */ 
        inline bool isPlaying() {
            return audio.isRunning();
        }    

        /**
         * Sets the playback volume. The volume level can typically range from 0 (mute) to 21 (maximum), 
         * with a default of 10.
         * @param volume The desired volume level (0-21). Values outside this range are clamped
         */
        void setVolume(uint8_t volume) { 
            volume = volume > 0 ? volume : 0; // Ensure volume is not negative
            audio.setVolume(volume); 
        }

        /**
         * Plays an audio file from the SPIFFS filesystem. 
         * The file must be in a supported format (e.g., WAV, MP3) and located in the SPIFFS directory.
         * @param filename The path to the audio file within SPIFFS (e.g., "/music/song.mp3"). The leading slash is required.
         * @return true if the file was successfully opened and playback started, false otherwise (e.g., file not found, unsupported format).
         */
        void play(String filename) {
            if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
                newFilenameToPlay = filename; // Set the pending filename to play. The audio loop will handle starting playback.
                xSemaphoreGive(_mutex);
            }
        }

        /**
         * This function should be called repeatedly in the loop() function to allow the audio library to process audio data and handle events.
         */
        void audioLoop() {
            while(true) {
                if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
                    if (newFilenameToPlay.length() > 0) {
                        // Start to play the next file
                        if (!audio.connecttoFS(SPIFFS, newFilenameToPlay.c_str())) {
                            Serial.println("Failed to play audio file: " + newFilenameToPlay);
                        }
                        newFilenameToPlay = ""; // Clear the pending filename after starting playback
                    }

                    audio.loop();
                    xSemaphoreGive(_mutex);
                }
                delay(1); // Yield to other tasks. The audio library needs to be called frequently.
            }

        }
};
