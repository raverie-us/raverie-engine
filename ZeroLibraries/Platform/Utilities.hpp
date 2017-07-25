///////////////////////////////////////////////////////////////////////////////
///
/// \file Utilities.hpp
/// Declaration of the Utilities class.
/// 
/// Authors: Trevor Sundberg, Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/StringRange.hpp"
#include "Utility/EnumDeclaration.hpp"
#include "Utility/Status.hpp"

namespace Zero
{
/// System Memory Information
struct ZeroShared MemoryInfo
{
  uint Reserve;
  uint Commit;
  uint Free;
};

namespace Os
{

// Sleep the current thread for ms milliseconds.
ZeroShared void Sleep(uint ms);

// Set the Timer Frequency (How often the OS checks threads for sleep, etc)
ZeroShared void SetTimerFrequency(uint ms);

// Get the user name for the current profile
ZeroShared String UserName();

// Get the computer name
ZeroShared String ComputerName();

// Get computer Mac Address of adapter 0
ZeroShared u64 GetMacAddress();

// Check if a debugger is attached
ZeroShared bool IsDebuggerAttached();

// Debug break (only if a debugger is attached)
ZeroShared void DebugBreak();

// Verb used to open file
DeclareEnum4(Verb, Default, Open, Edit, Run);

// Open the file using the appropriate Os application or
// launch an external application.
ZeroShared void SystemOpenFile(cstr file, uint verb=Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);
ZeroShared bool SystemOpenFile(Status& status, cstr file, uint verb=Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);

// Open the network file (including urls) using the appropriate
// Os application or launch an external application
ZeroShared void SystemOpenNetworkFile(cstr file, uint verb = Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);
ZeroShared bool SystemOpenNetworkFile(Status& status, cstr file, uint verb = Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);

// Get the memory status of the Os.
ZeroShared void GetMemoryStatus(MemoryInfo& memoryInfo);

// Get an Environmental variable
ZeroShared String GetEnvironmentalVariable(StringParam variable);

// Translate a OS error code.
ZeroShared String TranslateErrorCode(int errorCode);

// Get a string describing the current operating system version.
ZeroShared String GetVersionString();

}

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
ZeroShared u64 GenerateUniqueId64();

// Waits for expression to evaluate to true, checking approximately every pollPeriod (in milliseconds)
#define WaitUntil(expression, pollPeriod) \
do { while(!(expression)) { Os::Sleep(pollPeriod); } } while(gConditionalFalseConstant)

}//namespace Zero
