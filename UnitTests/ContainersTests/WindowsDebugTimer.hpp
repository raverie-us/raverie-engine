#include "Common/String/String.hpp"
#include "Platform/Timer.hpp"
#include <windows.h>
#include <debugapi.h>

#pragma once

using namespace Zero;

class WindowsDebugTimer
{
public:
  String Name;
  WindowsDebugTimer(StringParam name)
    :Name(name)
  {
  }

  ~WindowsDebugTimer()
  {
    float time = (float)timer.UpdateAndGetTime();
    sprintf(buffer, "%s %.6fs\n", Name.c_str(), time);
    OutputDebugStringA(buffer);
  }
  Timer timer;
  char buffer[50];
};