///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for windows library.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include "SDL.h"

#define GLEW_STATIC

// Include glew before OpenGl
#include <GL/glew.h>

// Include OpenGl
#include <GL/GL.h>

#pragma comment(lib, "freetype28.lib")
#pragma comment(lib, "OpenGL32.Lib")

#if ZeroRelease
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "glew32s.lib")
#else
#pragma comment(lib, "zlibd.lib")
#pragma comment(lib, "libpngd.lib")
#pragma comment(lib, "glew32s.lib")
#endif

#pragma comment(lib, "SDL2.lib")