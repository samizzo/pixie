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

	while (true)
	{
		if (!pixie.Update())
			break;
	}

	printf("done");
}
