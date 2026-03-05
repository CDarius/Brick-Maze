#pragma once

#include <TextAnimation.hpp>
#include <CancelToken.hpp>

class CenterGrowAndFadeAnimation {
    public:
        CenterGrowAndFadeAnimation(PuzzleDisplay& display, uint16_t growDurationMs, uint16_t middlePaudeMs, uint16_t fadeTimeMs, uint8_t fps = ANIM_TEXT_FPS) 
            : display(display), growDurationMs(growDurationMs), middlePaudeMs(middlePaudeMs), fadeTimeMs(fadeTimeMs), fps(fps) {}

        void animate(String text, RgbColor zoomColor, RgbColor textGradientColors[ANIM_TEXT_FONT_HEIGHT], CancelToken& cancelToken);

    private:
        PuzzleDisplay& display;
        uint16_t growDurationMs;
        uint16_t middlePaudeMs;
        uint16_t fadeTimeMs;
        uint8_t fps;
};