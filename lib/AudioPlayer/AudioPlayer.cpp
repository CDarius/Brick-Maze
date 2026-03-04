#include <AudioPlayer.hpp>

static float sinTable[256];
static bool sinTableInitialized = false;

void AudioPlayer::generateTone() {
    if (!sinTableInitialized) {
        for (int i = 0; i < 256; i++) {
            sinTable[i] = sinf(i * 2.0f * PI / 256.0f);
        }
        sinTableInitialized = true;
    }

    size_t bytes_written;
    int16_t samples[1024]; // 512 stereo samples (~11.6ms at 44.1kHz)
    
    // Map volume 0-21 to amplitude (approx 0-30000)
    int amplitude = (m_volume * 30000) / AUDIO_MAX_VOLUME;
    
    float phaseIncrement = 256.0f * m_toneFreq / 44100.0f;

    for (int i = 0; i < 512; i++) {
        int16_t sample = (int16_t)(sinTable[(int)m_tonePhase & 0xFF] * amplitude);
        samples[i * 2] = sample;
        samples[i * 2 + 1] = sample;
        
        m_tonePhase += phaseIncrement;
        while (m_tonePhase >= 256.0f) m_tonePhase -= 256.0f;
    }
    
    // Write to I2S with a small timeout to avoid blocking too long if buffer is full
    i2s_write(m_i2s_port, samples, sizeof(samples), &bytes_written, 10);
}

bool AudioPlayer::isPlaying() {
    bool currentlyRunning = false;
    bool isPlayingNow = false;

    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        currentlyRunning = audio.isRunning();
        if (currentlyRunning || m_tonePlaying) {
            isPlayingLatched = true;
            lastPlaybackActivityMs = millis();
        }

        isPlayingNow = hasPendingPlayback || isPlayingLatched || currentlyRunning || m_tonePlaying;
        xSemaphoreGive(_mutex);
    }

    return isPlayingNow;
}    

void AudioPlayer::setVolume(uint8_t volume) { 
    volume = volume > 0 ? volume : 0; // Ensure volume is not negative
    if (volume > AUDIO_MAX_VOLUME) volume = AUDIO_MAX_VOLUME;
    audio.setVolume(volume); 
    m_volume = volume;
}

void AudioPlayer::play(const char* filename) {
    if (filename == nullptr || strlen(filename) == 0) {
        return;
    }

    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        newFilenameToPlay = filename; // Set the pending filename to play. The audio loop will handle starting playback.
        hasPendingPlayback = true;
        isPlayingLatched = true;
        lastPlaybackActivityMs = millis();
        xSemaphoreGive(_mutex);
    }
}

void AudioPlayer::playTone(uint16_t frequency, uint32_t durationMs) {
    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        if (audio.isRunning()) {
            audio.stopSong();
        }
        newFilenameToPlay = ""; // Cancel any pending file
        hasPendingPlayback = false;
        
        if (!m_tonePlaying) m_tonePhase = 0.0f; // Reset phase if starting new tone sequence
        m_toneFreq = frequency;
        m_toneEndMs = millis() + durationMs;
        m_tonePlaying = true;
        isPlayingLatched = true;
        lastPlaybackActivityMs = millis();
        
        // Ensure sample rate is set for tone generation
        i2s_set_sample_rates(m_i2s_port, 44100);
        
        xSemaphoreGive(_mutex);
    }
}

void AudioPlayer::stop() {
    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        if (audio.isRunning()) {
            audio.stopSong();
        }
        newFilenameToPlay = ""; // Clear any pending filename
        hasPendingPlayback = false;
        m_tonePlaying = false; // Stop tone if playing
        isPlayingLatched = false;
        xSemaphoreGive(_mutex);
    }
}

void AudioPlayer::audioLoop() {
    while(true) {
        if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
            if (hasPendingPlayback && newFilenameToPlay.length() > 0) {
                m_tonePlaying = false; // Stop tone if new file requested
                String filenameToPlay = newFilenameToPlay;

                if (audio.isRunning()) {
                    audio.stopSong();
                    audio.loop();
                }

                // Start to play the next file
                if (!audio.connecttoFS(SPIFFS, filenameToPlay.c_str())) {
                    Serial.println("Failed to play audio file: " + filenameToPlay);
                    hasPendingPlayback = false;
                    isPlayingLatched = false;
                } else {
                    hasPendingPlayback = false;
                    isPlayingLatched = true;
                    lastPlaybackActivityMs = millis();
                }
                newFilenameToPlay = ""; // Clear the pending filename after starting playback
            }

            if (m_tonePlaying) {
                if (millis() >= m_toneEndMs) {
                    m_tonePlaying = false;
                    // Write silence to clear buffer
                    size_t bytes_written;
                    int16_t zero_samples[128] = {0};
                    i2s_write(m_i2s_port, zero_samples, sizeof(zero_samples), &bytes_written, 0);
                } else {
                    generateTone();
                }
            } else {
                audio.loop();
            }

            bool running = audio.isRunning();
            if (running || m_tonePlaying) {
                isPlayingLatched = true;
                lastPlaybackActivityMs = millis();
            } else if (isPlayingLatched && !hasPendingPlayback && (millis() - lastPlaybackActivityMs > 200)) {
                isPlayingLatched = false;
            }
            xSemaphoreGive(_mutex);
        }
        delay(5); // Yield to other tasks. The audio library needs to be called frequently.
    }

}
