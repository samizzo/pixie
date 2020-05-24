#include "pixie.h"
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

    m_keyMap[Key_Backspace] = VK_BACK;
    m_keyMap[Key_Tab] = VK_TAB;
    m_keyMap[Key_Enter] = VK_RETURN;
    m_keyMap[Key_Escape] = VK_ESCAPE;
    m_keyMap[Key_Up] = VK_UP;
    m_keyMap[Key_Down] = VK_DOWN;
    m_keyMap[Key_Left] = VK_LEFT;
    m_keyMap[Key_Right] = VK_RIGHT;
    m_keyMap[Key_Home] = VK_HOME;
    m_keyMap[Key_End] = VK_END;
    m_keyMap[Key_PageUp] = VK_PRIOR;
    m_keyMap[Key_PageDown] = VK_NEXT;
    m_keyMap[Key_Delete] = VK_DELETE;
    m_keyMap[Key_Insert] = VK_INSERT;
    m_keyMap[Key_LeftShift] = VK_LSHIFT;
    m_keyMap[Key_RightShift] = VK_RSHIFT;
    m_keyMap[Key_LeftControl] = VK_LCONTROL;
    m_keyMap[Key_RightControl] = VK_RCONTROL;
    m_keyMap[Key_F1] = VK_F1;
    m_keyMap[Key_F2] = VK_F2;
    m_keyMap[Key_F3] = VK_F3;
    m_keyMap[Key_F4] = VK_F4;
    m_keyMap[Key_F5] = VK_F5;
    m_keyMap[Key_F6] = VK_F6;
    m_keyMap[Key_F7] = VK_F7;
    m_keyMap[Key_F8] = VK_F8;
    m_keyMap[Key_F9] = VK_F9;
    m_keyMap[Key_F10] = VK_F10;
    m_keyMap[Key_F11] = VK_F11;
    m_keyMap[Key_F12] = VK_F12;
    m_keyMap['['] = VK_OEM_4;
    m_keyMap['\\'] = VK_OEM_5;
    m_keyMap[']'] = VK_OEM_6;
    m_keyMap['`'] = VK_OEM_3;
    m_keyMap['\''] = VK_OEM_7;
    //m_keyMap['='] = kVK_ANSI_Equal;
    m_keyMap['-'] = VK_OEM_MINUS;
    m_keyMap['.'] = VK_OEM_PERIOD;
    m_keyMap['/'] = VK_OEM_2;
    m_keyMap[';'] = VK_OEM_1;
    m_keyMap[','] = VK_OEM_COMMA;
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

    m_scalex = (float)m_scale;
    m_scaley = (float)m_scale;

    int style = 0;
    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
    int xPos, yPos;

    if (m_fullscreen)
    {
        style = WS_POPUP;
        xPos = yPos = 0;
        width = desktopWidth;
        height = desktopHeight;
        m_scalex = width / (float)m_width;
        m_scaley = m_maintainAspectRatio ? m_scalex : (height / (float)m_height);
    }
    else
    {
        style = WS_BORDER | WS_CAPTION;

        RECT rect;
        rect.left = 0;
        rect.right = width * m_scale;
        rect.top = 0;
        rect.bottom = height * m_scale;
        AdjustWindowRect(&rect, style, FALSE);

        xPos = (desktopWidth - rect.right) >> 1;
        yPos = (desktopHeight - rect.bottom) >> 1;

        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    m_windowWidth = width;
    m_windowHeight = height;

    HWND window = CreateWindow(PixieWindowClass, title, style, xPos, yPos, width, height, NULL, NULL, hInstance, NULL);
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

    if (m_fullscreen)
    {
        m_mouseX = (int)(m_mouseX / m_scalex);
        m_mouseY = (int)(m_mouseY / m_scaley);
    }
    else
    {
        m_mouseX /= m_scale;
        m_mouseY /= m_scale;
    }

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
    HDC hdc = GetDC((HWND)m_window);
    BITMAPINFO bitmapInfo;
    BITMAPINFOHEADER& bmiHeader = bitmapInfo.bmiHeader;
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = m_width;
    bmiHeader.biHeight = -(int32_t)m_height; // Negative indicates a top-down DIB. Otherwise DIB is bottom up.
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 32;
    bmiHeader.biCompression = BI_RGB;
    bmiHeader.biSizeImage = 0;
    bmiHeader.biXPelsPerMeter = 0;
    bmiHeader.biYPelsPerMeter = 0;
    bmiHeader.biClrUsed = 0;
    bmiHeader.biClrImportant = 0;
    if (m_scale > 1 || m_fullscreen)
    {
        int yofs = 0;
        int destWidth = (int)(m_width * m_scalex);
        int destHeight = (int)(m_height * m_scaley);
        if (m_maintainAspectRatio)
            yofs = (m_windowHeight - destHeight) >> 1;

        if (m_maintainAspectRatio && yofs > 0)
        {
            // Fill top/bottom with black for letterboxing.
            HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = m_windowWidth;
            rect.bottom = yofs;
            FillRect(hdc, &rect, blackBrush);
            rect.top = m_windowHeight - yofs;
            rect.bottom = m_windowHeight;
            FillRect(hdc, &rect, blackBrush);
        }

        StretchDIBits(hdc, 0, yofs, destWidth, destHeight, 0, 0, m_width, m_height, m_pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        SetDIBitsToDevice(hdc, 0, 0, m_width, m_height, 0, 0, 0, m_height, m_pixels, &bitmapInfo, DIB_RGB_COLORS);
    }
    ReleaseDC((HWND)m_window, hdc);

    return true;
}

void Window::PlatformClose()
{
    DestroyWindow((HWND)m_window);
}

WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam)
{
    WPARAM newVk = vk;
    UINT scancode = (lParam & 0x00ff0000) >> 16;
    int extended = (lParam & 0x01000000) != 0;

    switch (vk)
    {
        case VK_SHIFT:
        {
            newVk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
            break;
        }

        case VK_CONTROL:
        {
            newVk = extended ? VK_RCONTROL : VK_LCONTROL;
            break;
        }

        case VK_MENU:
        {
            newVk = extended ? VK_RMENU : VK_LMENU;
            break;
        }

        default:
        {
            // Not a key we map from generic to left/right specialized.
            newVk = vk;
            break;
        }
    }

    return newVk;
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
                if (wParam < MaxPlatformKeys)
                {
                    wParam = MapLeftRightKeys(wParam, lParam);
                    window->SetKeyDown((int)wParam, true);
                }
                break;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                if (wParam < MaxPlatformKeys)
                {
                    wParam = MapLeftRightKeys(wParam, lParam);
                    window->SetKeyDown((int)wParam, false);
                }
                break;
            }

            case WM_CHAR:
            {
                if (wParam < MaxPlatformKeys)
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

#if defined(PIXIE_NORMALISE_MAIN)
extern int main(int argc, char** argv);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
#endif
