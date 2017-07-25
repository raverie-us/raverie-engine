///////////////////////////////////////////////////////////////////////////////
///
/// \file Resolution.hpp
/// Resolution support functions.
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/String.hpp"
#include "Containers/Array.hpp"

namespace Zero
{

struct ZeroShared Resolution
{
  Resolution()
    :Width(0), Height(0)
  {}

  Resolution(int w, int h)
    :Width(w), Height(h)
  {}

  bool SameAspect(const Resolution& other)const
  {
    return other.Width * Height == other.Height * Width;
  }

  bool operator==(const Resolution& other) const
  {
    return other.Width == Width &&  other.Height * Height;
  }

  bool operator<(const Resolution& other) const
  {
    if (Width < other.Width)
      return true;
    else
    {
      if (Height < other.Height)
        return true;
      else
        return false;
    }
  }

  String ToString(bool shortFormat) const
  {
    return String::Format("%d x %d", Width, Height);
  }

  size_t Hash() const
  {
    return Hash64to32Shift( *(u64*)this );
  }

  int Width;
  int Height;
};

const Resolution AspectAny(0,0);
const Resolution Aspect4by3(4,3);
const Resolution Aspect5by4(5,4);
const Resolution Aspect16by9(16,9);
const Resolution Aspect16by10(16,10);
const Resolution Aspects[5] = {AspectAny, Aspect4by3, Aspect5by4, Aspect16by9, Aspect16by10};

ZeroShared inline uint GetAspectIndex(Resolution& toTest)
{
  for(uint i=1;i<5;++i)
  {
    if(Aspects[i].SameAspect(toTest))
      return i;
  }
  return 0;
}

// Return the index of the first resolution that is greater than or
// equal to minWidth and minHeight. Return the first resolution to pass or the last in the list
ZeroShared inline int FindMinResolution(Array<Resolution>& resolutions, int minWidth, int minHeight)
{
  for(uint i=0;i<resolutions.Size();++i)
  {
    Resolution r = resolutions[i];
    if(r.Width >= minWidth && r.Height >= minHeight)
      return (int)i;
  }
  return (int)(resolutions.Size()-1);
}

/// Get the current resolution of the desktop.
ZeroShared Resolution GetDesktopResolution();

/// Enumerate all Resolutions valid for the display adapter with the given bit depth
/// and aspect ratio
/// bitDepth zero for any bit depth
/// resolution any resolution with the same aspect ratio (0,0) will return all.
ZeroShared void Enumerate(Array<Resolution>& resolutions, uint bitDepth, Resolution aspect);

}

