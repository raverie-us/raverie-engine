// MIT Licensed (see LICENSE.md).
#pragma once
#include "Math.hpp"

namespace Zero
{
using Math::IntVec2;

/// Integer rectangle where the origin is the top left.
struct IntRect
{
  IntRect()
  {
  }
  IntRect(int x, int y, int sx, int sy) : X(x), Y(y), SizeX(sx), SizeY(sy)
  {
  }
  IntVec2 TopLeft() const
  {
    return IntVec2(X, Y);
  }
  IntVec2 Size() const
  {
    return IntVec2(SizeX, SizeY);
  }
  int Left() const
  {
    return X;
  }
  int Right() const
  {
    return X + SizeX;
  }
  int Top() const
  {
    return Y;
  }
  int Bottom() const
  {
    return Y + SizeY;
  }

  bool Contains(IntVec2& v) const
  {
    if (v.x < X)
      return false;
    if (v.y < Y)
      return false;
    if (v.y >= Y + SizeY)
      return false;
    if (v.x >= X + SizeX)
      return false;
    return true;
  }

  IntVec2 Center(IntVec2 size) const
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

} // namespace Zero
