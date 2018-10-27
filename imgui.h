#pragma once

#include <stdint.h>
#include "core.h"

namespace Pixie
{
    class Window;
    class Font;

    class ImGui
    {
        public:
            static void Begin(Window* window, Font* font);
            static void End();

            // UI widgets
            static void Label(const char* text, int x, int y, uint32_t colour);
            static bool Button(const char* label, int x, int y, int width, int height);
            static void Input(char* text, int textBufferLength, int x, int y, int width, int height);
            static bool Checkbox(const char* label, bool checked, int x, int y);
            static bool RadioButton(const char* label, bool checked, int x, int y);

            // Basic drawing
            static void Rect(int x, int y, int width, int height, uint32_t borderColour);
            static void FilledRect(int x, int y, int width, int height, uint32_t colour, uint32_t borderColour);
    };
}
