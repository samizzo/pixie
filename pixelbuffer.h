#pragma once

#include "types.h"

class PixelBuffer
{
	public:
		PixelBuffer(int width, int height);
		~PixelBuffer();

		uint32* GetPixels() const;
		int GetWidth() const;
		int GetHeight() const;

	private:
		uint32* m_pixels;
		int m_width;
		int m_height;
};

inline PixelBuffer::PixelBuffer(int width, int height)
{
	m_pixels = new uint32[width * height];
	m_width = width;
	m_height = height;
}

inline PixelBuffer::~PixelBuffer()
{
	delete[] m_pixels;
}

inline uint32* PixelBuffer::GetPixels() const
{
	return m_pixels;
}

inline int PixelBuffer::GetWidth() const
{
	return m_width;
}

inline int PixelBuffer::GetHeight() const
{
	return m_height;
}
