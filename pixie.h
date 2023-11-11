#pragma once

#include <stdint.h>
#include <assert.h>
#include "core.h"

// Public interface.

typedef enum _PixieMouseButton
{
    PixieMouseButton_Left	= 0,
    PixieMouseButton_Middle	= 1,
    PixieMouseButton_Right	= 2,
    PixieMouseButton_Num
} PixieMouseButton;

typedef enum _PixieKey
{
    PixieKey_Backspace = 0,
    PixieKey_Tab,
    PixieKey_Enter,
    PixieKey_Escape,

    PixieKey_Up,
    PixieKey_Down,
    PixieKey_Left,
    PixieKey_Right,

    PixieKey_Home,
    PixieKey_End,
    PixieKey_PageUp,
    PixieKey_PageDown,

    PixieKey_Delete,
    PixieKey_Insert,

    PixieKey_LeftShift,
    PixieKey_RightShift,
    PixieKey_LeftControl,
    PixieKey_RightControl,
    PixieKey_LeftAlt,
    PixieKey_RightAlt,

    PixieKey_F1,
    PixieKey_F2,
    PixieKey_F3,
    PixieKey_F4,
    PixieKey_F5,
    PixieKey_F6,
    PixieKey_F7,
    PixieKey_F8,
    PixieKey_F9,
    PixieKey_F10,
    PixieKey_F11,
    PixieKey_F12,

    PixieKey_ASCII_Start = 32,

    // 32 to 127 are ASCII printable characters.
    // Note: these are unshifted keys.

    PixieKey_ASCII_End = 127,

    PixieKey_Num
} PixieKey;

enum
{
    MaxPlatformKeys = 256
};

typedef struct _PixieWindow PixieWindow;
typedef void(*KeyCallback)(PixieWindow* window, PixieKey key, char down);

typedef struct _PixieWindow
{
    int mousex;
    int mousey;
    char lastMouseButtonDown[PixieMouseButton_Num];
    char mouseButtonDown[PixieMouseButton_Num];

    int keyMap[PixieKey_Num];
    char lastKeyDown[MaxPlatformKeys];
    char keyDown[MaxPlatformKeys];
    char anyKeyDown;
    char inputCharacters[16 + 1];

    float delta;

    uint32_t* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t windowWidth;
    uint32_t windowHeight;
    int scale;
    char fullscreen;
    char maintainAspectRatio;
    float scalex, scaley;

    float time;
    int64_t lastTime;
    int64_t freq;

    KeyCallback keyCallback;
    void* handle;
} PixieWindow;

// Open a Pixie window with the specified title bar, width, and height.
// If scale is greater than 1 the window will be rendered scale times larger
// and the buffer will be stretched to fit.
PixieWindow* Pixie_Open(const TCHAR* title, int width, int height, char fullscreen);
PixieWindow* Pixie_OpenEx(const TCHAR* title, int width, int height, char fullscreen, char maintainAspectRatio, int scale);

// Close a Pixie window.
void Pixie_Close(PixieWindow* window);

// Update a Pixie window. This will copy the backing buffer to the actual window.
char Pixie_Update(PixieWindow* window);

// Returns true in the frame the mouse button went down.
char Pixie_HasMouseGoneDown(const PixieWindow* window, PixieMouseButton button);

// Returns true in the frame the mouse button went up.
char Pixie_HasMouseGoneUp(const PixieWindow* window, PixieMouseButton button);

// Returns true if the mouse button is currently down.
char Pixie_IsMouseDown(const PixieWindow* window, PixieMouseButton button);

// Returns true in the frame the key specified went down.
char Pixie_HasKeyGoneDown(const PixieWindow* window, PixieKey key);

// Returns true if any key went down in the frame.
char Pixie_HasAnyKeyGoneDown(const PixieWindow* window);

// Returns true in the frame the key specified went up.
char Pixie_HasKeyGoneUp(const PixieWindow* window, PixieKey key);

// Returns true if the key specified is currently down.
char Pixie_IsKeyDown(const PixieWindow* window, PixieKey key);

// Returns true if any key is currently down.
char Pixie_IsAnyKeyDown(const PixieWindow* window);

// Returns an array of ASCII input for the current frame.
const char* Pixie_GetInputCharacters(const PixieWindow* window);

// Clears the ASCII input for the current frame.
void Pixie_ClearInputCharacters(PixieWindow* window);

// Key callback handler. Called on any key state change.
void Pixie_SetKeyCallback(PixieWindow* window, KeyCallback callback);

inline char Pixie_HasMouseGoneDown(const PixieWindow* window, PixieMouseButton button)
{
    assert(window);
    return !window->lastMouseButtonDown[button] && window->mouseButtonDown[button];
}

inline char Pixie_HasMouseGoneUp(const PixieWindow* window, PixieMouseButton button)
{
    assert(window);
    return window->lastMouseButtonDown[button] && !window->mouseButtonDown[button];
}

inline char Pixie_IsMouseDown(const PixieWindow* window, PixieMouseButton button)
{
    assert(window);
    return window->mouseButtonDown[button];
}

inline char Pixie_HasAnyKeyGoneDown(const PixieWindow* window)
{
    assert(window);
    for (int i = 0; i < PixieKey_Num; i++)
    {
        if (Pixie_HasKeyGoneDown(window, (PixieKey)i))
            return 1;
    }

    return 0;
}

inline char Pixie_HasKeyGoneDown(const PixieWindow* window, PixieKey key)
{
    assert(window);
    int index = window->keyMap[key];
    if (index == -1)
        return 0;
    assert(index >= 0 && index < MaxPlatformKeys);
    return !window->lastKeyDown[index] && window->keyDown[index];
}

inline char Pixie_HasKeyGoneUp(const PixieWindow* window, PixieKey key)
{
    assert(window);
    int index = window->keyMap[key];
    if (index == -1)
        return 0;
    assert(index >= 0 && index < MaxPlatformKeys);
    return window->lastKeyDown[index] && !window->keyDown[index];
}

inline char Pixie_IsKeyDown(const PixieWindow* window, PixieKey key)
{
    assert(window);
    int index = window->keyMap[key];
    if (index == -1)
        return 0;
    assert(index >= 0 && index < MaxPlatformKeys);
    return window->keyDown[index];
}

inline char Pixie_IsAnyKeyDown(const PixieWindow* window)
{
    assert(window);
    for (int i = 0; i < PixieKey_Num; i++)
    {
        if (Pixie_IsKeyDown(window, (PixieKey)i))
            return 1;
    }

    return 0;
}

inline void Pixie_ClearInputCharacters(PixieWindow* window)
{
    assert(window);
    window->inputCharacters[0] = 0;
}

// --------------------------------------------------------------------------

// Internal implementation details.
void Pixie_AddInputCharacter(PixieWindow* window, char c);

inline void Pixie_SetKeyDown(PixieWindow* window, int platformKey, char down)
{
    assert(window);
    assert(platformKey >= 0 && platformKey < MaxPlatformKeys);
    if (window->keyDown[platformKey] == down)
        return;

    window->keyDown[platformKey] = down;
    if (window->keyCallback)
    {
        for (int i = 0; i < PixieKey_Num; i++)
        {
            int key = window->keyMap[i];
            if (key == platformKey)
            {
                window->keyCallback(window, (PixieKey)i, down);
                return;
            }
        }
    }
}
