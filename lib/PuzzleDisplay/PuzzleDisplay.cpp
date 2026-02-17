#include "PuzzleDisplay.hpp"

int32_t PuzzleDisplay::getPixelIndex(int16_t x, int16_t y) const {
    // 1. Boundary Check
    if (x < 0 || x >= TOTAL_WIDTH || y < 0 || y >= PANEL_HEIGHT) {
        return -1;
    }

    // 2. Determine which Unit we are in (0 to 7)
    int16_t unitIndex = x / PANEL_WIDTH;

    // 3. Determine the local X relative to that unit (0 to 7)
    int16_t localX = x % PANEL_WIDTH;

    // 4. Calculate Hardware Index
    // Logic: (Skip previous units) + (Skip previous columns in this unit) + (Inverted Y)
    // Note: (7 - y) handles the bottom-to-top hardware layout vs top-to-bottom logical layout
    int32_t index = (unitIndex * 64) + (localX * 8) + (7 - y);

    return index;
}

void PuzzleDisplay::show() {
    // Copy every pixel from Canvas to Strip, applying Dimming on the fly
    for(uint16_t i = 0; i < TOTAL_LEDS; i++) {
        // RgbColor::Dim(x) scales the color by x/255
        strip.SetPixelColor(i, _canvas[i].Dim(_brightness));
    }
    strip.Show();
}

void PuzzleDisplay::drawPixel(int16_t x, int16_t y, RgbColor color) {
    int32_t index = getPixelIndex(x, y);
    if (index != -1) {
        // Store the ORIGINAL color in the canvas
        _canvas[index] = color;
    }
}

// 2. Draw a filled rectangle
void PuzzleDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color) {
    if (x >= TOTAL_WIDTH || y >= PANEL_HEIGHT) {
        return; // Rectangle is completely off-screen
    }
    if (x + w <= 0 || y + h <= 0) {
        return; // Rectangle is completely off-screen
    }
    // Clip the rectangle to the display boundaries
    int16_t startX = x > 0 ? x : 0;
    int16_t endX = (x + w) < TOTAL_WIDTH ? (x + w) : TOTAL_WIDTH;
    int16_t startY = y > 0 ? y : 0;
    int16_t endY = (y + h) < PANEL_HEIGHT ? (y + h) : PANEL_HEIGHT;
    for (int16_t i = startX; i < endX; i++) {
        uint16_t pixelIndex = getPixelIndex(i, startY);
        for (int16_t j = startY; j < endY; j++) {
            _canvas[pixelIndex] = color;
            pixelIndex--; // Move to the next pixel in the same column
        }
    }
}

// 3. Draw a rectangle outline
void PuzzleDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, RgbColor color) {
    drawLine(x, y, x + w - 1, y, color);         // Top
    drawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // Bottom
    drawLine(x, y, x, y + h - 1, color);         // Left
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // Right
}

// 4. Draw a Line (Bresenham's Algorithm) with optimization for horizontal and vertical lines
void PuzzleDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, RgbColor color) {
    // Check for horizontal line
    if (y0 == y1) {
        // Optimization for horizontal line
        if (y0 < 0 || y0 >= PANEL_HEIGHT) {
            return; // Line is completely off-screen vertically
        }
        
        int16_t start = x0 < x1 ? x0 : x1;
        int16_t end = x0 > x1 ? x0 : x1;
        if (start >= TOTAL_WIDTH) {
            return; // Line is completely off-screen to the right
        }
        if (end < 0) {
            return; // Line is completely off-screen to the left
        }
        // Clip the line to the display boundaries
        start = start > 0 ? start : 0;
        end = end < TOTAL_WIDTH ? end : TOTAL_WIDTH - 1;
        
        int16_t pixeIndex = getPixelIndex(start, y0);
        for (int16_t x = start; x <= end; x++) {
            _canvas[pixeIndex] = color;
            pixeIndex += PANEL_HEIGHT; // Move to the next pixel in the same row
        }
        return;
    }

    // Check for vertical line
    if (x0 == x1) {
        // Optimization for vertical line
        if (x0 < 0 || x0 >= TOTAL_WIDTH) {
            return; // Line is completely off-screen horizontally
        }

        int16_t start = y0 < y1 ? y0 : y1;
        int16_t end = y0 > y1 ? y0 : y1;
        if (start >= PANEL_HEIGHT) {
            return; // Line is completely off-screen below
        }
        if (end < 0) {
            return; // Line is completely off-screen above
        }
        // Clip the line to the display boundaries
        start = start > 0 ? start : 0;
        end = end < PANEL_HEIGHT ? end : PANEL_HEIGHT - 1;

        int16_t pixelIndex = getPixelIndex(x0, start);
        for (int16_t y = start; y <= end; y++) {
            _canvas[pixelIndex] = color;
            pixelIndex--; // Move to the next pixel in the same column (decreasing index because of hardware layout)
        }
        return;
    }

    // Bresenham's algorithm for general lines
    int16_t dx = abs(x1 - x0);
    int16_t dy = -abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;
    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

uint8_t PuzzleDisplay::drawChar(int16_t x, int16_t y, unsigned char c, RgbColor color, uint8_t font, bool use_std_width) {
    const FontDefinition* fontDef = GET_FONT_DEFINITION(font);
    const uint8_t* fontData = GET_FONT_DATA(font);
    
    // Check if character is within font range
    if (c < fontDef->first_char || c > fontDef->last_char)
        return 0; // Character not supported by this font

    uint8_t charIndex = c - fontDef->first_char;    
    const uint8_t* charData = fontData + (charIndex * (fontDef->max_width + 1)); // +1 for width byte

    uint8_t charWidth = charData[0];
    uint8_t charHeight = fontDef->height;

    uint8_t offsetX = 0;
    if (use_std_width && charWidth < fontDef->std_width) {
        offsetX = fontDef->std_width - charWidth; // Add extra spacing to reach standard width
        charWidth = fontDef->std_width; // Use standard width for spacing
    }
        
    for (uint8_t col = 0; col < charWidth; col++) {
        uint8_t final_x = x + col + offsetX; // Apply offset for standard width if needed
        if (final_x < 0 || final_x >= TOTAL_WIDTH) {
            continue; // Skip columns that are off-screen
        }
        
        uint8_t line = charData[col + 1]; // +1 to skip width byte
        for (uint8_t row = 0; row < charHeight; row++) {
            if (line & (1 << row)) {
                drawPixel(final_x, y + row, color); 
            }
        }
    }

    return charWidth + 1; // Character width + 1 pixel spacing
}

uint8_t PuzzleDisplay::drawChar(int16_t x, int16_t y, unsigned char c, RgbColor color[], uint8_t font, bool use_std_width) {
    const FontDefinition* fontDef = GET_FONT_DEFINITION(font);
    const uint8_t* fontData = GET_FONT_DATA(font);
    
    // Check if character is within font range
    if (c < fontDef->first_char || c > fontDef->last_char)
        return 0; // Character not supported by this font

    uint8_t charIndex = c - fontDef->first_char;    
    const uint8_t* charData = fontData + (charIndex * (fontDef->max_width + 1)); // +1 for width byte

    uint8_t charWidth = charData[0];
    uint8_t charHeight = fontDef->height;

    uint8_t offsetX = 0;
    if (use_std_width && charWidth < fontDef->std_width) {
        offsetX = fontDef->std_width - charWidth; // Add extra spacing to reach standard width
        charWidth = fontDef->std_width; // Use standard width for spacing
    }
        
    for (uint8_t col = 0; col < charWidth; col++) {
        int16_t final_x = x + col + offsetX; // Apply offset for standard width if needed
        if (final_x < 0) {
            continue; // Skip columns that are off-screen
        }
        if (final_x >= TOTAL_WIDTH) {
            break; // No need to continue if we've reached the end of the display
        }
        
        uint8_t line = charData[col + 1]; // +1 to skip width byte
        int32_t pixelIndex = -1;
        for (uint8_t row = 0; row < charHeight; row++) {
            int16_t final_y = y + row;
            if (final_y < 0) {
                continue; // Skip rows that are off-screen
            }
            if (final_y >= PANEL_HEIGHT) {
                break; // No need to continue if we've reached the bottom of the display
            }
            
            if (line & (1 << row)) {
                if (pixelIndex < 0) {
                    pixelIndex = getPixelIndex(final_x, final_y);
                }
                _canvas[pixelIndex] = color[row];
            }
            pixelIndex--;
        }
    }

    return charWidth + 1; // Character width + 1 pixel spacing
}

void PuzzleDisplay::drawString(int16_t x, int16_t y, String text, RgbColor color, uint8_t fontSize, bool use_std_width) {
    int16_t cursorX = x;

    for (int i = 0; i < text.length(); i++) {
        // Check if we are off screen to save time
        if (cursorX >= TOTAL_WIDTH) break;
        
        unsigned char c = text[i];
        uint8_t charWidth = drawChar(cursorX, y, c, color, fontSize, applyStandardWidth(c, use_std_width));
        cursorX += charWidth;
    }
}

void PuzzleDisplay::drawString(int16_t x, int16_t y, String text, RgbColor color[], uint8_t font, bool use_std_width) {
    int16_t cursorX = x;

    for (int i = 0; i < text.length(); i++) {
        // Check if we are off screen to save time
        if (cursorX >= TOTAL_WIDTH) break;
        
        unsigned char c = text[i];
        uint8_t charWidth = drawChar(cursorX, y, c, color, font, applyStandardWidth(c, use_std_width));
        cursorX += charWidth;
    }
}

uint16_t PuzzleDisplay::getStringWidth(String text, uint8_t font, bool use_std_width) const {
    const FontDefinition* fontDef = GET_FONT_DEFINITION(font);
    const uint8_t* fontData = GET_FONT_DATA(font);

    uint16_t totalWidth = 0;

    for (int i = 0; i < text.length(); i++) {
        unsigned char c = text[i];

        // Check if character is within font range
        if (c < fontDef->first_char || c > fontDef->last_char)
            continue; // Character not supported by this font

        if (applyStandardWidth(c, use_std_width)) {
            totalWidth += fontDef->std_width + 1; // Standard width + 1 pixel spacing
            continue;
        } else {
            uint8_t charIndex = c - fontDef->first_char;    
            const uint8_t* charData = fontData + (charIndex * (fontDef->max_width + 1)); // +1 for width byte
            uint8_t charWidth = charData[0];
            totalWidth += charWidth + 1; // Character width + 1 pixel spacing
        }
    }

    if (totalWidth > 0) {
        totalWidth -= 1; // Remove spacing after the last character
    }

    return totalWidth;
}

void PuzzleDisplay::copyCanvasTo(RgbColor* targetCanvas) const {
    for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
        targetCanvas[i] = _canvas[i];
    }
}

void PuzzleDisplay::copyCanvasFrom(const RgbColor* sourceCanvas) {
    for (uint16_t i = 0; i < TOTAL_LEDS; i++) {
        _canvas[i] = sourceCanvas[i];
    }
}

void PuzzleDisplay::copyCanvasFrom(const RgbColor* sourceCanvas, int16_t sourceX, int16_t sourceY, int16_t width, int16_t height, int16_t destX, int16_t destY) {
    if (destX >= TOTAL_WIDTH || destY >= PANEL_HEIGHT) {
        return; // Destination is completely out of bounds
    }

    if (destX < 0) {
        // If the destination X is negative, we need to skip some columns from the source
        int16_t skipColumns = -destX;
        sourceX += skipColumns;
        width -= skipColumns;
        destX = 0; // Start at the left edge of the display
    }
    if (destY < 0) {
        // If the destination Y is negative, we need to skip some rows from the source
        int16_t skipRows = -destY;
        sourceY += skipRows;
        height -= skipRows;
        destY = 0; // Start at the top edge of the display
    }

    // Adjust sourceX and sourceY if they are negative, and reduce width and height to the maximum possible
    int16_t sourceX2 = sourceX > 0 ? sourceX : 0;
    int16_t sourceY2 = sourceY > 0 ? sourceY : 0;

    int16_t maxWidth = TOTAL_WIDTH - destX;
    int16_t maxHeight = PANEL_HEIGHT - destY;

    width = width < maxWidth ? width : maxWidth;
    height = height < maxHeight ? height : maxHeight;

    if (width <= 0 || height <= 0) {
        return; // Nothing to copy
    }
    
    if ((destX + width) < 0 || (destY + height) < 0) {
        return; // Destination is completely off-screen
    }
    
    for (int16_t x = 0; x < width; x++) {
        int16_t xs = sourceX2 + x;
        int16_t xd = destX + x;
        int32_t sourcePixelIndex = getPixelIndex(xs, sourceY2);
        int32_t destPixelIndex = getPixelIndex(xd, destY);
        for (int16_t y = 0; y < height; y++) {
            _canvas[destPixelIndex] = sourceCanvas[sourcePixelIndex];
            sourcePixelIndex--;
            destPixelIndex--;
        }
    }    
}

void PuzzleDisplay::linearColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const {
    // 1. Handle edge cases to prevent division by zero or errors
    if (colorsLength == 0) return;
    
    if (colorsLength == 1) {
        colors[0] = startColor;
        return;
    }

    // 2. Loop through the array and calculate the gradient
    for (uint8_t i = 0; i < colorsLength; i++) {
        // Calculate progress ratio between 0.0 and 1.0
        float progress = (float)i / (colorsLength - 1);

        // LinearBlend calculates the mixed color based on the progress
        colors[i] = RgbColor::LinearBlend(startColor, endColor, progress);
    }
}

void PuzzleDisplay::mirroredColorGradient(RgbColor startColor, RgbColor endColor, RgbColor* colors, uint8_t colorsLength) const {
    // 1. Safety check
    if (colorsLength == 0) return;

    // 2. Determine how many steps we have to the "peak" (middle)
    // If length is 10 (even), we have 5 steps (0-4).
    // If length is 11 (odd), we have 6 steps (0-5), where 5 is the exact center.
    uint8_t steps = (colorsLength + 1) / 2;

    // 3. Loop only through the first half
    for (uint8_t i = 0; i < steps; i++) {
        
        // Calculate progress towards the center (0.0 to 1.0)
        // We use (steps - 1) because at the last step of the half, we want exactly 'endColor'
        float progress;
        if (steps <= 1) {
            progress = 1.0f; // Edge case for very small arrays
        } else {
            progress = (float)i / (float)(steps - 1);
        }

        // Calculate the color
        RgbColor color = RgbColor::LinearBlend(startColor, endColor, progress);

        // Assign to the front
        colors[i] = color;

        // Assign to the back (mirroring)
        // This math works for both odd and even lengths.
        // If odd, the middle element overwrites itself, which is harmless.
        colors[colorsLength - 1 - i] = color;
    }
}