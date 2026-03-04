#include "DemolitionCharsAnimation.hpp"

void DemolitionCharsAnimation::addText(int16_t x, const char* text, RgbColor gradientColors[ANIM_TEXT_FONT_HEIGHT]) {
    for (char* p = (char*)text; *p != '\0'; p++) {
        FallingLetter newChar;
        newChar.character = *p;
        newChar.x = x;
        newChar.y = 0;
        newChar.velocity = 0;
        newChar.fallStarted = false;
        memcpy(newChar.gradient, gradientColors, sizeof(RgbColor) * ANIM_TEXT_FONT_HEIGHT);
        
        chars.push_back(newChar);

        // Move x position for the next character (assuming 6 pixels width per character)
        uint8_t charWidth = display.getStringWidth(String(*p), ANIM_TEXT_FONT, false);
        x += charWidth + 1; // Character width + 1 pixel spacing
    }
}

void DemolitionCharsAnimation::run(CancelToken& cancelToken) {
    uint16_t dh = display.getHeight();
    uint16_t len = chars.size();
    float gravity = 0.25; // Constant acceleration for the falling effect

    bool allFallen = false;
    while (!allFallen) {
        IF_CANCELLED(cancelToken, return;)

        allFallen = true;

        display.clear();

        for (uint16_t i = 0; i < len; i++) {
            FallingLetter& letter = chars[i];
            if (letter.y < dh) { // Se la lettera è ancora visibile
                allFallen = false;

                // Randomly decide when to start the fall for each letter to create a staggered effect
                if (!letter.fallStarted && random(0, 100) < 15) {
                    letter.fallStarted = true;
                }

                // Physics of the fall
                if (letter.fallStarted) {
                    letter.velocity += gravity; // Accelerate
                    letter.y += letter.velocity; // Move
                }

                // Draw the letter at its current position if it's still visible
                if (letter.character != ' ') {
                    display.drawChar(letter.x, static_cast<int16_t>(letter.y), letter.character, letter.gradient, ANIM_TEXT_FONT);
                }
            }
        }
        
        display.show();
        delay(40); // ~25 FPS
    }
}