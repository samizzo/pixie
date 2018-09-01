#include "pixie.h"
#include "font.h"
#include <stdio.h>

static const char* WindowTitle = "Hello, World!";
static const int WindowWidth = 640;
static const int WindowHeight = 400;

static void draw(int x, int y, uint32* buffer)
{
	for (int i = x; i < x+4; i++)
	{
		for (int j = y; j < y+4; j++)
		{
			if (i < WindowWidth && j < WindowHeight)
			{
				int index = (i + (j * WindowWidth));
				buffer[index] = 0xff;
			}
		}
	}
}

int main(int argc, char** argv)
{
	Font font;
	if (!font.Load("font.bmp", 9, 16))
		return 0;

	Pixie pixie;
	if (!pixie.Open(WindowTitle, WindowWidth, WindowHeight))
		return 0;

	uint32* buffer = new uint32[WindowWidth * WindowHeight];
	const float SPEED = 100.0f;
	float x = 0, y = 0;
	float xadd = SPEED, yadd = SPEED;

	while (true)
	{
		float delta = pixie.GetDelta();
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

		memset(buffer, 0, WindowWidth * WindowHeight * sizeof(uint32));

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
			font.Draw(buf, cx, cy, buffer, WindowWidth, WindowHeight);
			cx += 9;
		}

		draw((int)x, (int)y, buffer);

		if (!pixie.Update(buffer))
			break;
	}

	delete[] buffer;
	pixie.Close();

	printf("done");
}
