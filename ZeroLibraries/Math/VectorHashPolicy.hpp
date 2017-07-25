///////////////////////////////////////////////////////////////////////////////
///
/// Authors: 
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Hashing for an integer vector2
template<>
struct ZeroShared HashPolicy<Math::IntVec2>
{
  inline size_t operator()(Math::IntVec2Param value) const
  {
    return HashUint(*(unsigned int*)&value.x) +
           HashUint(*(unsigned int*)&value.y);
  }

  inline bool Equal(Math::IntVec2Param left, Math::IntVec2Param right) const
  {
    return left == right;
  }
};

} // namespace Zero
