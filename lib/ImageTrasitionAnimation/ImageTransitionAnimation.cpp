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

void ImageTransitionAnimation::horizontalCenterInverseTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint16_t durationMs, CancelToken& cancelToken) {
    int16_t width = display.getWidth();
    int16_t height = display.getHeight();
    int16_t hh = height / 2;
    int16_t maxStep = hh + (height % 2); // Include one extra step for odd heights to reveal the center line
    uint16_t delayMs = durationMs / (maxStep + 1);

    for (int16_t step = 0; step <= maxStep; step++) {
        if (cancelToken.isCancelled()) {
            return; // Exit the transition if cancelled
        }

        int16_t yTop = step;
        int16_t yBottom = height - 1 - step;

        display.copyCanvasFrom(fromImage); // Start from the "fromImage" for each frame to ensure proper layering of the transition

        // Reveal "toImage" from top edge to top line
        if (yTop > 0) {
            display.copyCanvasFrom(toImage, 0, 0, width, yTop, 0, 0);
        }

        // Reveal "toImage" from bottom line to bottom edge
        if (yBottom < height - 1) {
            int16_t bottomStart = yBottom + 1;
            display.copyCanvasFrom(toImage, 0, bottomStart, width, height - bottomStart, 0, bottomStart);
        }

        // Draw moving lines while they are still on-screen and not crossed
        if (yTop <= yBottom) {
            display.drawLine(0, yTop, width - 1, yTop, lineColor); // Top line
            if (yBottom != yTop) {
                display.drawLine(0, yBottom, width - 1, yBottom, lineColor); // Bottom line
            }
        }

        display.show();
        delay(delayMs);
    }
}