#pragma once

#ifdef _WIN32
#define PIXIE_PLATFORM_WIN 1
#elif __APPLE__
#define PIXIE_PLATFORM_OSX 1
#else
#error "Unsupported platform"
#endif

#if PIXIE_PLATFORM_WIN
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdint>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(x, a, b) min(max((x), (a)), (b))
#endif

#if PIXIE_PLATFORM_OSX
#define strcat_s(dst, size, src) strlcat(dst, src, size)
#define sprintf_s(dst, size, fmt, ...) snprintf(dst, size, fmt, __VA_ARGS__)
#define strcpy_s(dst, size, src) snprintf(dst, size, "%s", src)
#define MAKE_RGB(r, g, b) ((r)|((g)<<8)|((b)<<16))
#else
#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))
#endif
