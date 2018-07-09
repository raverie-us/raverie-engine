///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include "../Empty/Precompiled.hpp"
#include "../STL/Precompiled.hpp"

#include "SDL2/SDL.h"

#define GLEW_STATIC

// Include glew before OpenGl
#include "GL/glew.h"

// Include OpenGl
#include "GL/GL.h"

#include <stdio.h>
#include <emscripten.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
