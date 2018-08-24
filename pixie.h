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

		int GetMouseX() const;
		int GetMouseY() const;
		bool IsMouseOverWindow() const;

	private:
		void UpdateMousePosition();

		HWND m_window;

		int m_width;
		int m_height;

		int m_mouseX;
		int m_mouseY;
};

inline int Pixie::GetMouseX() const
{
	return m_mouseX;
}

inline int Pixie::GetMouseY() const
{
	return m_mouseY;
}

inline bool Pixie::IsMouseOverWindow() const
{
	return m_mouseX >= 0 && m_mouseX < m_width && m_mouseY >= 0 && m_mouseY < m_height;
}
