#include <stdlib.h>
#include "pixie.h"
#include "buffer.h"
#include <assert.h>

using namespace Pixie;

static const char* PixieWindowClass = "PixieWindowClass";

Window::Window()
{
	m_delta = 0.0f;
	m_buffer = 0;

	memset(m_mouseButtonDown, 0, sizeof(m_mouseButtonDown));
	memset(m_lastMouseButtonDown, 0, sizeof(m_lastMouseButtonDown));

	memset(m_inputCharacters, 0, sizeof(m_inputCharacters));
	memset(m_lastKeyDown, 0, sizeof(m_lastKeyDown));
	memset(m_keyDown, 0, sizeof(m_keyDown));

	for (int i = 0; i < Key::Num; i++)
		m_keyMap[i] = -1;
	m_keyMap[Key::Escape] = VK_ESCAPE;
	m_keyMap[Key::Left] = VK_LEFT;
	m_keyMap[Key::Right] = VK_RIGHT;
	m_keyMap[Key::Home] = VK_HOME;
	m_keyMap[Key::End] = VK_END;
	m_keyMap[Key::Backspace] = VK_BACK;
	m_keyMap[Key::Delete] = VK_DELETE;
	m_keyMap[Key::LeftShift] = VK_LSHIFT;
	m_keyMap[Key::RightShift] = VK_RSHIFT;
	for (int i = Key::A; i <= Key::Z; i++)
		m_keyMap[i] = (i - Key::A) + 'A';
}

Window::~Window()
{
	delete m_buffer;
}

bool Window::Open(const char* title, int width, int height)
{
	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = PixieWindowClass;

	if (!RegisterClass(&wc))
		return 0;

	m_width = width;
	m_height = height;

	int style = WS_BORDER | WS_CAPTION;
	RECT rect;
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	rect.bottom = height;
	AdjustWindowRect(&rect, style, FALSE);
	m_window = CreateWindow(PixieWindowClass, title, style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	if (m_window != 0)
	{
		SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);
		ShowWindow(m_window, SW_SHOW);
	}

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_lastTime);

	m_buffer = new Buffer(width, height);

	return m_window != 0;
}

bool Window::Update()
{
	__int64 time;
	QueryPerformanceCounter((LARGE_INTEGER*)&time);
	__int64 delta = time - m_lastTime;
	m_delta = (delta / (float)m_freq);
	m_lastTime = time;

	UpdateMouse();
	UpdateKeyboard();

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	// Copy buffer to the window.
	HDC hdc = GetDC(m_window);
	BITMAPINFO bitmapInfo;
	BITMAPINFOHEADER& bmiHeader = bitmapInfo.bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = m_width;
	bmiHeader.biHeight = -m_height; // Negative indicates a top-down DIB. Otherwise DIB is bottom up.
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	SetDIBitsToDevice(hdc, 0, 0, m_width, m_height, 0, 0, 0, m_height, m_buffer->GetPixels(), &bitmapInfo, DIB_RGB_COLORS);
	ReleaseDC(m_window, hdc);

	return true;
}

void Window::Close()
{
	DestroyWindow(m_window);
}

void Window::UpdateMouse()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_window, &p);
	m_mouseX = p.x;
	m_mouseY = p.y;
	memcpy(m_lastMouseButtonDown, m_mouseButtonDown, sizeof(m_mouseButtonDown));
}

void Window::UpdateKeyboard()
{
	memset(m_inputCharacters, 0, sizeof(m_inputCharacters));
	memcpy(m_lastKeyDown, m_keyDown, sizeof(m_keyDown));
}

void Window::AddInputChar(char c)
{
	if (!isprint(c))
		return;

	int length = strlen(m_inputCharacters);
	if (length + 1 < sizeof(m_inputCharacters))
	{
		m_inputCharacters[length] = c;
		m_inputCharacters[length + 1] = 0;
	}
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
	{
		return window->WndProc(msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

LRESULT CALLBACK Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			int button = MouseButton_Left;
			if (msg == WM_MBUTTONDOWN) button = MouseButton_Middle;
			if (msg == WM_RBUTTONDOWN) button = MouseButton_Right;
			m_mouseButtonDown[button] = true;
			break;
		}

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			int button = MouseButton_Left;
			if (msg == WM_MBUTTONUP) button = MouseButton_Middle;
			if (msg == WM_RBUTTONUP) button = MouseButton_Right;
			m_mouseButtonDown[button] = false;
			break;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			if (wParam < 256)
				m_keyDown[wParam] = true;
			break;
		}

		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			if (wParam < 256)
				m_keyDown[wParam] = false;
			break;
		}

		case WM_CHAR:
		{
			if (wParam < 256)
				AddInputChar((char)wParam);
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProc(m_window, msg, wParam, lParam);
}

extern int main(int argc, char** argv);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
