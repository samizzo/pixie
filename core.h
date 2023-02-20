#pragma once

// Define PIXIE_NORMALISE_MAIN (either here or in your compiler command-line
// arguments) to allow Pixie to normalise the program entry point regardless
// of platform and console/GUI subsystem to:
//
//           int main(int argc, char** argv)
//
//#define PIXIE_NORMALISE_MAIN

#ifdef _WIN32
#define PIXIE_PLATFORM_WIN 1
#elif __APPLE__
#define PIXIE_PLATFORM_OSX 1
#else
#error "Unsupported platform"
#endif

#if PIXIE_PLATFORM_OSX
#define strcat_s(dst, size, src) strlcat(dst, src, size)
#define sprintf_s(dst, size, fmt, ...) snprintf(dst, size, fmt, __VA_ARGS__)
#define strcpy_s(dst, size, src) snprintf(dst, size, "%s", src)
#endif

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))
