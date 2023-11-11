#include <stdio.h>
#include "pixie.h"
#include "font.h"
#include "imgui.h"

static const TCHAR* WindowTitle = TEXT("Hello, World!");
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
    PixieFont* font = Pixie_FontLoad("font.bmp", 9, 16);
    if (!font)
    {
#if PIXIE_PLATFORM_WIN
        MessageBox(NULL, TEXT("Failed to load font.bmp"), TEXT("Pixie Error"), MB_ICONERROR);
#else
        printf("pixie: failed to load font.bmp\n");
#endif
        return 0;
    }

    PixieWindow* window = Pixie_Open(WindowTitle, WindowWidth, WindowHeight, 0);
    if (!window)
        return 0;

    uint32_t* pixels = window->pixels;

    const float SPEED = 100.0f;
    float x = 0, y = 0;
    float xadd = SPEED, yadd = SPEED;
    char buf[16] = { 0 };
    strcat_s(buf, sizeof(buf), "Hello, World!");

    while (!Pixie_HasKeyGoneUp(window, PixieKey_Escape))
    {
        Pixie_ImGuiBegin(window, font);

        float delta = window->delta;

        x += xadd*delta;
        y += yadd*delta;
        if (x >= WindowWidth - 1)
        {
            x = (float)(WindowWidth - 1);
            xadd = -SPEED;
        }
        else if (x < 0)
        {
            x = 0;
            xadd = SPEED;
        }

        if (y >= WindowHeight - 1)
        {
            y = (float)(WindowHeight - 1);
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
            Pixie_FontDraw(font, buf, cx, cy, window);
            cx += 9;
        }

        {
            char buf[128];
            sprintf_s(buf, sizeof(buf), "%.4f", window->time);
            Pixie_FontDraw(font, buf, 10, 90, window);
        }

        draw((int)x, (int)y, pixels);

        Pixie_ImGuiFilledRect(10, 240, 100, 100, MAKE_RGB(255, 0, 0), MAKE_RGB(128, 0, 0));

        if (Pixie_ImGuiButton("Hello", 100, 100, 100, 30))
            strcpy_s(buf, sizeof(buf), "Hello, World!");
        if (Pixie_ImGuiButton("Goodbye", 100, 140, 100, 30))
            strcpy_s(buf, sizeof(buf), "Goodbye, World!");

        Pixie_ImGuiInput(buf, sizeof(buf), 100, 180, 400, 20);

        static char checked = 0;
        checked = Pixie_ImGuiCheckbox("Do the thing", checked, 100, 210);

        static int selection = 0;
        if (Pixie_ImGuiRadioButton("Banana", selection == 0, 300, 210))
            selection = 0;
        if (Pixie_ImGuiRadioButton("Apple", selection == 1, 300, 230))
            selection = 1;
        if (Pixie_ImGuiRadioButton("Pear", selection == 3, 300, 250))
            selection = 3;

        for (int i = 0; i < PixieMouseButton_Num; i++)
        {
            if (Pixie_IsMouseDown(window, (PixieMouseButton)i))
            {
                Pixie_ImGuiFilledRect((i*33) + 240, 280, 32, 32, MAKE_RGB(255, 0, 0), MAKE_RGB(255, 0, 0));
            }
            else
            {
                Pixie_ImGuiRect((i*33) + 240, 280, 32, 32, MAKE_RGB(255, 0, 0));
            }
        }

        static float accumTime = 0.0f;
        static int numFrames = 0;
        static float avgFrameTime = 0.0f;
        numFrames++;
        accumTime += delta;
        if (numFrames == 16)
        {
            avgFrameTime = accumTime / (float)numFrames;
            numFrames = 0;
            accumTime = 0.0f;
        }

        int fpsWidth = min(WindowWidth, (int)((avgFrameTime*20.0f)*WindowWidth));
        Pixie_ImGuiFilledRect(0, 0, fpsWidth, 10, MAKE_RGB(255, 0, 0), MAKE_RGB(255, 0, 0));
        Pixie_ImGuiFilledRect((int)((1.0f/60.0f)*20.0f*WindowWidth), 0, 2, 10, MAKE_RGB(0, 255, 0), MAKE_RGB(0, 255, 0));

        {
            char buf[128];
            sprintf_s(buf, sizeof(buf), "%.2f fps", 1.0f / avgFrameTime);
            Pixie_FontDraw(font, buf, 10, 106, window);
        }

        Pixie_ImGuiEnd();

        if (!Pixie_Update(window))
            break;
    }

    Pixie_Close(window);

    printf("done\n");
}
