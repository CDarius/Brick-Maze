#pragma once

#include <PuzzleDisplay.hpp>
#include <CancelToken.hpp>

class ImageTransitionAnimation {
private:
    PuzzleDisplay& display;

public:
    ImageTransitionAnimation(PuzzleDisplay& display) : display(display) {}

    void horizontalWipeTransition(const RgbColor* fromImage, const RgbColor* toImage, uint16_t durationMs) {
        uint16_t width = display.getWidth();
        uint16_t height = display.getHeight();

        // Calculate the delay between each column transition
        uint16_t delayPerColumn = durationMs / width;

        for (uint16_t col = 0; col < width; col++) {
            // Copy the current column from the "toImage" to the display canvas
            for (uint16_t row = 0; row < height; row++) {
                uint16_t pixelIndex = row * width + col;
                display.copyCanvasFrom(toImage, col, row, 1, 1, col, row);
            }
            display.show();
            delay(delayPerColumn);
        }
    }


    /**
     * Two horizontal lines will move simultaneously from the center to the edges, revealing the toImage and covering the fromImage.
     * @param fromImage is the image that will be covered by the transition (it should be already displayed before starting the animation).
     * @param toImage is the image that will be revealed by the transition.
     * @param lineColor is the color of the lines that will move. If it's the same as the background color, it will create a "curtain" effect.
     * @param durationMs is the total duration of the transition in milliseconds.
     * @param cancelToken is used to cancel the transition prematurely.
     */
    void horizontalCenterTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint16_t durationMs, CancelToken& cancelToken);

};