#include "pixie.h"
#include <stdio.h>

static const char* WindowTitle = "Hello, World!";
static const int WindowWidth = 320;
static const int WindowHeight = 200;

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
	Pixie pixie;
	if (!pixie.Open(WindowTitle, WindowWidth, WindowHeight))
		return 0;

	uint32* buffer = new uint32[WindowWidth * WindowHeight];

	while (true)
	{
		//memset(buffer, 0, WindowWidth*WindowHeight*4);
		if (pixie.IsMouseOverWindow())
		{
			int mouseX = pixie.GetMouseX();
			int mouseY = pixie.GetMouseY();
			draw(mouseX, mouseY, buffer);
		}

		if (!pixie.Update(buffer))
			break;
	}

	delete[] buffer;
	pixie.Close();

	printf("done");
}
