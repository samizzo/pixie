#include <string.h>
#include <ctype.h>
#include "pixie.h"
#include <assert.h>

using namespace Pixie;

Window::Window()
{
    m_delta = 0.0f;
    m_pixels = 0;
    m_scale = 1;

    assert(sizeof(m_mouseButtonDown) == sizeof(m_lastMouseButtonDown));
    memset(m_mouseButtonDown, 0, sizeof(m_mouseButtonDown));
    memset(m_lastMouseButtonDown, 0, sizeof(m_lastMouseButtonDown));

    memset(m_inputCharacters, 0, sizeof(m_inputCharacters));
    assert(sizeof(m_keyDown) == sizeof(m_lastKeyDown));
    memset(m_lastKeyDown, 0, sizeof(m_lastKeyDown));
    memset(m_keyDown, 0, sizeof(m_keyDown));

    PlatformInit();
}

Window::~Window()
{
    delete[] m_pixels;
}

bool Window::Open(const char* title, int width, int height, bool fullscreen, bool maintainAspectRatio /*= false*/, int scale /*= 1*/)
{
    // Create the buffer first because on OSX we need it to exist when initialising.
    m_pixels = new uint32_t[width * height];
    m_width = width;
    m_height = height;
    m_scale = scale;
    m_time = 0.0f;
    m_fullscreen = fullscreen;
    m_maintainAspectRatio = maintainAspectRatio;

    if (!PlatformOpen(title, width, height))
    {
        delete[] m_pixels;
        m_pixels = 0;
        return false;
    }

    return true;
}

bool Window::Update()
{
    UpdateMouse();
    UpdateKeyboard();
    bool result = PlatformUpdate();
    m_time += m_delta;
    return result;
}

void Window::Close()
{
    PlatformClose();
}

void Window::UpdateMouse()
{
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

