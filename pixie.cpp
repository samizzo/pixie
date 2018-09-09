#include <stdlib.h>
#include "pixie.h"
#include "pixelbuffer.h"

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

Pixie::Pixie()
{
	m_delta = 0.0f;
	m_buffer = 0;
	m_mouseButtonDown = m_lastMouseButtonDown = 0;
	memset(m_lastKeyboardState, 0, sizeof(m_lastKeyboardState));
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
}

Pixie::~Pixie()
{
	delete m_buffer;
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
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
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
		UpdateMouse();
		UpdateKeyboard();
	}

	QueryPerformanceFrequency((LARGE_INTEGER*)&m_freq);
	QueryPerformanceCounter((LARGE_INTEGER*)&m_lastTime);

	m_buffer = new PixelBuffer(width, height);

	return m_window != 0;
}

bool Pixie::Update()
{
	__int64 p;
	QueryPerformanceCounter((LARGE_INTEGER*)&p);
	__int64 delta = p - m_lastTime;
	m_delta = (delta / (float)m_freq);
	m_lastTime = p;

	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	UpdateMouse();
	UpdateKeyboard();

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

void Pixie::Close()
{
	DestroyWindow(m_window);
}

void Pixie::UpdateMouse()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(m_window, &p);
	m_mouseX = p.x;
	m_mouseY = p.y;

	m_lastMouseButtonDown = m_mouseButtonDown;
	m_mouseButtonDown = 0;
	if ((GetAsyncKeyState(VK_LBUTTON) & 1<<15) != 0)
		m_mouseButtonDown |= Mouse::LeftButton;
	if ((GetAsyncKeyState(VK_RBUTTON) & 1<<15) != 0)
		m_mouseButtonDown |= Mouse::RightButton;
}

void Pixie::UpdateKeyboard()
{
	memcpy(m_lastKeyboardState, m_keyboardState, sizeof(m_keyboardState));
	GetKeyboardState(m_keyboardState);
}

static const uint8 s_vkKeyMap[] =
{
	VK_LEFT,
	VK_RIGHT,
	VK_HOME,
	VK_END,
	VK_BACK,
	VK_DELETE,
	VK_LSHIFT,
	VK_RSHIFT,
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	VK_OEM_PERIOD
};

bool Pixie::HasKeyGoneDown(Key key) const
{
	uint8 vkCode = s_vkKeyMap[key];
	return (m_lastKeyboardState[vkCode] & 1<<7) == 0 && (m_keyboardState[vkCode] & 1<<7) != 0;
}

bool Pixie::HasKeyGoneUp(Key key) const
{
	uint8 vkCode = s_vkKeyMap[key];
	return (m_lastKeyboardState[vkCode] & 1<<7) != 0 && (m_keyboardState[vkCode] & 1<<7) == 0;
}

bool Pixie::IsKeyDown(Key key) const
{
	uint8 vkCode = s_vkKeyMap[key];
	return (m_keyboardState[vkCode] & 1<<7) != 0;
}

char Pixie::GetChar(Key key) const
{
	uint8 vkCode = s_vkKeyMap[key];
	WORD character;
	ToAscii(vkCode, 0, m_keyboardState, &character, 0);
	return character & 0xff;
}

extern int main(int argc, char** argv);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
	return main(__argc, __argv);
}
