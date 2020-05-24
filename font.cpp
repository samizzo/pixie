#include "core.h"
#if PIXIE_PLATFORM_WIN
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include "font.h"
#include "pixie.h"
#if PIXIE_PLATFORM_WIN
#include <Windows.h>
#else
#include <string.h>
#endif

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

using namespace Pixie;

Font::~Font()
{
    delete[] m_fontBuffer;
}

bool Font::Load(const char* filename, int characterSizeX, int characterSizeY)
{
    m_characterSizeX = characterSizeX;
    m_characterSizeY = characterSizeY;

    FILE* infile = fopen(filename, "rb");
    if (!infile)
        return false;

    BITMAPFILEHEADER bmfh;
    fread(&bmfh, sizeof(bmfh), 1, infile);
    if (bmfh.bfType != 0x4d42) // 'MB'
    {
        fclose(infile);
        return false;
    }

    BITMAPINFOHEADER bmih;
    fread(&bmih, sizeof(bmih), 1, infile);
    if (bmih.biSize != sizeof(bmih))
    {
        fclose(infile);
        return false;
    }

    if ((bmih.biBitCount != 32 && bmih.biBitCount != 24) || bmih.biCompression != BI_RGB)
    {
        fclose(infile);
        return false;
    }

    fseek(infile, bmfh.bfOffBits, SEEK_SET);

    m_width = bmih.biWidth;
    m_height = abs(bmih.biHeight);

    uint32_t size = 256 * m_characterSizeX * m_characterSizeY;
    m_fontBuffer = new uint32_t[size];

    if (bmih.biBitCount == 32)
    {
        fread(m_fontBuffer, 1, size * 4, infile);
    }
    else if (bmih.biBitCount == 24)
    {
        // read into 24bit buf and expand
        uint8_t* buf24 = new uint8_t[size * 3];
        fread(buf24, 1, size * 3, infile);

        uint8_t* pcurr = buf24;
        for (uint32_t i = 0; i < size; ++i)
        {
            // from: R,G,B
            // to:   xRGB
            m_fontBuffer[i] =
                0xff000000 |
                (*(pcurr) << 16) |
                (*(pcurr + 1) << 8) |
                (*(pcurr + 2));

            pcurr += 3;
        }

        delete[] buf24;
    }

    // if we don't have a negative height, the bmp is stored upside-down, so go through and flip it
    if (bmih.biHeight > 0)
    {
        uint32_t* rowbuf = new uint32_t[m_width];
        // flip all rows apart from the middle one if the image has an odd height
        // h = 4; h/2 = 2; y 0,fy 3 | y 1,fy 2
        // h = 5; h/2 = 2; y 0,fy 4 | y 1,fy 3  [row 2 unchanged]
        for (uint32_t y = 0; y < m_height / 2; ++y)
        {
            uint32_t flipy = (m_height - (y + 1));

            void* row = m_fontBuffer + (m_width * y);
            void* fliprow = m_fontBuffer + (m_width * flipy);
            uint32_t rowsize = m_width * sizeof(uint32_t);

            // 1. copy fliprow into rowbuf
            memcpy(rowbuf, fliprow, rowsize);

            // 2. copy row into fliprow
            memcpy(fliprow, row, rowsize);

            // 3. copy rowbuf into row
            memcpy(row, rowbuf, rowsize);
        }

        delete[] rowbuf;
    }

    fclose(infile);

    return true;
}

void Font::Draw(const char* msg, int x, int y, Pixie::Window* window)
{
    uint32_t* pixels = window->GetPixels();
    int width = window->GetWidth();
    int height = window->GetHeight();

    for ( ; *msg; msg++)
    {
        uint8_t c = *msg;
        uint32_t* charStart = m_fontBuffer + (c * m_characterSizeX);

        for (int cx = 0; cx < m_characterSizeX; cx++)
        {
            for (int cy = 0; cy < m_characterSizeY; cy++)
            {
                int sx = cx + x;
                int sy = cy + y;
                if (sx >= 0 && sx < width && sy >= 0 && sy < height)
                {
                    uint32_t pixel = charStart[cx + (cy * 256 * m_characterSizeX)];
                    if (pixel & 0xffffff)
                        pixels[sx+(sy*width)] = pixel;
                }
            }
        }

        x += m_characterSizeX;
    }
}

void Font::DrawColour(const char* msg, int x, int y, uint32_t colour, Pixie::Window* window)
{
    uint32_t* pixels = window->GetPixels();
    int width = window->GetWidth();
    int height = window->GetHeight();

    for (; *msg; msg++)
    {
        uint8_t c = *msg;
        uint32_t* charStart = m_fontBuffer + (c * m_characterSizeX);

        for (int cx = 0; cx < m_characterSizeX; cx++)
        {
            for (int cy = 0; cy < m_characterSizeY; cy++)
            {
                int sx = cx + x;
                int sy = cy + y;
                if (sx >= 0 && sx < width && sy >= 0 && sy < height)
                {
                    uint32_t pixel = charStart[cx + (cy * 256 * m_characterSizeX)];
                    if (pixel & 0xffffff)
                        pixels[sx + (sy*width)] = colour;
                }
            }
        }

        x += m_characterSizeX;
    }
}

int Font::GetStringWidth(const char* msg) const
{
    return (int)strlen(msg) * m_characterSizeX;
}
