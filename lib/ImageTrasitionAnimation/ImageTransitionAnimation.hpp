#pragma once

#include <PuzzleDisplay.hpp>
#include <CancelToken.hpp>

class ImageTransitionAnimation {
private:
    PuzzleDisplay& display;

public:
    ImageTransitionAnimation(PuzzleDisplay& display) : display(display) {}

    /**
     * A simple horizontal wipe transition where columns of pixels from the toImage are revealed 
     * one by one from left to right, covering the fromImage.
      * @param fromImage is the image that will be covered by the transition (it should be already displayed before starting the animation).
      * @param toImage is the image that will be revealed by the transition.
      * @param lineColor is the color of the lines that will move.
      * @param lineWidth is the width of the lines that will move. If it's 0, it will only copy the pixels without drawing lines.
      * @param durationMs is the total duration of the transition in milliseconds.
      * @param fps is the frames per second for the transition animation.
      * @param cancelToken is used to cancel the transition prematurely.
     */
    void horizontalWipeTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint8_t lineWidth, uint16_t durationMs, uint8_t fps, CancelToken& cancelToken);

    /**
     * Two horizontal lines will move simultaneously from the center to the edges, revealing the toImage and covering the fromImage.
     * @param fromImage is the image that will be covered by the transition (it should be already displayed before starting the animation).
     * @param toImage is the image that will be revealed by the transition.
     * @param lineColor is the color of the lines that will move. If it's the same as the background color, it will create a "curtain" effect.
     * @param durationMs is the total duration of the transition in milliseconds.
     * @param cancelToken is used to cancel the transition prematurely.
     */
    void horizontalCenterTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint16_t durationMs, CancelToken& cancelToken);

    /**
     * Inverse of horizontalCenterTransition: two horizontal lines move from the top and bottom borders toward the center,
     * revealing the toImage from the edges inward.
     * @param fromImage is the image that will be covered by the transition (it should be already displayed before starting the animation).
     * @param toImage is the image that will be revealed by the transition.
     * @param lineColor is the color of the lines that will move. If it's the same as the background color, it will create a "curtain" effect.
     * @param durationMs is the total duration of the transition in milliseconds.
     * @param cancelToken is used to cancel the transition prematurely.
     */
    void horizontalCenterInverseTransition(const RgbColor* fromImage, const RgbColor* toImage, RgbColor lineColor, uint16_t durationMs, CancelToken& cancelToken);

    /**
     * A vertical page scroll transition where the fromImage scrolls out and the toImage scrolls in from bottom to top.
     * The transition use a EaseInOut easing for a smooth acceleration and deceleration effect.
     * @param fromImage is the image that will be scrolled out upwards by the transition (it should be already displayed before starting the animation).
     * @param toImage is the image that will be revealed by the transition.
     * @param durationMs is the total duration of the transition in milliseconds.
     * @param fps is the frames per second for the transition animation.
     * @param cancelToken is used to cancel the transition prematurely.
     */
    void verticalPageScrollTransition(const RgbColor* fromImage, const RgbColor* toImage, uint16_t durationMs, uint8_t fps, CancelToken& cancelToken);

    /**
     * A vertical page scroll transition where the current image scrolls out upwards leaving an empty screen.
     * The transition use a EaseInOut easing for a smooth acceleration and deceleration effect.
     * @param fromImage is the image that will be scrolled out upwards by the transition (it should be already displayed before starting the animation).
     * @param durationMs is the total duration of the transition in milliseconds.
     * @param fps is the frames per second for the transition animation.
     * @param cancelToken is used to cancel the transition prematurely.
     */
    void verticalPageScrollOutTransition(const RgbColor* fromImage, uint16_t durationMs, uint8_t fps, CancelToken& cancelToken) {
        RgbColor emptyImage[TOTAL_LEDS] = {0}; // Create an empty image (all black)
        for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
            emptyImage[i] = COLOR_BLACK;
        }
        verticalPageScrollTransition(fromImage, emptyImage, durationMs, fps, cancelToken);
    }
};