///////////////////////////////////////////////////////////////////////////////
///
/// \file Graphics.hpp
/// Declaration of the Graphics System interface.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Math.hpp"

namespace Zero
{
using Math::IntVec2;

struct PixelRect
{
  IntVec2 TopLeft(){return IntVec2(X, Y);}
  IntVec2 Size(){return IntVec2(SizeX, SizeY); }
  int Left(){return X;}
  int Right(){return X + SizeX;}
  int Top(){return Y;}
  int Bottom(){return Y + SizeY;}

  bool Contains(IntVec2& v)
  {
    if(v.x < X) return false;
    if(v.y < Y) return false;
    if(v.y >= Y + SizeY) return false;
    if(v.x >= X + SizeX) return false;
    return true;
  }

  IntVec2 Center(IntVec2 size)
  {
    IntVec2 v;
    v.x = X + (SizeX - size.x) / 2;
    v.y = Y + (SizeY - size.y) / 2;
    return v;
  }

  int X;
  int Y;
  int SizeX;
  int SizeY;
};

}
