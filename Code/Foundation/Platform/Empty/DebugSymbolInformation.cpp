// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

String CallStackSymbolInfos::ToString() const
{
  return String();
}

void GetSymbolInfo(OsInt processHandle, SymbolInfo& symbolInfo)
{
  symbolInfo.mAddress = nullptr;
  symbolInfo.mLineNumber = 0;
}

size_t GetStackAddresses(CallStackAddresses& callStack, size_t stacksToCapture, size_t framesToSkip)
{
  return -1;
}

void GetStackInfo(CallStackAddresses& callStackAddresses, CallStackSymbolInfos& callStackSymbols)
{
}

void SimpleStackWalker::ShowCallstack(void* context, StringParam extraSymbolPaths, int stacksToSkip)
{
}

void SimpleStackWalker::AddSymbolInformation(SymbolInfo& symbolInfo)
{
}

String SimpleStackWalker::GetFinalOutput()
{
  return String();
}

} // namespace Zero
