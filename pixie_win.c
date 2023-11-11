#include <assert.h>
#include <stdlib.h>
#include "pixie.h"

static const TCHAR* PixieWindowClass = TEXT("PixieWindowClass");
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

char Pixie_PlatformOpen(PixieWindow* window, const TCHAR* title)
{
    assert(window);
    assert(sizeof(HWND) == sizeof(void*));

    window->keyMap[PixieKey_Backspace] = VK_BACK;
    window->keyMap[PixieKey_Tab] = VK_TAB;
    window->keyMap[PixieKey_Enter] = VK_RETURN;
    window->keyMap[PixieKey_Escape] = VK_ESCAPE;
    window->keyMap[PixieKey_Up] = VK_UP;
    window->keyMap[PixieKey_Down] = VK_DOWN;
    window->keyMap[PixieKey_Left] = VK_LEFT;
    window->keyMap[PixieKey_Right] = VK_RIGHT;
    window->keyMap[PixieKey_Home] = VK_HOME;
    window->keyMap[PixieKey_End] = VK_END;
    window->keyMap[PixieKey_PageUp] = VK_PRIOR;
    window->keyMap[PixieKey_PageDown] = VK_NEXT;
    window->keyMap[PixieKey_Delete] = VK_DELETE;
    window->keyMap[PixieKey_Insert] = VK_INSERT;
    window->keyMap[PixieKey_LeftShift] = VK_LSHIFT;
    window->keyMap[PixieKey_RightShift] = VK_RSHIFT;
    window->keyMap[PixieKey_LeftControl] = VK_LCONTROL;
    window->keyMap[PixieKey_RightControl] = VK_RCONTROL;
    window->keyMap[PixieKey_LeftAlt] = VK_LMENU;
    window->keyMap[PixieKey_RightAlt] = VK_RMENU;
    window->keyMap[PixieKey_F1] = VK_F1;
    window->keyMap[PixieKey_F2] = VK_F2;
    window->keyMap[PixieKey_F3] = VK_F3;
    window->keyMap[PixieKey_F4] = VK_F4;
    window->keyMap[PixieKey_F5] = VK_F5;
    window->keyMap[PixieKey_F6] = VK_F6;
    window->keyMap[PixieKey_F7] = VK_F7;
    window->keyMap[PixieKey_F8] = VK_F8;
    window->keyMap[PixieKey_F9] = VK_F9;
    window->keyMap[PixieKey_F10] = VK_F10;
    window->keyMap[PixieKey_F11] = VK_F11;
    window->keyMap[PixieKey_F12] = VK_F12;
    window->keyMap['['] = VK_OEM_4;
    window->keyMap['\\'] = VK_OEM_5;
    window->keyMap[']'] = VK_OEM_6;
    window->keyMap['`'] = VK_OEM_3;
    window->keyMap['\''] = VK_OEM_7;
    //window->keyMap['='] = kVK_ANSI_Equal;
    window->keyMap['-'] = VK_OEM_MINUS;
    window->keyMap['.'] = VK_OEM_PERIOD;
    window->keyMap['/'] = VK_OEM_2;
    window->keyMap[';'] = VK_OEM_1;
    window->keyMap[','] = VK_OEM_COMMA;

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

    int style = 0;
    int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
    int desktopHeight = GetSystemMetrics(SM_CYSCREEN);
    int xPos, yPos;

    if (window->fullscreen)
    {
        style = WS_POPUP;
        xPos = yPos = 0;

        window->windowWidth = desktopWidth;
        window->windowHeight = desktopHeight;
        window->scalex = desktopWidth / (float)window->width;
        window->scaley = window->maintainAspectRatio ? window->scalex : (desktopHeight / (float)window->height);
    }
    else
    {
        style = WS_BORDER | WS_CAPTION;

        RECT rect;
        rect.left = 0;
        rect.right = window->width * window->scale;
        rect.top = 0;
        rect.bottom = window->height * window->scale;
        AdjustWindowRect(&rect, style, FALSE);

        xPos = (desktopWidth - rect.right) >> 1;
        yPos = (desktopHeight - rect.bottom) >> 1;

        window->windowWidth = rect.right - rect.left;
        window->windowHeight = rect.bottom - rect.top;
    }

    HWND handle = CreateWindow(PixieWindowClass, title, style, xPos, yPos, window->windowWidth, window->windowHeight, NULL, NULL, hInstance, NULL);
    window->handle = (HWND)handle;
    if (handle == 0)
        return 0;

    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)window);
    ShowWindow(handle, SW_SHOW);

    QueryPerformanceFrequency((LARGE_INTEGER*)&window->freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&window->lastTime);

    return 1;
}

char Pixie_PlatformUpdate(PixieWindow* window)
{
    assert(window);

    // Update mouse cursor location.
    POINT p;
    GetCursorPos(&p);
    ScreenToClient((HWND)window->handle, &p);
    window->mousex = p.x;
    window->mousey = p.y;

    if (window->fullscreen)
    {
        window->mousex = (int)(window->mousex / window->scalex);
        window->mousey = (int)(window->mousey / window->scaley);
    }
    else
    {
        window->mousex /= window->scale;
        window->mousey /= window->scale;
    }

    __int64 time;
    QueryPerformanceCounter((LARGE_INTEGER*)&time);
    __int64 delta = time - window->lastTime;
    window->delta = (delta / (float)window->freq);
    window->lastTime = time;

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT)
            return 0;
    }

    // Copy buffer to the window.
    HDC hdc = GetDC((HWND)window->handle);
    BITMAPINFO bitmapInfo;
    BITMAPINFOHEADER* bmiHeader = &bitmapInfo.bmiHeader;
    bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader->biWidth = window->width;
    bmiHeader->biHeight = -(int32_t)window->height; // Negative indicates a top-down DIB. Otherwise DIB is bottom up.
    bmiHeader->biPlanes = 1;
    bmiHeader->biBitCount = 32;
    bmiHeader->biCompression = BI_RGB;
    bmiHeader->biSizeImage = 0;
    bmiHeader->biXPelsPerMeter = 0;
    bmiHeader->biYPelsPerMeter = 0;
    bmiHeader->biClrUsed = 0;
    bmiHeader->biClrImportant = 0;
    if (window->scale > 1 || window->fullscreen)
    {
        int yofs = 0;
        int destWidth = (int)(window->width * window->scalex);
        int destHeight = (int)(window->height * window->scaley);
        if (window->maintainAspectRatio)
            yofs = (window->windowHeight - destHeight) >> 1;

        if (window->maintainAspectRatio && yofs > 0)
        {
            // Fill top/bottom with black for letterboxing.
            HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = window->windowWidth;
            rect.bottom = yofs;
            FillRect(hdc, &rect, blackBrush);
            rect.top = window->windowHeight - yofs;
            rect.bottom = window->windowHeight;
            FillRect(hdc, &rect, blackBrush);
        }

        StretchDIBits(hdc, 0, yofs, destWidth, destHeight, 0, 0, window->width, window->height, window->pixels, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
        SetDIBitsToDevice(hdc, 0, 0, window->width, window->height, 0, 0, 0, window->height, window->pixels, &bitmapInfo, DIB_RGB_COLORS);
    }
    ReleaseDC((HWND)window->handle, hdc);

    return 1;
}

void Pixie_PlatformClose(PixieWindow* window)
{
    assert(window);
    DestroyWindow((HWND)window->handle);
}

static WPARAM MapLeftRightKeys(WPARAM vk, LPARAM lParam)
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
    PixieWindow* window = (PixieWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (window)
    {
        switch (msg)
        {
            case WM_LBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_RBUTTONDOWN:
            {
                PixieMouseButton button = PixieMouseButton_Left;
                if (msg == WM_MBUTTONDOWN) button = PixieMouseButton_Middle;
                if (msg == WM_RBUTTONDOWN) button = PixieMouseButton_Right;
                window->mouseButtonDown[button] = 1;
                break;
            }

            case WM_LBUTTONUP:
            case WM_MBUTTONUP:
            case WM_RBUTTONUP:
            {
                PixieMouseButton button = PixieMouseButton_Left;
                if (msg == WM_MBUTTONUP) button = PixieMouseButton_Middle;
                if (msg == WM_RBUTTONUP) button = PixieMouseButton_Right;
                window->mouseButtonDown[button] = 0;
                break;
            }

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                if (wParam < MaxPlatformKeys)
                {
                    wParam = MapLeftRightKeys(wParam, lParam);
                    Pixie_SetKeyDown(window, (int)wParam, 1);
                }
                break;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                if (wParam < MaxPlatformKeys)
                {
                    wParam = MapLeftRightKeys(wParam, lParam);
                    Pixie_SetKeyDown(window, (int)wParam, 0);
                }
                break;
            }

            case WM_CHAR:
            {
                if (wParam < MaxPlatformKeys)
                    Pixie_AddInputCharacter(window, (char)wParam);
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
