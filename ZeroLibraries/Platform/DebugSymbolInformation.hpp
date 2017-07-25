///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "String/String.hpp"
#include "String/StringBuilder.hpp"
#include "OsHandle.hpp"

namespace Zero
{

struct SymbolInfo
{
  // The address of the symbol to look-up
  void* mAddress;

  // The name of the symbol (could be a function name or variable name).
  String mSymbolName;
  String mFileName;
  // The name of the module this symbol is in (stripped path).
  String mModuleName;
  String mModulePath;
  size_t mLineNumber;
};

/// Stores the addresses of all functions in a callstack. Used to efficiently capture callstacks to
/// then later look-up symbol information. Hard-coded max for now (no allocations).
struct CallStackAddresses
{
  const static int mMaxCallstacks = 150;
  void* mAddresses[mMaxCallstacks];
  size_t mCaptureFrameCount;
};

/// Stores resolved symbol information for call stack addresses.
struct CallStackSymbolInfos
{
  /// Display the callstacks in a nice format
  String ToString() const;

  SymbolInfo mSymbols[CallStackAddresses::mMaxCallstacks];
  size_t mCaptureSymbolCount;
};

// Given a process fill out the information for the symbol at the mAddress   location on the SymbolInfo class.
void GetSymbolInfo(OsInt processHandle, SymbolInfo& symbolInfo);
/// Capture the current callstack into memory addresses. This is more efficient than actually looking up
/// symbol information which can be done at a later time. Default frames to skip is 1 so that this function is ignored.
size_t GetStackAddresses(CallStackAddresses& callStack, size_t stacksToCapture = CallStackAddresses::mMaxCallstacks, size_t framesToSkip = 1);
/// Convert the stack address pointers to the actual symbol information.
void GetStackInfo(CallStackAddresses& callStackAddresses, CallStackSymbolInfos& callStackSymbols);

// A simple stack walker that isn't fully flushed out but is much easier to follow than the StackWalker class.
class SimpleStackWalker
{
public:
  virtual ~SimpleStackWalker() {};

  void ShowCallstack(void* context, StringParam extraSymbolPaths = String(), int stacksToSkip = 1);
  virtual void AddSymbolInformation(SymbolInfo& symbolInfo);
  virtual String GetFinalOutput();

  StringBuilder mBuilder;
};

}//namespace Zero
