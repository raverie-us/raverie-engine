///////////////////////////////////////////////////////////////////////////////
///
/// \file OpenGl.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#if defined(ANDROID)

// Android OpenGl ES
#include <GLES2/gl2.h>
#define ZOPENGLES

#elif defined(IOS)

// Apple Ios OpenGl ES 
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#define ZOPENGLES

#elif defined(__APPLE__)

// Apple Desktop
#include "glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#define ZOPENGLFULL

#else

// Linux Desktop
#include "GL/glew.h"
#include "GL/gl.h"
#define ZOPENGLFULL

#endif
