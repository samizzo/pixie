#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include "types.h"

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))

namespace Pixie
{
	class Buffer;

	enum MouseButton
	{
		MouseButton_Left	= 0,
		MouseButton_Middle	= 1,
		MouseButton_Right	= 2,
		MouseButton_Num
	};

	enum Key
	{
		Escape,
		Left,
		Right,
		Home,
		End,
		Backspace,
		Delete,
		LeftShift,
		RightShift,
		A,
		Z = A + 25,
		Zero,
		Nine = Zero + 9,
		Period,
		Num
	};

	class Window
	{
		public:
			Window();
			~Window();

			// Open the Pixie window with the specified title bar, width, and height.
			bool Open(const char* title, int width, int height);

			// Close the Pixie window.
			void Close();

			// Update the Pixie window. This will copy the backing buffer to the actual window.
			bool Update();

			// Returns true in the frame the mouse button went down.
			bool HasMouseGoneDown(MouseButton button) const;

			// Returns true in the frame the mouse button went up.
			bool HasMouseGoneUp(MouseButton button) const;

			// Returns true if the mouse button is currently down.
			bool IsMouseDown(MouseButton button) const;

			// Returns true in the frame the key specified went down.
			bool HasKeyGoneDown(Key key) const;

			// Returns true if any key went down in the frame.
			bool HasAnyKeyGoneDown() const;

			// Returns true in the frame the key specified went up.
			bool HasKeyGoneUp(Key key) const;

			// Returns true if the key specified is currently down.
			bool IsKeyDown(Key key) const;

			// Returns true if any key is currently down.
			bool IsAnyKeyDown() const;

			// Returns the ASCII character for the specified key if valid, based on any current keyboard state (e.g. shift).
			char GetChar(Key key) const;

			// Returns an array of ASCII input for the current frame.
			const char* GetInputCharacters() const;

			// Clears the ASCII input for the current frame.
			void ClearInputCharacters();

			// Returns the current mouse X position.
			int GetMouseX() const;

			// Returns the current mouse Y position.
			int GetMouseY() const;

			// Returns the time delta since the last time the window was updated.
			float GetDelta() const;

			// Returns the backing buffer for the window.
			Buffer* GetBuffer() const;

		private:
			void AddInputChar(char c);

			void UpdateMouse();
			void UpdateKeyboard();

			int m_width;
			int m_height;

			int m_mouseX;
			int m_mouseY;
			bool m_lastMouseButtonDown[MouseButton_Num];
			bool m_mouseButtonDown[MouseButton_Num];

			int m_keyMap[Key::Num];
			bool m_lastKeyDown[256];
			bool m_keyDown[256];
			bool m_anyKeyDown;
			char m_inputCharacters[16+1];

			float m_delta;

			Buffer* m_buffer;

			HWND m_window;
			__int64 m_lastTime;
			__int64 m_freq;

			static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
			LRESULT CALLBACK WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	inline int Window::GetMouseX() const
	{
		return m_mouseX;
	}

	inline int Window::GetMouseY() const
	{
		return m_mouseY;
	}

	inline float Window::GetDelta() const
	{
		return m_delta;
	}

	inline Pixie::Buffer* Window::GetBuffer() const
	{
		return m_buffer;
	}

	inline bool Window::HasMouseGoneDown(MouseButton button) const
	{
		return !m_lastMouseButtonDown[button] && m_mouseButtonDown[button];
	}

	inline bool Window::HasMouseGoneUp(MouseButton button) const
	{
		return m_lastMouseButtonDown[button] && !m_mouseButtonDown[button];
	}

	inline bool Window::IsMouseDown(MouseButton button) const
	{
		return m_mouseButtonDown[button];
	}

	inline bool Window::HasAnyKeyGoneDown() const
	{
		return m_anyKeyDown;
	}

	inline bool Window::HasKeyGoneDown(Key key) const
	{
		uint8 index = m_keyMap[key];
		return !m_lastKeyDown[index] && m_keyDown[index];
	}

	inline bool Window::HasKeyGoneUp(Key key) const
	{
		uint8 index = m_keyMap[key];
		return m_lastKeyDown[index] && !m_keyDown[index];
	}

	inline bool Window::IsKeyDown(Key key) const
	{
		uint8 index = m_keyMap[key];
		return m_keyDown[index];
	}

	inline bool Window::IsAnyKeyDown() const
	{
		for (int i = 0; i < Key::Num; i++)
		{
			if (IsKeyDown((Key)i))
				return true;
		}

		return false;
	}

	inline const char* Window::GetInputCharacters() const
	{
		return m_inputCharacters;
	}

	inline void Window::ClearInputCharacters()
	{
		m_inputCharacters[0] = 0;
	}
}
