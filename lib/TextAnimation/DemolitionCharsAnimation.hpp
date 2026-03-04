#include <PuzzleDisplay.hpp>
#include <TextAnimation.hpp>
#include <CancelToken.hpp>
#include <vector>

class DemolitionCharsAnimation {
    public:
        /**
         * Constructor for the DemolitionCharsAnimation class.
         * @param display Reference to the PuzzleDisplay object to draw on.
         */
        DemolitionCharsAnimation(PuzzleDisplay& display) : display(display), chars() {}
        ~DemolitionCharsAnimation() { 
            chars.clear(); 
        }

        /**
         * Add a text to the animation. The text will be split into characters and each character will fall down with a demolition effect.
         * The characters will be colored with the provided gradient colors.
         * @param x The x-coordinate where the text animation should start.
         * @param text The text to animate. It should fit within the display dimensions.
         * @param gradientColors The gradient colors to use for the text. It should have a length equal to the font height.
         */
        void addText(int16_t x, const char* text, RgbColor gradientColors[ANIM_TEXT_FONT_HEIGHT]);

        /**
         * Animate the demolition effect for all added characters. Each character will fall down from its initial position and disappear at the bottom of the display.
         * The animation will run until all characters have disappeared. 
         * The animation can be cancelled using the cancel token, which will stop the animation immediately and leave the display in its current state.
         * @param cancelToken The cancel token to stop the animation.
         */
        void run(CancelToken& cancelToken);

    private:
        PuzzleDisplay& display;
        
        // Structure to represent each falling letter
        struct FallingLetter {
            char character;
            int16_t x;
            float y;
            float velocity;
            bool fallStarted;
            RgbColor gradient[ANIM_TEXT_FONT_HEIGHT];
        };

        std::vector<FallingLetter> chars;
};