///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include "SDL.h"

// Include glew before OpenGl
#define GLEW_STATIC
#include <GL/glew.h>

// Include OpenGl
#include <GL/GL.h>

#ifdef PLATFORM_APPLE
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#ifdef PLATFORM_WINDOWS
#pragma comment(lib, "freetype28.lib")
#pragma comment(lib, "OpenGL32.Lib")

#ifdef ZeroRelease
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "glew32s.lib")
#else
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "glew32s.lib")
#endif

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")
#endif
