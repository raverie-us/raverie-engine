///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

template <>
NativeType* NativeType::GetInstance<void>()
{
  // Because void is an incomplete type we do not provide type info for it.
  // Instead, NativeTypeOf(void) is used as a sentinel value, returning nullptr.
  return nullptr;
}

} // namespace Zero

