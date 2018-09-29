#pragma once

#include "core.h"

namespace Pixie
{
	// Simple class to wrap a 2D buffer.
	class Buffer
	{
		public:
			Buffer(int width, int height);
			~Buffer();

			uint32_t* GetPixels() const;
			int GetWidth() const;
			int GetHeight() const;

		private:
			uint32_t* m_pixels;
			int m_width;
			int m_height;
	};

	inline Buffer::Buffer(int width, int height)
	{
		m_pixels = new uint32_t[width * height];
		m_width = width;
		m_height = height;
	}

	inline Buffer::~Buffer()
	{
		delete[] m_pixels;
	}

	inline uint32_t* Pixie::Buffer::GetPixels() const
	{
		return m_pixels;
	}

	inline int Pixie::Buffer::GetWidth() const
	{
		return m_width;
	}

	inline int Pixie::Buffer::GetHeight() const
	{
		return m_height;
	}
}
