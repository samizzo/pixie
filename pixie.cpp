#include <stdlib.h>
#include "pixie.h"

static const char* PixieWindowClass = "PixieWindowClass";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYUP:
		{
			if (wParam == VK_ESCAPE)
				DestroyWindow(hWnd);
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool Pixie::Open(const char* title, int width, int height)
{
	HINSTANCE hInstance = GetModuleHandle(0);

	WNDCLASS wc;
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
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
		SetWindowLongPtr(m_window, GWL_USERDATA, (LONG)this);
		ShowWindow(m_window, SW_SHOW);
		UpdateMousePosition();
	}

	return m_window != 0;
}

bool Pixie::Update(const uint32* buffer)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	UpdateMousePosition();

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
	SetDIBitsToDevice(hdc, 0, 0, m_width, m_height, 0, 0, 0, m_height, buffer, &bitmapInfo, DIB_RGB_COLORS);
	ReleaseDC(m_window, hdc);

	return true;
}

void Pixie::Close()
{
	DestroyWindow(m_window);
}

void Pixie::UpdateMousePosition()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_window, &p);
	m_mouseX = p.x;
	m_mouseY = p.y;
}

extern int main(int argc, char** argv);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
