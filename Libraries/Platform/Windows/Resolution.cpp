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

namespace Zero
{

Resolution GetDesktopResolution()
{
  Resolution desktopRes;
  //Enumerate the display settings for the default device so we can
  //get the desktop resolution
  DEVMODE data;
  data.dmSize = sizeof(DEVMODE);
  data.dmDriverExtra = 0;
  EnumDisplaySettings(NULL , ENUM_REGISTRY_SETTINGS , &data);
  desktopRes.Width = data.dmPelsWidth;
  desktopRes.Height = data.dmPelsHeight;
  return desktopRes;
}

void Enumerate(Array<Resolution>& resolutions, uint bitDepth, Resolution aspect)
{
  HashSet<Resolution> ResolutionMap;

  resolutions.Clear();
  // Enumerate all display modes for the default display adapter.
  DEVMODE Win32Mode;
  Win32Mode.dmSize = sizeof(DEVMODE);
  for(int index = 0; EnumDisplaySettings(NULL, index, &Win32Mode); ++index)
  {
    Resolution resolution(Win32Mode.dmPelsWidth, Win32Mode.dmPelsHeight);
    // Check bit depth
    if(bitDepth ==0 || Win32Mode.dmBitsPerPel == bitDepth)
      // Check aspect ratio
      if(resolution.SameAspect(aspect))
      {
        ResolutionMap.Insert(resolution);
      }
  }

  //push all in array
  PushAll(resolutions, ResolutionMap.All());
  //sort
  Sort(resolutions.All());
}


}//namespace Zero
