#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

typedef unsigned int uint32;
typedef unsigned char uint8;

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))

class Pixie
{
	public:
		bool Open(const char* title, int width, int height);
		void Close();
		bool Update(const uint32* buffer);

	private:
		HWND m_window;
		int m_width;
		int m_height;
};
