// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

int PlatformMain(const Array<String>& arguments)
{
  CommonLibrary::Initialize();
  int result = BrowserSubProcess::Execute();
  CommonLibrary::Shutdown();
  return result;
}

} // namespace Zero