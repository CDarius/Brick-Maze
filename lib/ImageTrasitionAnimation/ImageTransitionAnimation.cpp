#include "ImageTransitionAnimation.hpp"

void ImageTransitionAnimation::horizontalCenterTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint16_t durationMs, CancelToken& cancelToken) {
    int16_t hh = display.getHeight() / 2;
    uint16_t delayMs = durationMs / (hh + 1); // +1 beacuse the center line must go off screen to complete the transition;

    int16_t y2 = hh; // Start from the center and move outwards
    for(int16_t y = hh - 1; y >= -1; y--) { // Start from the center and move outwards
        if (cancelToken.isCancelled()) {
            return; // Exit the transition if cancelled
        }

        display.copyCanvasFrom(fromImage); // Start from the "fromImage" for each frame to ensure proper layering of the transition

        // Draw the lines that will cover the "fromImage" and reveal the "toImage"
        display.drawLine(0, y, display.getWidth() - 1, y, lineColor); // Top line
        display.drawLine(0, y2, display.getWidth() - 1, y2, lineColor); // Bottom line
        
        // Draw "toImage" pixels revealed between the lines
        display.copyCanvasFrom(toImage, 0, y + 1, display.getWidth(), y2 - y -1, 0, y + 1);

        display.show();
        delay(delayMs);

        y2++; // Move the bottom line downwards
    }
}