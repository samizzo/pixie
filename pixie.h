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
		enum Mouse
		{
			LeftButton = 1 << 0,
			RightButton = 1 << 1
		};

		enum Key
		{
			Left,
			Right,
			Home,
			End,
			Backspace,
			Delete,
			LeftShift,
			RightShift,
			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
			Zero,
			One,
			Two,
			Three,
			Four,
			Five,
			Six,
			Seven,
			Eight,
			Nine,
			Period
		};

		Pixie();
		~Pixie();

		bool Open(const char* title, int width, int height);
		void Close();
		bool Update();

		// Returns true in the frame the mouse button went down.
		bool HasMouseGoneDown(Mouse button) const;

		// Returns true in the frame the mouse button went up.
		bool HasMouseGoneUp(Mouse button) const;

		// Returns true if the mouse button is currently down.
		bool IsMouseDown(Mouse button) const;

		// Returns true in the frame the key specified went down.
		bool HasKeyGoneDown(Key key) const;

		// Returns true if any key went down in the frame.
		bool HasAnyKeyGoneDown() const;

		// Returns true in the frame the key specified went up.
		bool HasKeyGoneUp(Key key) const;

		// Returns true if the key specified is currently down.
		bool IsKeyDown(Key key) const;

		// Returns the ASCII character for the specified key if valid, based on any current keyboard state (e.g. shift).
		char GetChar(Key key) const;

		int GetMouseX() const;
		int GetMouseY() const;
		bool IsMouseOverWindow() const;

		float GetDelta() const;

		PixelBuffer* GetPixelBuffer() const;

	private:
		void UpdateMouse();
		void UpdateKeyboard();

		HWND m_window;

		int m_width;
		int m_height;

		int m_mouseX;
		int m_mouseY;
		uint32 m_lastMouseButtonDown;
		uint32 m_mouseButtonDown;

		uint8 m_lastKeyboardState[256];
		uint8 m_keyboardState[256];

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

inline bool Pixie::HasMouseGoneDown(Mouse button) const
{
	return (m_lastMouseButtonDown & button) == 0 && (m_mouseButtonDown & button) != 0;
}

inline bool Pixie::HasMouseGoneUp(Mouse button) const
{
	return (m_lastMouseButtonDown & button) != 0 && (m_mouseButtonDown & button) == 0;
}

inline bool Pixie::IsMouseDown(Mouse button) const
{
	return (m_mouseButtonDown & button) != 0;
}

inline bool Pixie::HasAnyKeyGoneDown() const
{
	for (int i = 0; i < sizeof(m_keyboardState); i++)
	{
		if ((m_lastKeyboardState[i] & 1<<7) == 0 && (m_keyboardState[i] & 1<<7) != 0)
			return true;
	}

	return false;
}

