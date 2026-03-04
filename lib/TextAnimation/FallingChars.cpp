#include <FallingChars.hpp>

constexpr int16_t bounceOffsets[] = {-8, -5, -2, 0, -1, 0};
constexpr int16_t numFrames = 6;

void FallingCharsAnimation::InCharAnimation(uint16_t x, char c, uint16_t charWidth, const RgbColor* gradientColors, uint16_t frameDelayMs, RgbColor initialCanvas[TOTAL_LEDS], CancelToken& cancelToken) {
    for (uint16_t frame = 0; frame < numFrames; frame++) {
        IF_CANCELLED(cancelToken, return;)

        // Restore the initial canvas to clear previous character position
        display.copyCanvasFrom(initialCanvas, x, 0, charWidth, display.getHeight(), x, 0);

        // Draw the character at its current falling position based on the bounce offsets
        int16_t yOffset = bounceOffsets[frame];
        display.drawChar(x, yOffset, c, gradientColors, FONT_6x8);

        // Play bounce sound when hit ground
        if (yOffset == 0 && bounceAudioFile != nullptr) {
            audioPlayer.play(bounceAudioFile);
        }

        display.show();
        delay(frameDelayMs);
    }    
}

void FallingCharsAnimation::InAnimation(uint16_t x, const char* text, const RgbColor* gradientColors, uint16_t charDurationMs, CancelToken& cancelToken) {    
    uint16_t dh = display.getHeight();

    // Get text extents to determine the final Y position of the characters
    uint16_t textWidth = display.getStringWidth(text, FONT_6x8);

    // Save the starting canvas to restore the background during animation
    RgbColor startCanvas[TOTAL_LEDS];
    display.copyCanvasTo(startCanvas);

    // Calculate animation frames and delay for each character
    uint16_t charDelayMs = charDurationMs / numFrames;

    uint16_t textLen = strlen(text);
    for (uint16_t i = 0; i < textLen; i++) {
        IF_CANCELLED(cancelToken, return;)        
        char c = text[i];
        uint16_t charW = display.getStringWidth(String(c), FONT_6x8);
        InCharAnimation(x, c, charW, gradientColors, charDelayMs, startCanvas, cancelToken);
        x += charW + 1; // Move x position for the next character
    }
}