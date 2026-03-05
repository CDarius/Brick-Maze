#include <CenterGrowAndFadeAnimation.hpp>

void CenterGrowAndFadeAnimation::animate(String text, RgbColor zoomColor, RgbColor textGradientColors[ANIM_TEXT_FONT_HEIGHT], CancelToken& cancelToken) {
    uint16_t dw = display.getWidth();
    uint16_t dh = display.getHeight();
    int16_t centerX = dw / 2;
    int16_t centerY = dh / 2;
    
    uint16_t fpsTimeMs = 1000 / fps; 

    int16_t textWidth = display.getStringWidth(text, ANIM_TEXT_FONT);
    int16_t startX = (dw - textWidth) / 2;

    // --- PHASE 1: ZOOM IN (Fast) ---
    uint16_t zoomFrames = growDurationMs * fps / 1000;
    for (int i = 1; i <= zoomFrames; i++) {
        IF_CANCELLED(cancelToken, return;)

        display.clear();

        // Draw an expanding rectangle
        int16_t w = (textWidth * i) / zoomFrames;
        int16_t h = (dh * i) / zoomFrames;
        // Center the expanding rectangle
        display.fillRect(centerX - w/2, centerY - h/2, w, h, zoomColor);

        display.show();
        delay(fpsTimeMs);
    }

    // --- PHASE 2: FULL DISPLAY ---
    display.clear();
    display.drawCenteredString(0, text, textGradientColors, ANIM_TEXT_FONT);
    display.show();
    delay(middlePaudeMs);

    // --- PHASE 3: FADE OUT (Toward white/off) ---
    uint16_t fadeFrames = fadeTimeMs * fps / 1000;
    for (uint16_t i = 1; i <= fadeFrames; i++) {
        float f = static_cast<float>(i) / fadeFrames;
        // Blend the color toward black, which is led off and for this display all led off are white
        RgbColor fadeColor = RgbColor::LinearBlend(zoomColor, COLOR_BLACK, f);
        display.clear();
        display.drawCenteredString(0, text, fadeColor, ANIM_TEXT_FONT);
        display.show();
        delay(fpsTimeMs);
    }    
}