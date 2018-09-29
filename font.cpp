#include "font.h"
#include "buffer.h"
#include <stdio.h>
#include <Windows.h>

using namespace Pixie;

Font::~Font()
{
	delete[] m_fontBuffer;
}

bool Font::Load(const char* filename, int characterSizeX, int characterSizeY)
{
	m_characterSizeX = characterSizeX;
	m_characterSizeY = characterSizeY;

	FILE* infile = 0;
	fopen_s(&infile, filename, "rb");
	if (!infile)
		return false;

	BITMAPFILEHEADER bmfh;
	fread_s(&bmfh, sizeof(bmfh), sizeof(bmfh), 1, infile);
	if (bmfh.bfType != 'MB')
	{
		fclose(infile);
		return false;
	}

	BITMAPINFOHEADER bmih;
	fread_s(&bmih, sizeof(bmih), sizeof(bmih), 1, infile);
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
	m_fontBuffer = new uint32[size];
	fread(m_fontBuffer, 1, size*4, infile);

	fclose(infile);

	return true;
}

void Font::Draw(const char* msg, int x, int y, Pixie::Buffer* buffer)
{
	uint32* pixels = buffer->GetPixels();
	int width = buffer->GetWidth();
	int height = buffer->GetHeight();

	for ( ; *msg; msg++)
	{
		uint8 c = *msg;
		uint32* charStart = m_fontBuffer + (c * m_characterSizeX);

		for (int cx = 0; cx < m_characterSizeX; cx++)
		{
			for (int cy = 0; cy < m_characterSizeY; cy++)
			{
				int sx = cx + x;
				int sy = cy + y;
				if (sx >= 0 && sx < width && sy >= 0 && sy < height)
				{
					uint32 pixel = charStart[cx + (cy * 256 * m_characterSizeX)];
					if (pixel != 0xff000000)
						pixels[sx+(sy*width)] = pixel;
				}
			}
		}

		x += m_characterSizeX;
	}
}

void Font::DrawColour(const char* msg, int x, int y, uint32 colour, Pixie::Buffer* buffer)
{
	uint32* pixels = buffer->GetPixels();
	int width = buffer->GetWidth();
	int height = buffer->GetHeight();

	for (; *msg; msg++)
	{
		uint8 c = *msg;
		uint32* charStart = m_fontBuffer + (c * m_characterSizeX);

		for (int cx = 0; cx < m_characterSizeX; cx++)
		{
			for (int cy = 0; cy < m_characterSizeY; cy++)
			{
				int sx = cx + x;
				int sy = cy + y;
				if (sx >= 0 && sx < width && sy >= 0 && sy < height)
				{
					uint32 pixel = charStart[cx + (cy * 256 * m_characterSizeX)];
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
