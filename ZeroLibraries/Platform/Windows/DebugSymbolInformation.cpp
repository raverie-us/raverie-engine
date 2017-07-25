///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Dbghelp has warnings in VS2015 (unnamed typedef)
#pragma warning(disable : 4091)
#ifdef _MSC_VER
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace Zero
{

String CallStackSymbolInfos::ToString() const
{
  StringBuilder builder;

  for(size_t i = 0; i < mCaptureSymbolCount; ++i)
  {
    const SymbolInfo& symbolInfo = mSymbols[i];
    if(symbolInfo.mFileName.Empty())
    {
      builder.Append(String::Format("%X (%s): (filename not available): %s\n", symbolInfo.mAddress, symbolInfo.mModuleName.c_str(), symbolInfo.mSymbolName.c_str()));
    }
    else
    {
      builder.Append(String::Format("%s (%d): %s\n", symbolInfo.mFileName.c_str(), symbolInfo.mLineNumber, symbolInfo.mSymbolName.c_str()));
    }
  }

  return builder.ToString();
}

void GetSymbolInfo(OsInt processHandle, SymbolInfo& symbolInfo)
{
  HANDLE process = (HANDLE)processHandle;

  //fill in all the junk needed for a SYMBOL_INFO structure
  ULONG64 buffer[(sizeof(SYMBOL_INFO) +
    MAX_SYM_NAME * sizeof(TCHAR) +
    sizeof(ULONG64) - 1) /
    sizeof(ULONG64)] = { 0 };
  PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;
  pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  pSymbol->MaxNameLen = MAX_SYM_NAME;
  //needed so that the line number functions doesn't freak
  DWORD  dwDisplacement;

  IMAGEHLP_LINE line = { 0 };
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

  DWORD64 returnAddress = reinterpret_cast<DWORD64>(symbolInfo.mAddress);

  //(these functions annoy me....)
  //get the line from the saved address
  SymGetLineFromAddr(process, static_cast<DWORD>(returnAddress), &dwDisplacement, &line);

  //get the symbol from the address
  SymFromAddr(process, returnAddress, 0, pSymbol);

  // Get the module information
  HMODULE hmodule = NULL;
  ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(symbolInfo.mAddress), &hmodule);
  wchar_t modulePath[MAX_PATH];
  GetModuleFileName(hmodule, modulePath, MAX_PATH);

  symbolInfo.mLineNumber = line.LineNumber;
  symbolInfo.mSymbolName = pSymbol->Name;
  // The file name could be unknown so check to make sure it's not null
  if(line.FileName != nullptr)
    symbolInfo.mFileName = line.FileName;

  // Get the symbol name by splitting the path (find last '\' and last '.' and take what's in-between)
  symbolInfo.mModulePath = Narrow(modulePath);
  StringRange found = symbolInfo.mModulePath.FindLastOf('\\');
  if(!found.Empty())
    symbolInfo.mModuleName = symbolInfo.mModulePath.SubString(found.End(), symbolInfo.mModulePath.End());

  found = symbolInfo.mModuleName.FindLastOf('.');
  if(found.Begin() < symbolInfo.mModuleName.End())
    symbolInfo.mModuleName = symbolInfo.mModuleName.SubString(symbolInfo.mModuleName.Begin(), found.Begin());
}

size_t GetStackAddresses(CallStackAddresses& callStack, size_t stacksToCapture, size_t framesToSkip)
{
  callStack.mCaptureFrameCount = CaptureStackBackTrace(framesToSkip, stacksToCapture, callStack.mAddresses, 0);
  return callStack.mCaptureFrameCount;
}

void GetStackInfo(CallStackAddresses& callStackAddresses, CallStackSymbolInfos& callStackSymbols)
{
  HANDLE process = GetCurrentProcess();
  SymInitialize(process, nullptr, TRUE);

  callStackSymbols.mCaptureSymbolCount = callStackAddresses.mCaptureFrameCount;
  for(size_t i = 0; i < callStackAddresses.mCaptureFrameCount; ++i)
  {
    SymbolInfo& symbolInfo = callStackSymbols.mSymbols[i];
    symbolInfo.mAddress = callStackAddresses.mAddresses[i];
    // Generate the symbol information for each stack entry
    GetSymbolInfo((OsInt)process, symbolInfo);
  }
}

void SimpleStackWalker::ShowCallstack(void* context, StringParam extraSymbolPaths, int stacksToSkip)
{
  HANDLE process = GetCurrentProcess();
  SymInitialize(process, extraSymbolPaths.c_str(), TRUE);

  // We were not handed a context to walk, capture the current call stack
  if(context == nullptr)
  {
    const int maxStacks = 150;
    void *stacks[maxStacks];
    // This function fills out function pointers for the current stack
    size_t capturedStacks = CaptureStackBackTrace(stacksToSkip, maxStacks, stacks, 0);

    for(size_t i = 0; i < capturedStacks; ++i)
    {
      SymbolInfo symbolInfo;
      symbolInfo.mAddress = stacks[i];
      // Generate the symbol information for each stack entry
      GetSymbolInfo((OsInt)process, symbolInfo);
      
      AddSymbolInformation(symbolInfo);
    }
  }
  // We had a context (most likely from a crash). Unfortunately, there's no super easy way to use
  // CaptureStackBackTrace which is a much nicer function than StalkWalk64.
  // This is a bit hacky but oh-well...
  else
  {
    PCONTEXT pcontext = (PCONTEXT)context;

    STACKFRAME64 stackFrame;
    ZeroMemory(&stackFrame, sizeof(stackFrame));
#if PLATFORM_64
    stackFrame.AddrPC.Offset = pcontext->Rip;
    stackFrame.AddrFrame.Offset = pcontext->Rbp;
    stackFrame.AddrStack.Offset = pcontext->Rsp;
#else
    stackFrame.AddrPC.Offset = pcontext->Eip;
    stackFrame.AddrFrame.Offset = pcontext->Ebp;
    stackFrame.AddrStack.Offset = pcontext->Esp;
#endif
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    int imageType;
#ifdef _M_IX86
    imageType = IMAGE_FILE_MACHINE_I386;
#elif _M_X64
    imageType = IMAGE_FILE_MACHINE_AMD64;
#elif _M_IA64
    imageType = IMAGE_FILE_MACHINE_IA64;
#endif

    while(1)
    {
      int i = 0;
      // Use stackwalk64 to iterate through frames. This modifies stackFrame.AddrPC.Offset each time.
      if(StackWalk64(imageType, process, GetCurrentThread(), &stackFrame, pcontext, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
      {
        // Skip all of the requested frames
        if(i < stacksToSkip)
          continue;

        ++i;
        SymbolInfo symbolInfo;
        symbolInfo.mAddress = (void*)stackFrame.AddrPC.Offset;
        GetSymbolInfo((OsInt)process, symbolInfo);

        AddSymbolInformation(symbolInfo);
      }
      // We failed to get a stack frame, we must be done
      else
        break;
    }
  }

  // Make sure to clean up!!! (bad things can happen otherwise)
  SymCleanup(process);
}

void SimpleStackWalker::AddSymbolInformation(SymbolInfo& symbolInfo)
{
  if(symbolInfo.mFileName.Empty())
  {
    mBuilder.Append(String::Format("%X (%s): (filename not available): %s\n", symbolInfo.mAddress, symbolInfo.mModuleName.c_str(), symbolInfo.mSymbolName.c_str()));
  }
  else
  {
    mBuilder.Append(String::Format("%s (%d): %s\n", symbolInfo.mFileName.c_str(), symbolInfo.mLineNumber, symbolInfo.mSymbolName.c_str()));
  }
}

String SimpleStackWalker::GetFinalOutput()
{
  return mBuilder.ToString();
}

}//namespace Zero

// For non microsoft on windows just stub for now
#else

namespace Zero
{

void GetSymbolInfo(OsInt processHandle, SymbolInfo& symbolInfo)
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

}//namespace Zero

#endif
