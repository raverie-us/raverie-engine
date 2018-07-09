///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String GetCallStack(StringParam extraSymbolPath, OsHandle exceptionContext)
{
  char buffer[4096];
  emscripten_get_callstack(
    EM_LOG_JS_STACK | EM_LOG_DEMANGLE | EM_LOG_FUNC_PARAMS,
    buffer,
    sizeof(buffer));
    
  return buffer;
}

}//namespace Zero
