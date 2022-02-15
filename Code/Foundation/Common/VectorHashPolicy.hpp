// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

/// Hashing for an integer vector2
template <>
struct ZeroShared HashPolicy<Math::IntVec2>
{
  inline size_t operator()(Math::IntVec2Param value) const
  {
    return HashUint(*(unsigned int*)&value.x) + HashUint(*(unsigned int*)&value.y);
  }

  inline bool Equal(Math::IntVec2Param left, Math::IntVec2Param right) const
  {
    return left == right;
  }
};

template <>
struct HashPolicy<Math::IntVec3>
{
  inline size_t operator()(Math::IntVec3Param value) const
  {
    const int h1 = 0x8da6b343;
    const int h2 = 0xd8163841;
    const int h3 = 0xcb1ab31f;
    size_t key = value.x * h1 + value.y * h2 + value.z * h3;
    return key;
  }

  inline bool Equal(Math::IntVec3Param left, Math::IntVec3Param right) const
  {
    return left == right;
  }
};

} // namespace Zero
