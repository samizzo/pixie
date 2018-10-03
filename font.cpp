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

    if (bmih.biBitCount != 32 || bmih.biCompression != BI_RGB)
    {
        fclose(infile);
        return false;
    }

    fseek(infile, bmfh.bfOffBits, SEEK_SET);

    m_width = bmih.biWidth;
    m_height = bmih.biHeight;

    int size = 256 * m_characterSizeX * m_characterSizeY;
    m_fontBuffer = new uint32_t[size];
    fread(m_fontBuffer, 1, size*4, infile);

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
                    if (pixel != 0xff000000)
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
                    if (pixel != 0xff000000)
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
