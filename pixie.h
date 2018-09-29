#pragma once

#include <assert.h>
#include <cstdint>
#include "core.h"

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
		Key_Escape,
		Key_Left,
		Key_Right,
		Key_Home,
		Key_End,
		Key_Backspace,
		Key_Delete,
		Key_LeftShift,
		Key_RightShift,
		Key_Num
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

			// Used by the window procedure to update key and mouse state.
			void SetMouseButtonDown(MouseButton button, bool down);
			void SetKeyDown(int key, bool down);
			void AddInputCharacter(char c);

		private:
			void PlatformInit();
			bool PlatformOpen(const char* title, int width, int height);
			bool PlatformUpdate();
			void PlatformClose();

			void UpdateMouse();
			void UpdateKeyboard();

			int m_mouseX;
			int m_mouseY;
			bool m_lastMouseButtonDown[MouseButton_Num];
			bool m_mouseButtonDown[MouseButton_Num];

			int m_keyMap[Key_Num];
			bool m_lastKeyDown[256];
			bool m_keyDown[256];
			bool m_anyKeyDown;
			char m_inputCharacters[16+1];

			float m_delta;

			Buffer* m_buffer;

			void* m_window;
			int64_t m_lastTime;
			int64_t m_freq;
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
		for (int i = 0; i < Key_Num; i++)
		{
			if (HasKeyGoneDown((Key)i))
				return true;
		}

		return false;
	}

	inline bool Window::HasKeyGoneDown(Key key) const
	{
		int index = m_keyMap[key];
		assert(index >= 0 && index < sizeof(m_keyDown));
		return !m_lastKeyDown[index] && m_keyDown[index];
	}

	inline bool Window::HasKeyGoneUp(Key key) const
	{
		int index = m_keyMap[key];
		assert(index >= 0 && index < sizeof(m_keyDown));
		return m_lastKeyDown[index] && !m_keyDown[index];
	}

	inline bool Window::IsKeyDown(Key key) const
	{
		int index = m_keyMap[key];
		assert(index >= 0 && index < sizeof(m_keyDown));
		return m_keyDown[index];
	}

	inline bool Window::IsAnyKeyDown() const
	{
		for (int i = 0; i < Key_Num; i++)
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

	inline void Window::SetMouseButtonDown(MouseButton button, bool down)
	{
		m_mouseButtonDown[button] = down;
	}

	inline void Window::SetKeyDown(int key, bool down)
	{
		m_keyDown[key] = down;
	}
}
