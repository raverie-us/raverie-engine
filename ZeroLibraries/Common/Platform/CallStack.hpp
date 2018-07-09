///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
/// Prints out the call stack for the current thread. The extra symbol path can specify
/// a location to search for symbol information. The exceptionContext is operating system dependent
/// and should be left as null unless we're handling a crash.
String GetCallStack(StringParam extraSymbolPath = String(), OsHandle exceptionContext = nullptr);

}//namespace Zero
