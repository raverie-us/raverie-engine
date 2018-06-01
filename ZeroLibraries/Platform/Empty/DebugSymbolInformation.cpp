///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String CallStackSymbolInfos::ToString() const
{
  Error("Not implemented");
  return String();
}

void GetSymbolInfo(OsInt processHandle, SymbolInfo& symbolInfo)
{
  Error("Not implemented");
}

size_t GetStackAddresses(CallStackAddresses& callStack, size_t stacksToCapture, size_t framesToSkip)
{
  Error("Not implemented");
  return -1;
}

void GetStackInfo(CallStackAddresses& callStackAddresses, CallStackSymbolInfos& callStackSymbols)
{
  Error("Not implemented");
}

void SimpleStackWalker::ShowCallstack(void* context, StringParam extraSymbolPaths, int stacksToSkip)
{
  Error("Not implemented");
}

void SimpleStackWalker::AddSymbolInformation(SymbolInfo& symbolInfo)
{
  Error("Not implemented");
}

String SimpleStackWalker::GetFinalOutput()
{
  Error("Not implemented");
  return String();
}

}//namespace Zero
