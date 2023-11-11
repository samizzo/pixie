#include <assert.h>
#include <stdlib.h>
#include "pixie.h"

char Pixie_PlatformOpen(PixieWindow* window, const TCHAR* title);
char Pixie_PlatformUpdate(PixieWindow* window);
void Pixie_PlatformClose(PixieWindow* window);

static void Pixie_UpdateMouse(PixieWindow* window);
static void Pixie_UpdateKeyboard(PixieWindow* window);

PixieWindow* Pixie_Open(const TCHAR* title, int width, int height, char fullscreen)
{
    return Pixie_OpenEx(title, width, height, fullscreen, 0, 1);
}

PixieWindow* Pixie_OpenEx(const TCHAR* title, int width, int height, char fullscreen, char maintainAspectRatio, int scale)
{
    // Create the buffer first because on OSX we need it to exist when initialising.
    PixieWindow* window = malloc(sizeof(PixieWindow));
    memset(window, 0, sizeof(PixieWindow));
    window->pixels = malloc(sizeof(uint32_t) * width * height);
    window->width = width;
    window->height = height;
    window->scale = scale;
    window->scalex = (float)scale;
    window->scaley = (float)scale;
    window->time = 0.0f;
    window->fullscreen = fullscreen;
    window->maintainAspectRatio = maintainAspectRatio;
    window->keyCallback = NULL;
    window->delta = 0.0f;

    // Initialise ASCII entries in keymap.
    for (int i = 0; i < PixieKey_Num; i++)
        window->keyMap[i] = i >= PixieKey_ASCII_Start && i <= PixieKey_ASCII_End ? i : PixieKey_Num;

    if (!Pixie_PlatformOpen(window, title))
    {
        free(window->pixels);
        free(window);
        return 0;
    }

    return window;
}

char Pixie_Update(PixieWindow* window)
{
    assert(window);
    Pixie_UpdateMouse(window);
    Pixie_UpdateKeyboard(window);
    char result = Pixie_PlatformUpdate(window);
    window->time += window->delta;
    return result;
}

void Pixie_Close(PixieWindow* window)
{
    assert(window);
    Pixie_PlatformClose(window);
    free(window->pixels);
    free(window);
}

static void Pixie_UpdateMouse(PixieWindow* window)
{
    assert(window);
    memcpy(window->lastMouseButtonDown, window->mouseButtonDown, sizeof(window->mouseButtonDown));
}

static void Pixie_UpdateKeyboard(PixieWindow* window)
{
    assert(window);
    memset(window->inputCharacters, 0, sizeof(window->inputCharacters));
    memcpy(window->lastKeyDown, window->keyDown, sizeof(window->keyDown));
}

void Pixie_AddInputCharacter(PixieWindow* window, char c)
{
    assert(window);

    if (!isprint(c))
        return;

    size_t length = strlen(window->inputCharacters);
    if (length + 1 < sizeof(window->inputCharacters))
    {
        window->inputCharacters[length] = c;
        window->inputCharacters[length + 1] = 0;
    }
}

