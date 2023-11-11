﻿#include <assert.h>
#include "imgui.h"
#include "pixie.h"
#include "font.h"

enum Flags
{
    Flags_Started = 1 << 0,
};

typedef struct _State
{
    int flags;
    int nextId;
    int hoverId;
    int focusId;
    int keyboardCursorPosition;
    float keyRepeatTimer;
    float keyRepeatTime;
    float cursorBlinkTimer;
    uint32_t defaultTextColour;

    PixieWindow* window;
    PixieFont* font;
} State;

static State s_state = { 0 };

char State_HasStarted() { return (s_state.flags & Flags_Started) != 0; }
int State_GetNextId() { return s_state.nextId++; }

void Pixie_ImGuiBegin(PixieWindow* window, PixieFont* font)
{
    assert(window);
    assert(font);

    s_state.flags = Flags_Started;
    s_state.nextId = 1;
    s_state.hoverId = 0;
    s_state.window = window;
    s_state.font = font;
    s_state.defaultTextColour = MAKE_RGB(200, 200, 200);
}

void Pixie_ImGuiEnd()
{
    s_state.flags = 0;

    if (Pixie_HasMouseGoneDown(s_state.window, PixieMouseButton_Left))
    {
        // If mouse has gone down over empty space, clear the current focus.
        if (s_state.hoverId == 0)
        {
            s_state.focusId = 0;
            s_state.keyboardCursorPosition = 0;
        }
    }

    s_state.window = 0;
}

void Pixie_ImGuiLabel(const char* text, int x, int y, uint32_t colour)
{
    assert(text);
    assert(State_HasStarted());
    Pixie_FontDrawColour(s_state.font, text, x, y, colour, s_state.window);
}

char Pixie_ImGuiButton(const char* label, int x, int y, int width, int height)
{
    assert(State_HasStarted());

    PixieWindow* window = s_state.window;
    int id = State_GetNextId();

    const uint32_t NormalColour = MAKE_RGB(32, 50, 77);
    const uint32_t HoverColour = MAKE_RGB(39, 73, 114);
    const uint32_t PressedColour = MAKE_RGB(22, 40, 67);
    const uint32_t BorderColour = MAKE_RGB(68, 79, 103);
    const uint32_t FocusBorderColour = MAKE_RGB(200, 200, 229);

    int mouseX = window->mousex;
    int mouseY = window->mousey;

    char hover = mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
    char pressed = 0;

    if (hover)
    {
        s_state.hoverId = id;

        // Mouse has just gone down over this element, so give it focus.
        if (Pixie_HasMouseGoneDown(window, PixieMouseButton_Left))
            s_state.focusId = id;

        // If mouse is still down over this element and it has focus, then it is pressed.
        pressed = Pixie_IsMouseDown(window, PixieMouseButton_Left) && s_state.focusId == id;
    }

    uint32_t buttonColour = pressed ? PressedColour : hover ? HoverColour : NormalColour;
    uint32_t borderColour = s_state.focusId == id ? FocusBorderColour : BorderColour;

    Pixie_ImGuiFilledRect(x, y, width, height, buttonColour, borderColour);

    if (label)
    {
        PixieFont* font = s_state.font;
        int textX = x + ((width - Pixie_FontGetStringWidth(font, label)) >> 1);
        int textY = y + ((height - font->characterSizeY) >> 1);

        Pixie_ImGuiLabel(label, textX, textY, s_state.defaultTextColour);
    }

    return hover && s_state.focusId == id && Pixie_HasMouseGoneUp(window, PixieMouseButton_Left);
}

void Pixie_ImGuiInput(char* text, int textBufferLength, int x, int y, int width, int height)
{
    assert(text);
    assert(State_HasStarted());

    PixieWindow* window = s_state.window;
    PixieFont* font = s_state.font;
    int id = State_GetNextId();

    const int LeftMargin = 8;
    const uint32_t NormalColour = MAKE_RGB(64, 68, 71);
    const uint32_t HoverColour = MAKE_RGB(74, 78, 81);
    const uint32_t BorderColour = MAKE_RGB(104, 108, 111);
    const uint32_t FocusBorderColour = MAKE_RGB(200, 200, 200);
    const uint32_t CursorColour = MAKE_RGB(220, 220, 220);
    const int CursorWidth = 8;
    const float KeyRepeatTimeInit = 0.2f;
    const float KeyRepeatTimeRepeat = 0.05f;
    const float CursorBlinkTime = 1.0f;

    int mouseX = window->mousex;
    int mouseY = window->mousey;

    char hover = mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height;
    char pressed = 0;
    int textLength = (int)strlen(text);

    int textX = x + LeftMargin;

    if (hover)
    {
        s_state.hoverId = id;

        // Mouse has just gone down over this element, so give it focus.
        if (Pixie_HasMouseGoneDown(window, PixieMouseButton_Left))
        {
            if (s_state.focusId != id)
            {
                s_state.keyRepeatTimer = 0.0f;
                s_state.cursorBlinkTimer = 0.0f;
                s_state.focusId = id;
            }

            // Move the cursor to whereever the user clicked.
            s_state.keyboardCursorPosition = min((mouseX - textX) / font->characterSizeX, textLength);

            // Also force the cursor to be visible.
            s_state.cursorBlinkTimer = CursorBlinkTime;
        }

        // If mouse is still down over this element and it has focus, then it is pressed.
        pressed = Pixie_IsMouseDown(window, PixieMouseButton_Left) && s_state.focusId == id;
    }

    uint32_t boxColour = pressed || hover || s_state.focusId == id ? HoverColour : NormalColour;
    uint32_t borderColour = s_state.focusId == id ? FocusBorderColour : BorderColour;

    // Draw the input field.
    Pixie_ImGuiFilledRect(x, y, width, height, boxColour, borderColour);

    int textY = y + ((height - font->characterSizeY) >> 1);
    Pixie_ImGuiLabel(text, textX, textY, s_state.defaultTextColour);

    if (s_state.focusId == id)
    {
        float delta = window->delta;

        // Input field has focus, draw the keyboard cursor and process input.
        s_state.cursorBlinkTimer -= delta;
        if (s_state.cursorBlinkTimer >= CursorBlinkTime * 0.5f)
            Pixie_ImGuiFilledRect(textX + (s_state.keyboardCursorPosition * font->characterSizeX), textY + font->characterSizeY - 2, CursorWidth, 2, CursorColour, CursorColour);
        if (s_state.cursorBlinkTimer <= 0.0f || Pixie_IsAnyKeyDown(window))
            s_state.cursorBlinkTimer = CursorBlinkTime;

        // TODO: Not sure if this is the right way to do this. The timer should probably be per-key.
        if (Pixie_HasAnyKeyGoneDown(window))
        {
            s_state.keyRepeatTimer = 0.0f;
            s_state.keyRepeatTime = KeyRepeatTimeInit;
        }

        s_state.keyRepeatTimer -= delta;
        if (s_state.keyRepeatTimer <= 0.0f)
        {
            s_state.keyRepeatTimer = s_state.keyRepeatTime;
            s_state.keyRepeatTime = KeyRepeatTimeRepeat;

            if (Pixie_IsKeyDown(window, PixieKey_Left))
            {
                s_state.keyboardCursorPosition = max(s_state.keyboardCursorPosition - 1, 0);
            }
            else if (Pixie_IsKeyDown(window, PixieKey_Right))
            {
                s_state.keyboardCursorPosition = min(s_state.keyboardCursorPosition + 1, textLength);
            }
            else if (Pixie_IsKeyDown(window, PixieKey_Backspace))
            {
                // Move cursor back.
                s_state.keyboardCursorPosition--;

                if (s_state.keyboardCursorPosition >= 0)
                {
                    // Copy everything after the current position to the current position.
                    int position = s_state.keyboardCursorPosition;
                    int copyAmount = (int)strlen(text + position + 1) + 1;
                    memcpy(text + position, text + position + 1, copyAmount);
                }
                else
                {
                    s_state.keyboardCursorPosition = 0;
                }
            }
            else if (Pixie_IsKeyDown(window, PixieKey_Delete))
            {
                // Delete the current character.
                int position = s_state.keyboardCursorPosition;
                if (position < textLength)
                {
                    // Copy everything after the current position to the current position.
                    int copyAmount = (int)strlen(text + position + 1) + 1;
                    memcpy(text + position, text + position + 1, copyAmount);
                }
            }
            else if (Pixie_IsKeyDown(window, PixieKey_Home))
            {
                s_state.keyboardCursorPosition = 0;
            }
            else if (Pixie_IsKeyDown(window, PixieKey_End))
            {
                s_state.keyboardCursorPosition = textLength;
            }
        }

        // Process remaining ASCII input.
        const char* inputCharacters = window->inputCharacters;
        if (*inputCharacters)
        {
            for ( ; *inputCharacters; inputCharacters++)
            {
                if (s_state.keyboardCursorPosition == textBufferLength - 1)
                    break;

                // Overwrite the character at the current position.
                int position = s_state.keyboardCursorPosition;
                assert(position >= 0 && position < textBufferLength - 1);
                text[position] = *inputCharacters;

                // If at the end of the string, add a null because we've just extended the string.
                if (position == textLength)
                {
                    position++;
                    assert(position >= 0 && position < textBufferLength);
                    text[position] = 0;
                }

                // Move the cursor.
                s_state.keyboardCursorPosition = min(s_state.keyboardCursorPosition + 1, textBufferLength);
            }

            // We have consumed the input so remove it from the buffer.
            Pixie_ClearInputCharacters(window);
        }
    }
}

char Pixie_ImGuiCheckbox(const char* label, char checked, int x, int y)
{
    assert(label);
    assert(State_HasStarted());

    const int TextLeftMargin = 8;
    const int BoxSize = 18;
    const int CheckSize = 8;

    PixieFont* font = s_state.font;
    int charHeight = font->characterSizeY;

    int textY = y + ((BoxSize - charHeight) >> 1) + 1;
    Pixie_ImGuiLabel(label, x + BoxSize + TextLeftMargin, textY, s_state.defaultTextColour);
    char wasChecked = checked;
    if (Pixie_ImGuiButton(0, x, y, BoxSize, BoxSize))
        checked = !checked;

    if (wasChecked)
    {
        // Draw check mark.
        int checkX = x + ((BoxSize - CheckSize) >> 1);
        int checkY = y + ((BoxSize - CheckSize) >> 1);
        PixieWindow* window = s_state.window;
        int windowWidth = window->width;
        uint32_t* pixels = window->pixels;
        y = checkY;
        for (int yy = y*windowWidth, x = 0; y < checkY + CheckSize; y++, x++, yy += windowWidth)
        {
            pixels[checkX + x + yy] = MAKE_RGB(255, 255, 255);
            pixels[checkX + CheckSize - x - 1 + yy] = MAKE_RGB(255, 255, 255);
        }
    }

    return checked;
}

char Pixie_ImGuiRadioButton(const char* label, char checked, int x, int y)
{
    assert(label);
    assert(State_HasStarted());

    const int TextLeftMargin = 8;
    const int BoxSize = 18;
    const int CheckSize = 8;
    const uint32_t FontColour = s_state.defaultTextColour;

    PixieFont* font = s_state.font;
    int charHeight = font->characterSizeY;

    int textY = y + ((BoxSize - charHeight) >> 1) + 1;
    Pixie_ImGuiLabel(label, x + BoxSize + TextLeftMargin, textY, FontColour);
    char wasChecked = checked;
    if (Pixie_ImGuiButton(0, x, y, BoxSize, BoxSize))
        checked = !checked;

    if (wasChecked)
    {
        // Draw radio button mark.
        int checkX = x + ((BoxSize - CheckSize) >> 1);
        int checkY = y + ((BoxSize - CheckSize) >> 1);
        Pixie_ImGuiFilledRect(checkX, checkY, CheckSize, CheckSize, FontColour, FontColour);
    }

    return checked;
}

void Pixie_ImGuiRect(int x, int y, int width, int height, uint32_t borderColour)
{
    assert(State_HasStarted());
    PixieWindow* window = s_state.window;
    uint32_t* pixels = window->pixels;
    int windowWidth = window->width;
    int windowHeight = window->height;

    pixels += x + (y*windowWidth);

    for (int j = 0, ypos = y; j < height && ypos < windowHeight; j++, ypos++)
    {
        int left = x;
        if (left >= 0 && left < windowWidth)
            *pixels = borderColour;

        int right = x + width - 1;
        if (right >= 0 && right < windowWidth)
            *(pixels + width - 1) = borderColour;

        pixels += windowWidth;
    }

    pixels = window->pixels + x + (y*windowWidth);
    for (int i = 0, xpos = x; i < width; i++, xpos++)
    {
        int top = y;
        if (top >= 0 && top < windowHeight)
            *pixels = borderColour;

        int bottom = y + height - 1;
        if (bottom >= 0 && bottom < windowHeight)
            *(pixels + ((height-1)*windowWidth)) = borderColour;

        pixels++;
    }
}

void Pixie_ImGuiFilledRect(int x, int y, int width, int height, uint32_t colour, uint32_t borderColour)
{
    assert(State_HasStarted());
    PixieWindow* window = s_state.window;
    uint32_t* pixels = window->pixels;
    int windowWidth = window->width;
    int windowHeight = window->height;

    pixels += x + (y*windowWidth);

    for (int j = 0, ypos = y; j < height && ypos < windowHeight; j++, ypos++)
    {
        for (int i = 0, xpos = x; i < width; i++, xpos++)
        {
            if (xpos < 0 || xpos >= windowWidth || ypos < 0 || ypos >= windowHeight)
                continue;
            *pixels = (i == 0 || i == width - 1 || j == 0 || j == height - 1) ? borderColour : colour;
            pixels++;
        }

        pixels += windowWidth - width;
    }
}

