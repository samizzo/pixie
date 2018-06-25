#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

class Pixie
{
	public:
		bool Open(const char* title, int width, int height);
		void Close();
		bool Update();

	private:
		HWND m_window;
};
