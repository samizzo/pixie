#pragma once

#include "Types.h"

// BMP font loader. Expects the entire character set on one line.
class Font
{
	public:
		Font();
		~Font();
		bool Load(const char* filename, int characterSizeX, int characterSizeY);
		void Draw(const char* msg, int x, int y, uint32* buffer, int width, int height);

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
