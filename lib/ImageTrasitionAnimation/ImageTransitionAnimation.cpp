#include "ImageTransitionAnimation.hpp"

void ImageTransitionAnimation::horizontalWipeTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint8_t lineWidth, uint16_t durationMs, uint8_t fps, CancelToken& cancelToken) {
    uint16_t width = display.getWidth();
    uint16_t height = display.getHeight();
    uint16_t animWidth = width + lineWidth; // Total width to animate including the line width

    // Calculate the total animations steps based on fps and duration
    uint16_t totalFrames = (durationMs * fps) / 1000;
    uint16_t delayPerFrame = durationMs / totalFrames;
    
    // Start the transition by copying the "fromImage" to the display
    display.copyCanvasFrom(fromImage);

    for (uint16_t frame = 1; frame <= totalFrames; frame++) {
        if (cancelToken.isCancelled()) {
            return; // Exit the transition if cancelled
        }

        // Draw the transition line
        uint16_t col = frame * animWidth / totalFrames; // Calculate the current column based on the frame
        for (uint8_t lw = 0; lw < lineWidth; lw++) {
            int16_t lineCol = col - lw;
            display.drawLine(lineCol, 0, lineCol, height - 1, lineColor);
        }

        // Copy the current column from the "toImage" to the display canvas
        uint16_t imageCol = col - lineWidth;
        display.copyCanvasFrom(toImage, 0, 0, imageCol, height, 0, 0);

        display.show();
        delay(delayPerFrame);
    }
}

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

void ImageTransitionAnimation::verticalPageScrollTransition(const RgbColor* fromImage, const RgbColor* toImage, uint16_t durationMs, uint8_t fps, CancelToken& cancelToken) {
    int16_t width = display.getWidth();
    int16_t height = display.getHeight();
    uint16_t scrollHeight = height + 1; // Total height to scroll including the extra line between the two images

    // Calculate the total animations steps based on fps and duration
    uint16_t totalFrames = (durationMs * fps) / 1000;
    uint16_t delayPerFrame = durationMs / totalFrames;

    for (uint16_t frame = 1; frame <= totalFrames; frame++) {
        if (cancelToken.isCancelled()) {
            return; // Exit the transition if cancelled
        }

        // Calculate the current scroll position using an EaseInOut easing function for smooth acceleration and deceleration
        float t = (float)frame / (float)totalFrames; // Normalize to 0-1
        float easedT = t < 0.5 ? 2 * t * t : (-1 + (4 - 2 * t) * t); // EaseInOut quadratic easing
        int16_t scrollDelta = (int16_t)(easedT * scrollHeight); // Calculate the current scroll delta Y based on the eased progress

        // Draw the "fromImage" up by copying only the visible portion
        display.copyCanvasFrom(fromImage, 0, scrollDelta, width, height - scrollDelta, 0, 0);

        // Draw the empty line between the two images during the transition
        int16_t emptyLineY = height - scrollDelta;
        display.drawLine(0, emptyLineY, width - 1, emptyLineY, COLOR_BLACK);

        // Draw the "toImage" up by copying only the visible portion
        int16_t toImageY = height + 1 - scrollDelta;
        int16_t toImageH = height - toImageY;
        if (toImageH > 0) {
            display.copyCanvasFrom(toImage, 0, 0, width, toImageH, 0, toImageY);
        }

        display.show();
        delay(delayPerFrame);
    }
}