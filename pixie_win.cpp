#include "pixie.h"
#include "buffer.h"
#include <assert.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>

using namespace Pixie;

static const char* PixieWindowClass = "PixieWindowClass";
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Window::PlatformInit()
{
	assert(sizeof(HWND) == sizeof(void*));

	for (int i = 0; i < Key_Num; i++)
		m_keyMap[i] = -1;
	m_keyMap[Key_Escape] = VK_ESCAPE;
	m_keyMap[Key_Left] = VK_LEFT;
	m_keyMap[Key_Right] = VK_RIGHT;
	m_keyMap[Key_Home] = VK_HOME;
	m_keyMap[Key_End] = VK_END;
	m_keyMap[Key_Backspace] = VK_BACK;
	m_keyMap[Key_Delete] = VK_DELETE;
	m_keyMap[Key_LeftShift] = VK_LSHIFT;
	m_keyMap[Key_RightShift] = VK_RSHIFT;
}

bool Window::PlatformOpen(const char* title, int width, int height)
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
		return false;

	int style = WS_BORDER | WS_CAPTION;
	RECT rect;
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	rect.bottom = height;
	AdjustWindowRect(&rect, style, FALSE);
	HWND window = CreateWindow(PixieWindowClass, title, style, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	m_window = (HWND)window;
	if (window == 0)
		return false;

	SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)this);
	ShowWindow(window, SW_SHOW);

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_lastTime);

	return true;
}

bool Window::PlatformUpdate()
{
	// Update mouse cursor location.
	POINT p;
	GetCursorPos(&p);
	ScreenToClient((HWND)m_window, &p);
	m_mouseX = p.x;
	m_mouseY = p.y;

	__int64 time;
	QueryPerformanceCounter((LARGE_INTEGER*)&time);
	__int64 delta = time - m_lastTime;
	m_delta = (delta / (float)m_freq);
	m_lastTime = time;

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	// Copy buffer to the window.
	int width = m_buffer->GetWidth();
	int height = m_buffer->GetHeight();

	HDC hdc = GetDC((HWND)m_window);
	BITMAPINFO bitmapInfo;
	BITMAPINFOHEADER& bmiHeader = bitmapInfo.bmiHeader;
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = width;
	bmiHeader.biHeight = -height; // Negative indicates a top-down DIB. Otherwise DIB is bottom up.
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 32;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;
	SetDIBitsToDevice(hdc, 0, 0, width, height, 0, 0, 0, height, m_buffer->GetPixels(), &bitmapInfo, DIB_RGB_COLORS);
	ReleaseDC((HWND)m_window, hdc);

	return true;
}

void Window::PlatformClose()
{
	DestroyWindow((HWND)m_window);
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (window)
	{
		switch (msg)
		{
			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
			{
				MouseButton button = MouseButton_Left;
				if (msg == WM_MBUTTONDOWN) button = MouseButton_Middle;
				if (msg == WM_RBUTTONDOWN) button = MouseButton_Right;
				window->SetMouseButtonDown(button, true);
				break;
			}

			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			{
				MouseButton button = MouseButton_Left;
				if (msg == WM_MBUTTONUP) button = MouseButton_Middle;
				if (msg == WM_RBUTTONUP) button = MouseButton_Right;
				window->SetMouseButtonDown(button, false);
				break;
			}

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				if (wParam < 256)
					window->SetKeyDown((int)wParam, true);
				break;
			}

			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				if (wParam < 256)
					window->SetKeyDown((int)wParam, false);
				break;
			}

			case WM_CHAR:
			{
				if (wParam < 256)
					window->AddInputCharacter((char)wParam);
				break;
			}

			case WM_DESTROY:
			{
				PostQuitMessage(0);
				break;
			}
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

extern int main(int argc, char** argv);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
