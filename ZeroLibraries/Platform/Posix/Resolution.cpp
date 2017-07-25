///////////////////////////////////////////////////////////////////////////////
///
/// \file Resolution.cpp
/// Resolution support functions.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Resolution.hpp"

namespace Zero
{


Resolution GetDesktopResolution()
{
  Resolution desktopRes;
  desktopRes.Width = 800;
  desktopRes.Height = 600;
  return desktopRes;
}

void Enumerate(Array<Resolution>& resolutions, int bitDepth, Resolution aspect)
{

}


}//namespace Zero
