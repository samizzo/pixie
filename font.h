#pragma once

#include "Types.h"

class PixelBuffer;

// BMP font loader. Expects the entire character set (256 characters) on one line.
class Font
{
	public:
		Font();
		~Font();

		// Loads the font in the given BMP filename using the specified character size.
		bool Load(const char* filename, int characterSizeX, int characterSizeY);

		// Draws the specified font to the pixel buffer in the font colour.
		void Draw(const char* msg, int x, int y, PixelBuffer* buffer);

		// Draws the specified font to the pixel buffer in the given colour.
		void DrawColour(const char* msg, int x, int y, uint32 colour, PixelBuffer* buffer);

		// Returns the width of the specified string in this font.
		int GetStringWidth(const char* msg) const;

		// Returns the character height of the font.
		int GetCharacterHeight() const;

		// Returns the character width of the font.
		int GetCharacterWidth() const;

	private:
		uint32* m_fontBuffer;
		uint32 m_width;
		uint32 m_height;
		uint8 m_characterSizeX;
		uint8 m_characterSizeY;
};

inline Font::Font()
{
	m_fontBuffer = 0;
	m_width = m_height = 0;
}

inline int Font::GetCharacterHeight() const
{
	return m_characterSizeY;
}

inline int Font::GetCharacterWidth() const
{
	return m_characterSizeX;
}

