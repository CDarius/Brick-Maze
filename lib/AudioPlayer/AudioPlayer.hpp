#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include "Audio.h"
#include <freertos/semphr.h>
#include <driver/i2s.h>
#include <math.h>

#define AUDIO_MAX_VOLUME 21

#define AUDIO_FILE_GAME_WIN         "/game-win.wav"
#define AUDIO_FILE_GAME_OVER        "/game-over.wav"
#define AUDIO_FILE_WARNING_BEEP     "/warning-beep.wav"
#define AUDIO_FILE_BADING           "/bading.wav"
#define AUDIO_FILE_SYSTEM_READY     "/unit-ready.wav"
#define AUDIO_FILE_NEW_HIGHSCORE    "/new-high-score.wav"
#define AUDIO_FILE_LEGO_SNAP        "/lego-snap.wav"
#define AUDIO_FILE_BRICK_MAZE       "/brick-maze.wav"

class AudioPlayer {
    private:
        Audio audio;
        String newFilenameToPlay;
        SemaphoreHandle_t _mutex;
        bool hasPendingPlayback;
        bool isPlayingLatched;
        uint32_t lastPlaybackActivityMs;

        i2s_port_t m_i2s_port;
        bool m_tonePlaying;
        uint16_t m_toneFreq;
        uint32_t m_toneEndMs;
        float m_tonePhase;
        uint8_t m_volume;

        // Internal function to generate a tone. This is called repeatedly in the audio loop
        // while a tone is active.
        void generateTone();

    public:
        /**
         * @brief Construct a new Audio Player object.
         * 
         * @param i2s_port_number The I2S port number to use (0 or 1). Port 1 is recommended to avoid conflicts with other peripherals like RMT (for NeoPixels).
         */
        AudioPlayer(uint8_t i2s_port_number = 0) : audio(false, 0, i2s_port_number), m_i2s_port((i2s_port_t)i2s_port_number) {
            _mutex = xSemaphoreCreateMutex();
            newFilenameToPlay = "";
            hasPendingPlayback = false;
            isPlayingLatched = false;
            lastPlaybackActivityMs = 0;
            m_tonePlaying = false;
            m_tonePhase = 0.0f;
            m_volume = AUDIO_MAX_VOLUME;
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
            m_volume = AUDIO_MAX_VOLUME;
        }

        /**
         * Checks if audio is currently playing. Note that this will return false if the audio has finished playing, 
         * even if the file is still open. Use this in combination with the audio_info callback to detect when playback 
         * has actually stopped.
         */ 
        bool isPlaying();

        /**
         * Sets the playback volume. The volume level can typically range from 0 (mute) to 21 (maximum), 
         * with a default of 10.
         * @param volume The desired volume level (0-21). Values outside this range are clamped
         */
        void setVolume(uint8_t volume);

        /**
         * Plays an audio file from the SPIFFS filesystem. 
         * The file must be in a supported format (e.g., WAV, MP3) and located in the SPIFFS directory.
         * @param filename The path to the audio file within SPIFFS (e.g., "/music/song.mp3"). The leading slash is required.
         * @return true if the file was successfully opened and playback started, false otherwise (e.g., file not found, unsupported format).
         */
        void play(const char* filename);

        /**
         * Plays a tone with the specified frequency and duration.
         * This will stop any currently playing audio file.
         * @param frequency The frequency of the tone in Hz.
         * @param durationMs The duration of the tone in milliseconds.
         */
        void playTone(uint16_t frequency, uint32_t durationMs);

        /**
         * Stops any currently playing audio file or tone.
         */
        void stop();

        /**
         * This function should be called repeatedly in the loop() function to allow the audio library to process audio data and handle events.
         */
        void audioLoop();
};
