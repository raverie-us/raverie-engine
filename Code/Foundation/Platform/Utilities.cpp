// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
}

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  const int cDebugBufferLength = 1024;
  char buffer[cDebugBufferLength];
  ZeroSPrintf(buffer,
              cDebugBufferLength,
              "%s(%d) : %s %s\n",
              errorData.File,
              errorData.Line,
              errorData.Message,
              errorData.Expression);
  Console::Print(Filter::ErrorFilter, buffer);
  return true;
}

void WebRequest(Status& status,
                StringParam url,
                const Array<WebPostData>& postData,
                const Array<String>& additionalRequestHeaders,
                WebRequestHeadersFn onHeadersReceived,
                WebRequestDataFn onDataReceived,
                void* userData)
{
  status.SetFailed("WebRequest not implemented");
}

void OpenUrl(cstr url)
{
}

unsigned int GetDoubleClickTimeMs()
{
  return 500;
}
} // namespace Os

u64 GenerateUniqueId64()
{
  static u64 counter = 0;
  return ++counter;
}

} // namespace Zero
