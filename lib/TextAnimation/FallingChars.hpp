#pragma once

#include <PuzzleDisplay.hpp>
#include <AudioPlayer.hpp>
#include <CancelToken.hpp>

class FallingCharsAnimation {
    public:
        FallingCharsAnimation(PuzzleDisplay& display, AudioPlayer& audioPlayer) 
            : display(display), audioPlayer(audioPlayer) {}

        /**
         * Set the audio clip to play when characters bounce at the end of their falling animation. 
         * The audio file should be in SPIFFS and in a supported format (e.g., WAV, MP3).
         * @param audioFile The path to the audio file in SPIFFS.
         */
        void setBounceAudioClip(const char* audioFile) {
            bounceAudioFile = audioFile;
        }

        /**
         * Animates the given text with a "falling characters" effect, where each character appears to fall from the top of the display to its final position.
         * The animation will use the provided gradient colors to color the characters, creating a visually appealing effect. 
         * The animation will run for the specified duration and frame rate, and can be cancelled using the cancel token.
         * Characters fall quickly and then bounce slightly when they hit their final position.
         * @param x The x-coordinate where the text animation should start.
         * @param text The text to animate. It should fit within the display dimensions.
         * @param gradientColors The gradient colors to use for the text.
         * @param charDurationMs The duration of the animation in milliseconds for each character to fall into place.
         * @param cancelToken The cancel token to stop the animation.
         */
        void InAnimation(uint16_t x, const char* text, const RgbColor* gradientColors, uint16_t charDurationMs, CancelToken& cancelToken);

    private:
        PuzzleDisplay& display;
        AudioPlayer& audioPlayer;

        const char* bounceAudioFile = nullptr;

        /**
         * Animates a single character falling into place with a bounce effect. This is called for each character in the InAnimation method.
         * The character will fall from the top of the display to its final position, following a predefined set of vertical offsets to create a bounce effect.
         * The animation will use the provided gradient colors to color the character, creating a visually appealing effect.
         * @param x The x-coordinate where the character should be drawn.
         * @param c The character to animate.
         * @param charWidth The width of the character in pixels.
         * @param gradientColors The gradient colors to use for the character.
         * @param frameDelayMs The delay in milliseconds between each frame of the animation.
         * @param initialCanvas The initial canvas to restore the background during animation.
         * @param cancelToken The cancel token to stop the animation.
         */
        void InCharAnimation(uint16_t x, char c, uint16_t charWidth, const RgbColor* gradientColors, uint16_t frameDelayMs, RgbColor initialCanvas[TOTAL_LEDS], CancelToken& cancelToken);
};