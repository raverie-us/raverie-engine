////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include <new>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cctype>
#include <sys/stat.h>

#pragma comment(lib, "freetype28.lib")
#pragma comment(lib, "libcef.lib")

#if ZeroRelease
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "libcef_dll_wrapper_release.lib")
#else
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "libcef_dll_wrapper_debug.lib")
#endif
