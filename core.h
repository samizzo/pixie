#pragma once

#include <cstdint>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define MAKE_RGB(r, g, b) ((b)|((g)<<8)|((r)<<16))

#if defined(_MSC_VER)
#define PIXIE_PLATFORM_WIN 1
#else
#error "Unsupported platform"
#endif
