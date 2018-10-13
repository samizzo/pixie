#include "pixie.h"
#include "font.h"
#include "imgui.h"
#include <string.h>
#include <stdio.h>

static const char* WindowTitle = "Hello, World!";
static const int WindowWidth = 640;
static const int WindowHeight = 400;

static void draw(int x, int y, uint32_t* pixels)
{
    for (int i = x; i < x+4; i++)
    {
        for (int j = y; j < y+4; j++)
        {
            if (i < WindowWidth && j < WindowHeight)
            {
                int index = (i + (j * WindowWidth));
                pixels[index] = MAKE_RGB(0, 0, 255);
            }
        }
    }
}

int main(int argc, char** argv)
{
    Pixie::Font font;
    if (!font.Load("font.bmp", 9, 16))
        return 0;

    Pixie::Window window;
    if (!window.Open(WindowTitle, WindowWidth, WindowHeight))
        return 0;

    uint32_t* pixels = window.GetPixels();

    const float SPEED = 100.0f;
    float x = 0, y = 0;
    float xadd = SPEED, yadd = SPEED;
    char buf[16] = { 0 };
    strcat_s(buf, sizeof(buf), "Hello, World!");

    while (!window.HasKeyGoneUp(Pixie::Key_Escape))
    {
        Pixie::ImGui::Begin(&window, &font);

        float delta = window.GetDelta();

        x += xadd*delta;
        y += yadd*delta;
        if (x >= WindowWidth - 1)
        {
            x = WindowWidth - 1;
            xadd = -SPEED;
        }
        else if (x < 0)
        {
            x = 0;
            xadd = SPEED;
        }

        if (y >= WindowHeight - 1)
        {
            y = WindowHeight - 1;
            yadd = -SPEED;
        }
        else if (y < 0)
        {
            y = 0;
            yadd = SPEED;
        }

        memset(pixels, 0, WindowWidth * WindowHeight * sizeof(uint32_t));

        int cx = 0, cy = 0;
        for (int i = 0; i < 256; i++)
        {
            char buf[128];
            sprintf_s(buf, sizeof(buf), "%c", i);
            if (cx >= WindowWidth-9)
            {
                cx = 0;
                cy += 16;
            }
            font.Draw(buf, cx, cy, &window);
            cx += 9;
        }

        {
            char buf[128];
            sprintf_s(buf, sizeof(buf), "%.4f", window.GetTime());
            font.Draw(buf, 10, 90, &window);
        }

        draw((int)x, (int)y, pixels);

        Pixie::ImGui::FilledRect(10, 240, 100, 100, MAKE_RGB(255, 0, 0), MAKE_RGB(128, 0, 0));
        Pixie::ImGui::FilledRoundedRect(120, 240, 100, 100, MAKE_RGB(0, 255, 0), MAKE_RGB(0, 128, 0));

        if (Pixie::ImGui::Button("Hello", 100, 100, 100, 30))
            strcpy_s(buf, sizeof(buf), "Hello, World!");
        if (Pixie::ImGui::Button("Goodbye", 100, 140, 100, 30))
            strcpy_s(buf, sizeof(buf), "Goodbye, World!");

        Pixie::ImGui::Input(buf, sizeof(buf), 100, 180, 400, 20);

        static bool checked = false;
        checked = Pixie::ImGui::Checkbox("Do the thing", checked, 100, 210);

        static int selection = 0;
        if (Pixie::ImGui::RadioButton("Banana", selection == 0, 300, 210))
            selection = 0;
        if (Pixie::ImGui::RadioButton("Apple", selection == 1, 300, 230))
            selection = 1;
        if (Pixie::ImGui::RadioButton("Pear", selection == 3, 300, 250))
            selection = 3;

        for (int i = 0; i < Pixie::MouseButton_Num; i++)
        {
            if (window.IsMouseDown((Pixie::MouseButton)i))
            {
                Pixie::ImGui::FilledRect((i*33) + 240, 280, 32, 32, MAKE_RGB(255, 0, 0), MAKE_RGB(255, 0, 0));
            }
            else
            {
                Pixie::ImGui::Rect((i*33) + 240, 280, 32, 32, MAKE_RGB(255, 0, 0));
            }
        }

        static float accumTime = 0.0f;
        static int numFrames = 0;
        static float avgFrameTime = 0.0f;
        numFrames++;
        accumTime += delta;
        if (numFrames == 8)
        {
            avgFrameTime = accumTime / (float)numFrames;
            numFrames = 0;
            accumTime = 0.0f;
        }

        int fpsWidth = (int)min(WindowWidth, (avgFrameTime*20.0f)*WindowWidth);
        Pixie::ImGui::FilledRect(0, 0, fpsWidth, 10, MAKE_RGB(255, 0, 0), MAKE_RGB(255, 0, 0));
        Pixie::ImGui::FilledRect((int)((1.0f/60.0f)*20.0f*WindowWidth), 0, 2, 10, MAKE_RGB(0, 255, 0), MAKE_RGB(0, 255, 0));

        Pixie::ImGui::End();

        if (!window.Update())
            break;
    }

    window.Close();

    printf("done\n");
}
