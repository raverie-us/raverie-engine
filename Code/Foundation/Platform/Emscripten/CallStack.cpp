// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

String GetCallStack(StringParam extraSymbolPath, OsHandle exceptionContext)
{
  auto flags = EM_LOG_JS_STACK | EM_LOG_FUNC_PARAMS;

  auto size = emscripten_get_callstack(flags, nullptr, 0);
  char* buffer = (char*)alloca(size);

  emscripten_get_callstack(flags, buffer, size);
  return buffer;
}

} // namespace Zero
