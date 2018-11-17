///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#include "SDL2/SDL.h"

// Include glew before OpenGl
#define GLEW_STATIC
#include "GL/glew.h"

// Include OpenGl
#include "GL/GL.h"

#include <emscripten.h>

#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <utime.h>
