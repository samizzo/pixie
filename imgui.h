#pragma once

#include <stdint.h>
#include "core.h"

void Pixie_ImGuiBegin(PixieWindow* window, PixieFont* font);
void Pixie_ImGuiEnd();

// UI widgets
void Pixie_ImGuiLabel(const char* text, int x, int y, uint32_t colour);
char Pixie_ImGuiButton(const char* label, int x, int y, int width, int height);
void Pixie_ImGuiInput(char* text, int textBufferLength, int x, int y, int width, int height);
char Pixie_ImGuiCheckbox(const char* label, char checked, int x, int y);
char Pixie_ImGuiRadioButton(const char* label, char checked, int x, int y);

// Basic drawing
void Pixie_ImGuiRect(int x, int y, int width, int height, uint32_t borderColour);
void Pixie_ImGuiFilledRect(int x, int y, int width, int height, uint32_t colour, uint32_t borderColour);
