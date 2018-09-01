#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include "Types.h"

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))

class Pixie
{
	public:
		Pixie();
		bool Open(const char* title, int width, int height);
		void Close();
		bool Update(const uint32* buffer);

		int GetMouseX() const;
		int GetMouseY() const;
		bool IsMouseOverWindow() const;
		float GetDelta() const;

	private:
		void UpdateMousePosition();

		HWND m_window;

		int m_width;
		int m_height;

		int m_mouseX;
		int m_mouseY;

		float m_delta;
		__int64 m_lastTime;
		__int64 m_freq;
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

inline float Pixie::GetDelta() const
{
	return m_delta;
}
