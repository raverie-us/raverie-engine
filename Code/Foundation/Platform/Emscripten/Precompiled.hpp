// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Common/CommonStandard.hpp"

#include "SDL.h"

// Include glew before OpenGl
#define GLEW_STATIC
#include "GL/glew.h"

// Include OpenGl
#include <GL/gl.h>

#include <emscripten.h>
#include <emscripten/html5.h>

#include <new>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <utime.h>
