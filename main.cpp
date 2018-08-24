#include "pixie.h"
#include <stdio.h>

static const char* WindowTitle = "Hello, World!";
static const int WindowWidth = 320;
static const int WindowHeight = 200;

int main(int argc, char** argv)
{
	Pixie pixie;
	if (!pixie.Open(WindowTitle, WindowWidth, WindowHeight))
		return 0;

	uint32* buffer = new uint32[WindowWidth * WindowHeight];

	while (true)
	{
		for (int y = 0; y < WindowHeight; y++)
		{
			for (int x = 0; x < WindowWidth; x++)
			{
				int index = (x + (y*WindowWidth));
				uint8 c = x & y;
				buffer[index] = MAKE_RGB(c, c, c);
			}
		}

		if (!pixie.Update(buffer))
			break;
	}

	pixie.Close();

	printf("done");
}
