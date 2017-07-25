///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2016 DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Engine/EngineStandard.hpp"
#include "Graphics/GraphicsStandard.hpp"

// Using static GLEW
#define GLEW_STATIC

// Include glew before OpenGl
#include <GL/glew.h>
#include <GL/wglew.h>

// Remove defines from WinDef.h
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// Include OpenGl
#include <GL/GL.h>

// Link Libraries
#pragma comment(lib, "OpenGL32.Lib")

#ifdef NDEBUG
#pragma comment(lib, "glew32s.lib")
#else
#pragma comment(lib, "glew32sd.lib")
#endif

namespace Zero
{
// Forward declarations
class X;

}//namespace Zero

#include "StreamedVertexBuffer.hpp"
#include "OpenglRenderer.hpp"
