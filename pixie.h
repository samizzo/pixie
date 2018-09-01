#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include "types.h"

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))

class PixelBuffer;

class Pixie
{
	public:
		Pixie();
		~Pixie();

		bool Open(const char* title, int width, int height);
		void Close();
		bool Update();

		int GetMouseX() const;
		int GetMouseY() const;
		bool IsMouseOverWindow() const;
		float GetDelta() const;

		PixelBuffer* GetPixelBuffer() const;

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

		PixelBuffer* m_buffer;
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

inline PixelBuffer* Pixie::GetPixelBuffer() const
{
	return m_buffer;
}
