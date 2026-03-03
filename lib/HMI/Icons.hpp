#pragma once

#include <NeoPixelBus.h>
#include "PuzzleDisplay.hpp"

// Definizione colori per la coppa (Stile 8-bit Gold)
const RgbColor C_GOLD(255, 180, 0);    // Oro base
const RgbColor C_SHINE(255, 255, 150); // Riflesso (lucidità)
const RgbColor C_SHADE(180, 90, 0);    // Ombra/Bordo
const RgbColor C_OFF(0, 0, 0);         // Spento (apparirà bianco sulla tua board)

const RgbColor ICON_TROPHY_8x8[64] = {
  C_OFF,   C_SHADE, C_GOLD,  C_GOLD,  C_GOLD,  C_GOLD,  C_SHADE, C_OFF,
  C_SHADE, C_GOLD,  C_SHINE, C_GOLD,  C_GOLD,  C_GOLD,  C_GOLD,  C_SHADE,
  C_SHADE, C_OFF,   C_GOLD,  C_GOLD,  C_GOLD,  C_GOLD,  C_OFF,   C_SHADE,
  C_OFF,   C_SHADE, C_GOLD,  C_GOLD,  C_GOLD,  C_GOLD,  C_SHADE, C_OFF,
  C_OFF,   C_OFF,   C_SHADE, C_GOLD,  C_GOLD,  C_SHADE, C_OFF,   C_OFF,
  C_OFF,   C_OFF,   C_OFF,   C_GOLD,  C_GOLD,  C_OFF,   C_OFF,   C_OFF,
  C_OFF,   C_OFF,   C_SHADE, C_GOLD,  C_GOLD,  C_SHADE, C_OFF,   C_OFF,
  C_OFF,   C_SHADE, C_GOLD,  C_GOLD,  C_GOLD,  C_GOLD,  C_SHADE, C_OFF
};
