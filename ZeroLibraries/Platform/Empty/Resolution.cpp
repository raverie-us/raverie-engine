////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
Resolution GetDesktopResolution()
{
  return Resolution(800, 600);
}

void Enumerate(Array<Resolution>& resolutions, int bitDepth, Resolution aspect)
{
  resolutions.PushBack(Resolution(800, 600));
}

}//namespace Zero
