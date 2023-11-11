#pragma once

#include <stdint.h>
#include "core.h"

// BMP font loader. Expects the entire character set (256 ASCII characters) on one line.
typedef struct _PixieFont
{
    uint32_t* pixels;
    uint32_t width;
    uint32_t height;
    uint8_t characterSizeX;
    uint8_t characterSizeY;
} PixieFont;

// Loads the font in the given BMP filename using the specified character size.
PixieFont* Pixie_FontLoad(const char* filename, int characterSizeX, int characterSizeY);

// Frees the font.
void Pixie_FontDestroy(PixieFont* font);

// Draws the specified font to the window in the font colour.
void Pixie_FontDraw(const PixieFont* font, const char* msg, int x, int y, PixieWindow* window);

// Draws the specified font to the window in the given colour.
void Pixie_FontDrawColour(const PixieFont* font, const char* msg, int x, int y, uint32_t colour, PixieWindow* window);

// Returns the width of the specified string in this font.
int Pixie_FontGetStringWidth(const PixieFont* font, const char* msg);
