#include <stdlib.h>
#include "pixie.h"
#include "buffer.h"
#include <assert.h>

#if PIXIE_PLATFORM_WIN
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#endif

using namespace Pixie;

#if PIXIE_PLATFORM_WIN
static const char* PixieWindowClass = "PixieWindowClass";
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif;

Window::Window()
{
#if PIXIE_PLATFORM_WIN
	assert(sizeof(HWND) == sizeof(void*));
#endif

	m_delta = 0.0f;
	m_buffer = 0;

	assert(sizeof(m_mouseButtonDown) == sizeof(m_lastMouseButtonDown));
	memset(m_mouseButtonDown, 0, sizeof(m_mouseButtonDown));
	memset(m_lastMouseButtonDown, 0, sizeof(m_lastMouseButtonDown));

	memset(m_inputCharacters, 0, sizeof(m_inputCharacters));
	assert(sizeof(m_keyDown) == sizeof(m_lastKeyDown));
	memset(m_lastKeyDown, 0, sizeof(m_lastKeyDown));
	memset(m_keyDown, 0, sizeof(m_keyDown));

#if PIXIE_PLATFORM_WIN
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
#endif
}

Window::~Window()
{
	delete m_buffer;
}

bool Window::Open(const char* title, int width, int height)
{
#if PIXIE_PLATFORM_WIN
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
#endif

	m_buffer = new Buffer(width, height);

	return true;
}

bool Window::Update()
{
#if PIXIE_PLATFORM_WIN
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
#endif

	return true;
}

void Window::Close()
{
#if PIXIE_PLATFORM_WIN
	DestroyWindow((HWND)m_window);
#endif
}

void Window::UpdateMouse()
{
#if PIXIE_PLATFORM_WIN
	POINT p;
	GetCursorPos(&p);
	ScreenToClient((HWND)m_window, &p);
	m_mouseX = p.x;
	m_mouseY = p.y;
#endif

	memcpy(m_lastMouseButtonDown, m_mouseButtonDown, sizeof(m_mouseButtonDown));
}

void Window::UpdateKeyboard()
{
	memset(m_inputCharacters, 0, sizeof(m_inputCharacters));
	memcpy(m_lastKeyDown, m_keyDown, sizeof(m_keyDown));
}

void Window::AddInputCharacter(char c)
{
	if (!isprint(c))
		return;

	int length = (int)strlen(m_inputCharacters);
	if (length + 1 < sizeof(m_inputCharacters))
	{
		m_inputCharacters[length] = c;
		m_inputCharacters[length + 1] = 0;
	}
}

#if PIXIE_PLATFORM_WIN
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
#endif

extern int main(int argc, char** argv);

#if PIXIE_PLATFORM_WIN
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
#endif
