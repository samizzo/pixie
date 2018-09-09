#pragma once

#include "types.h"

class PixelBuffer;
class Font;
class Pixie;

class ImGui
{
	public:
		static void Begin(Pixie* pixie, Font* font);
		static void End();
		static void Label(const char* text, int x, int y, uint32 colour);
		static bool Button(const char* text, int x, int y, int width, int height);
		static void Input(char* text, int textBufferLength, int x, int y, int width, int height);
		static bool Checkbox(const char* text, bool checked, int x, int y);
		static void FilledRect(int x, int y, int width, int height, uint32 colour, uint32 borderColour);
		static void FilledRoundedRect(int x, int y, int width, int height, uint32 colour, uint32 borderColour);
};
