#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------DebuggerListener
void DebuggerListener::Print(FilterType filterType, cstr message)
{
  OutputDebugStringW(Widen(message).c_str());
}

//-------------------------------------------------------------------StdOutListener
void StdOutListener::Print(FilterType filterType, cstr message)
{
  fwprintf(stdout, L"%s", Widen(message).c_str());
  //// As this is used for logging primarily on the build machine make
  //// sure to flush every time so we have an up-to-date log file.
  fflush(stdout);
}

}//namespace Zero
