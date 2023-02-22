Pixie
=====

![msbuild](https://github.com/samizzo/pixie/actions/workflows/msbuild.yml/badge.svg)
![macOS](https://github.com/samizzo/pixie/actions/workflows/macos.yml/badge.svg)

Pixie is a minimal, cross-platform pixel framebuffer library for Windows and macOS.

![example.gif](/example.gif)

### Quick Start

Copy the following files into your project:

    pixie.cpp
    pixie.h
    core.h
    Windows: pixie_win.cpp
    macOS: pixie_osx.cpp

To use Pixie:

```cpp
#include "pixie.h"

int main(int argc, char** argv)
{
    Pixie::Window window;
    if (!window.open("Hello, World!", 640, 480))
        return 0;

    while (!window.HasKeyGoneUp(Pixie::Key_Escape))
    {
        uint32_t* pixels = window.GetPixels();
        // ..draw pixels!

        if (!window.Update())
            break;
    }

    window.Close();
}
```

The pixel byte order is BGRx (from LSB to MSB). On macOS, the pixel buffer is transformed
from BGR to RGB.

### Examples

Pixie has an example program in `main.cpp`. It can be compiled for Windows with the
Visual Studio 2017 project, or for macOS with `makefile_osx` as follows:

    make -f makefile_osx

This will generate an executable `pixie_demo`.

On macOS Pixie requires the `CoreGraphics` and `AppKit` frameworks.

### API

Pixie has some basic keyboard and mouse handling. You can check for:

* Mouse or key down in the current frame: `HasMouseGoneDown`, `HasKeyGoneDown`, `HasAnyKeyGoneDown`
* Mouse or key up in the current frame: `HasMouseGoneUp`, `HasKeyGoneUp`
* Mouse or key currently down: `IsMouseDown`, `IsKeyDown`, `IsAnyKeyDown`

The mouse position (in window coordinates) can be obtained with `GetMouseX` and `GetMouseY`.

Additionally the current time delta in seconds can be obtained with `GetDelta`.

### ImGui

Pixie has a basic ImGui with support for:

* Labels
* Buttons
* Input fields
* Check boxes
* Radio boxes
* Drawing rectangles and filled rectangles

To use the ImGui, add the following files to your project:

    imgui.cpp
    imgui.h
    font.h

and ensure that `font.bmp` is in your working directory.

To load the font:

```cpp
Pixie::Font font;
const int FontWidth = 9;
const int FontHeight = 16;
if (!font.Load("font.bmp", FontWidth, FontHeight))
    return 0;
```

In your main loop:

```cpp
while (!window.HasKeyGoneUp(Pixie::Key_Escape))
{
    Pixie::ImGui::Begin(&window, &font);

    if (Pixie::ImGui::Button("Hello", 100, 100, 100, 30))
        printf("Hello button was pressed\n");

    Pixie::ImGui::End();

    if (!window.Update())
        break;
}
```

### License

Pixie is licensed under the MIT License. See LICENSE for more information.
