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

#if ZeroRelease
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng.lib")
#else
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpngd.lib")
#endif
