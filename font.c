#include "core.h"
#include <stdio.h>
#include <stdlib.h>
#include "font.h"
#include "pixie.h"

#if PIXIE_PLATFORM_OSX
struct BITMAPFILEHEADER
{
    uint16_t	bfType;
    uint32_t	bfSize;
    uint16_t	bfReserved1;
    uint16_t	bfReserved2;
    uint32_t	bfOffBits;
} __attribute__((packed));

struct BITMAPINFOHEADER
{
    uint32_t biSize;
    int32_t	biWidth;
    int32_t	biHeight;
    uint16_t	biPlanes;
    uint16_t	biBitCount;
    uint32_t	biCompression;
    uint32_t	biSizeImage;
    int32_t	biXPelsPerMeter;
    int32_t	biYPelsPerMeter;
    uint32_t	biClrUsed;
    uint32_t	biClrImportant;
} __attribute__((packed));

#define BI_RGB 0
#endif

PixieFont* Pixie_FontLoad(const char* filename, int characterSizeX, int characterSizeY)
{
    FILE* infile = fopen(filename, "rb");
    if (!infile)
        return 0;

    BITMAPFILEHEADER bmfh;
    fread(&bmfh, sizeof(bmfh), 1, infile);
    if (bmfh.bfType != 0x4d42) // 'MB'
    {
        fclose(infile);
        return 0;
    }

    BITMAPINFOHEADER bmih;
    fread(&bmih, sizeof(bmih), 1, infile);
    if (bmih.biSize != sizeof(bmih))
    {
        fclose(infile);
        return 0;
    }

    if ((bmih.biBitCount != 32 && bmih.biBitCount != 24) || bmih.biCompression != BI_RGB)
    {
        fclose(infile);
        return 0;
    }

    PixieFont* font = malloc(sizeof(PixieFont));
    memset(font, 0, sizeof(PixieFont));

    font->characterSizeX = characterSizeX;
    font->characterSizeY = characterSizeY;

    fseek(infile, bmfh.bfOffBits, SEEK_SET);

    font->width = bmih.biWidth;
    font->height = abs(bmih.biHeight);

    uint32_t size = 256 * font->characterSizeX * font->characterSizeY;
    font->pixels = malloc(sizeof(uint32_t) * size);

    if (bmih.biBitCount == 32)
    {
        fread(font->pixels, 1, size * 4, infile);
    }
    else if (bmih.biBitCount == 24)
    {
        // read into 24bit buf and expand
        uint8_t* buf24 = malloc(size * 3);
        fread(buf24, 1, size * 3, infile);

        uint8_t* pcurr = buf24;
        for (uint32_t i = 0; i < size; ++i)
        {
            // from: R,G,B
            // to:   xRGB
            font->pixels[i] =
                0xff000000 |
                (*(pcurr) << 16) |
                (*(pcurr + 1) << 8) |
                (*(pcurr + 2));

            pcurr += 3;
        }

        free(buf24);
    }

    // if we don't have a negative height, the bmp is stored upside-down, so go through and flip it
    if (bmih.biHeight > 0)
    {
        uint32_t* rowbuf = malloc(sizeof(uint32_t) * font->width);
        // flip all rows apart from the middle one if the image has an odd height
        // h = 4; h/2 = 2; y 0,fy 3 | y 1,fy 2
        // h = 5; h/2 = 2; y 0,fy 4 | y 1,fy 3  [row 2 unchanged]
        for (uint32_t y = 0; y < font->height / 2; ++y)
        {
            uint32_t flipy = (font->height - (y + 1));

            void* row = font->pixels + (font->width * y);
            void* fliprow = font->pixels + (font->width * flipy);
            uint32_t rowsize = font->width * sizeof(uint32_t);

            // 1. copy fliprow into rowbuf
            memcpy(rowbuf, fliprow, rowsize);

            // 2. copy row into fliprow
            memcpy(fliprow, row, rowsize);

            // 3. copy rowbuf into row
            memcpy(row, rowbuf, rowsize);
        }

        free(rowbuf);
    }

    fclose(infile);

    return font;
}

void Pixie_FontDestroy(PixieFont* font)
{
    assert(font);
    free(font->pixels);
    free(font);
}

void Pixie_FontDraw(const PixieFont* font, const char* msg, int x, int y, PixieWindow* window)
{
    assert(font);
    assert(window);

    uint32_t* pixels = window->pixels;
    int width = window->width;
    int height = window->height;

    for ( ; *msg; msg++)
    {
        uint8_t c = *msg;
        uint32_t* charStart = font->pixels + (c * font->characterSizeX);

        for (int cx = 0; cx < font->characterSizeX; cx++)
        {
            for (int cy = 0; cy < font->characterSizeY; cy++)
            {
                int sx = cx + x;
                int sy = cy + y;
                if (sx >= 0 && sx < width && sy >= 0 && sy < height)
                {
                    uint32_t pixel = charStart[cx + (cy * 256 * font->characterSizeX)];
                    if (pixel & 0xffffff)
                        pixels[sx+(sy*width)] = pixel;
                }
            }
        }

        x += font->characterSizeX;
    }
}

void Pixie_FontDrawColour(const PixieFont* font, const char* msg, int x, int y, uint32_t colour, PixieWindow* window)
{
    assert(font);
    assert(window);

    uint32_t* pixels = window->pixels;
    int width = window->width;
    int height = window->height;

    for (; *msg; msg++)
    {
        uint8_t c = *msg;
        uint32_t* charStart = font->pixels + (c * font->characterSizeX);

        for (int cx = 0; cx < font->characterSizeX; cx++)
        {
            for (int cy = 0; cy < font->characterSizeY; cy++)
            {
                int sx = cx + x;
                int sy = cy + y;
                if (sx >= 0 && sx < width && sy >= 0 && sy < height)
                {
                    uint32_t pixel = charStart[cx + (cy * 256 * font->characterSizeX)];
                    if (pixel & 0xffffff)
                        pixels[sx + (sy*width)] = colour;
                }
            }
        }

        x += font->characterSizeX;
    }
}

int Pixie_FontGetStringWidth(const PixieFont* font, const char* msg)
{
    assert(font);
    return (int)strlen(msg) * font->characterSizeX;
}
