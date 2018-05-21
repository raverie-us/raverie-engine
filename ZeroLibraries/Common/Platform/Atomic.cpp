///////////////////////////////////////////////////////////////////////////////
///
/// \file Atomic.cpp
/// Implementation of the atomic functions.
///
/// Authors: Chris Peters, Andrew Colean
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// AtomicStore
void AtomicStore(volatile s8*  target, s8  value)
{
  AtomicExchange(target, value);
}
void AtomicStore(volatile s16* target, s16 value)
{
  AtomicExchange(target, value);
}
void AtomicStore(volatile s32* target, s32 value)
{
  AtomicExchange(target, value);
}
void AtomicStore(volatile s64* target, s64 value)
{
  AtomicExchange(target, value);
}
void AtomicStore(void* volatile* target, void* value)
{
  AtomicExchange(target, value);
}

// AtomicLoad
s8  AtomicLoad(volatile s8*  target)
{
  return AtomicCompareExchange(target, s8(0), s8(0));
}
s16 AtomicLoad(volatile s16* target)
{
  return AtomicCompareExchange(target, s16(0), s16(0));
}
s32 AtomicLoad(volatile s32* target)
{
  return AtomicCompareExchange(target, s32(0), s32(0));
}
s64 AtomicLoad(volatile s64* target)
{
  return AtomicCompareExchange(target, s64(0), s64(0));
}
void* AtomicLoad(void* volatile* target)
{
  return AtomicCompareExchange(target, nullptr, nullptr);
}

} // namespace Zero
