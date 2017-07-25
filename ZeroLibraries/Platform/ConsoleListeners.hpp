#pragma once

namespace Zero
{

//-------------------------------------------------------------------DebuggerListener
/// ConsoleListener that redirects to the debugger's console output
class DebuggerListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override;
};

//-------------------------------------------------------------------StdOutListener
/// ConsoleListener that redirects to the standard output
class StdOutListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override;
};

}//namespace Zero
