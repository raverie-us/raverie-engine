// MIT Licensed (see LICENSE.md).

#include "RendererGLStandard.hpp"

#ifdef WelderTargetOsEmscripten
#  define ZeroWebgl
#else
#  define ZeroGl
#endif

#ifdef ZeroWebgl
# include "GLES3/gl32.h"
#else
// Include glew before OpenGl
# include <GL/glew.h>
# ifdef WIN32
#   include <GL/wglew.h>
# endif
#endif

// Include OpenGl
#include <GL/gl.h>
