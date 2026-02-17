#pragma once

#include <PuzzleDisplay.hpp>
#include <CancelToken.hpp>

#define TEXT_POSITION_CENTER 0
#define TEXT_POSITION_LEFT   1
#define TEXT_POSITION_RIGHT  2

#define ANIM_TEXT_FONT FONT_6x8
#define ANIM_TEXT_FONT_HEIGHT 8
#define ANIM_TEXT_FPS 25
#define ANIM_TEXT_FRAME_DELAY_MS (1000 / ANIM_TEXT_FPS)

#define ANIM_V_SCROLL_DIRECTION_TOP_TO_BOTTOM 0
#define ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP 1

class TextAnimation {
private:
    PuzzleDisplay& display;

    // Last completes animation status
    String lastAnimatedText;
    RgbColor lastAnimatedTextColor[PANEL_HEIGHT];
    uint8_t lastAnimatedTextPosition;    

    /**
     * Helper to calculate the X position for text justification
     * @param text The text to justify
     * @param textPosition One of TEXT_POSITION_CENTER, TEXT_POSITION_LEFT, TEXT_POSITION_RIGHT
     * @return The X position for the text
     */
    int16_t justifyText(String text, uint8_t textPosition) {
        if (textPosition == TEXT_POSITION_LEFT) {
            return 0; // Left-aligned
        } else{
            uint16_t textWidth = display.getStringWidth(text, ANIM_TEXT_FONT);

            if (textPosition == TEXT_POSITION_RIGHT) {
                return display.getWidth() - textWidth; // Right-aligned
            } else {
                return (display.getWidth() - textWidth) / 2; // Centered
            }
        } 
    }

    /**
     * Helper to print text at a specific offset (used for animation)
      * @param text The text to print
      * @param color The text color
      * @param textPosition One of TEXT_POSITION_CENTER, TEXT_POSITION_LEFT, TEXT_POSITION_RIGHT
      * @param xOffset X offset to apply to the justified position (can be negative)
      * @param yOffset Y offset to apply to the justified position (can be negative)
      */
    void printText(String text, RgbColor color[], uint8_t textPosition, int16_t xOffset, int16_t yOffset) {
        int16_t xPos = justifyText(text, textPosition);        
        int16_t x = xPos + xOffset;
        display.drawString(x, yOffset, text, color, ANIM_TEXT_FONT);
        display.show();
    }

    /**
     * Helper to store the last animation status so that it can be used as a start status for the next animation 
     * (e.g. for vertical scroll in, we need to know the previous text and its color to scroll it together with the new text)
     * @param text The text to store
     * @param color The text color to store
     * @param textPosition The text position to store
     */
    void storeLastAnimation(String text, RgbColor color[], uint8_t textPosition) {
        lastAnimatedText = text;
        for (uint8_t i = 0; i < PANEL_HEIGHT; i++) {
            lastAnimatedTextColor[i] = color[i];
        }
        lastAnimatedTextPosition = textPosition;
    }

public:
    TextAnimation(PuzzleDisplay& display) : display(display) {

    }

    /**
     * Show a text on the display with the specified color and position
     * Display the text immediately without any animation
     * @param text The text to display
     * @param color The text color
     * @param textPosition One of TEXT_POSITION_CENTER, TEXT_POSITION_LEFT, TEXT_POSITION_RIGHT
     */
    void showText(String text, RgbColor color, uint8_t textPosition) {
        RgbColor colorArray[PANEL_HEIGHT];
        for (uint8_t i = 0; i < PANEL_HEIGHT; i++) {
            colorArray[i] = color;
        }
        showText(text, colorArray, textPosition);
    }

    /**
     * Show a text on the display with the specified color and position
     * Display the text immediately without any animation
     * @param text The text to display
     * @param color Verical gradient color array (color[0] first char pixel row, color[1] second char pixel row, etc.). It must match the font height.
     * @param textPosition One of TEXT_POSITION_CENTER, TEXT_POSITION_LEFT, TEXT_POSITION_RIGHT
     */
    void showText(String text, RgbColor color[], uint8_t textPosition) {
        display.clear();
        printText(text, color, textPosition, 0, 0);
        display.show();

        storeLastAnimation(text, color, textPosition); // Store last animation status
    }

    /**
     * Animate a text scrolling vertically into the display from top or bottom
     * If a text is already display it will also scroll together with the new text.
     * @param text The text to display
     * @param color Verical gradient color array for the new text (color[0] first char pixel row, color[1] second char pixel row, etc.). It must match the font height.
     * @param textPosition One of TEXT_POSITION_CENTER, TEXT_POSITION_LEFT, TEXT_POSITION_RIGHT
     * @param gap The gap in pixels between the old text and the new text
     * @param direction One of ANIM_V_SCROLL_DIRECTION_TOP_TO_BOTTOM, ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP
     * @param cancelToken A token to cancel the animation before it completes (e.g. if we want to interrupt the animation to show something else). The animation will check the token status at each frame and stop if it's cancelled.
     */
    void verticalScrollIn(String text, RgbColor color[], uint8_t textPosition, uint8_t gap, uint8_t direction, CancelToken& cancelToken) {
        bool showOldText = lastAnimatedText.length() > 0;

        int16_t yScroll = showOldText ? PANEL_HEIGHT + gap : PANEL_HEIGHT; // If there's old text, scroll all the way out, otherwise just scroll the new text in
        int16_t scrollDir = 1;
        int16_t yOffset = 0;
        if (direction == ANIM_V_SCROLL_DIRECTION_BOTTOM_TO_TOP) {
            yScroll = -yScroll;
            scrollDir = -1;
        }
        int16_t newTextYOffset = -yScroll; // Start new text off-screen

        while (yOffset != yScroll && !cancelToken.isCancelled()) {            
            display.clear();
            if (showOldText) {
                printText(lastAnimatedText, lastAnimatedTextColor, lastAnimatedTextPosition, 0, yOffset); // Move old text
            }
            printText(text, color, textPosition, 0, yOffset + newTextYOffset);
            yOffset += scrollDir;
            display.show();
            delay(ANIM_TEXT_FRAME_DELAY_MS); // Delay between each frame
        }

        // Store the new text as last animation status for the next animation
        storeLastAnimation(text, color, textPosition);
    }
};