////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// AtomicStore
void AtomicStore(volatile s8*  target, s8  value)
{
  Error("Not implemented");
}
void AtomicStore(volatile s16* target, s16 value)
{
  Error("Not implemented");
}
void AtomicStore(volatile s32* target, s32 value)
{
  Error("Not implemented");
}
void AtomicStore(volatile s64* target, s64 value)
{
  Error("Not implemented");
}
void AtomicStore(void* volatile* target, void* value)
{
  Error("Not implemented");
}

// AtomicLoad
s8 AtomicLoad(volatile s8*  target)
{
  Error("Not implemented");
  return s8(0);
}
s16 AtomicLoad(volatile s16* target)
{
  Error("Not implemented");
  return s16(0);
}
s32 AtomicLoad(volatile s32* target)
{
  Error("Not implemented");
  return s32(0);
}
s64 AtomicLoad(volatile s64* target)
{
  Error("Not implemented");
  return s64(0);
}
void* AtomicLoad(void* volatile* target)
{
  Error("Not implemented");
  return nullptr;
}

// AtomicExchange
s8  AtomicExchange(volatile s8*  target, s8  value)
{
  Error("Not implemented");
  return s8(0);
}

s16 AtomicExchange(volatile s16* target, s16 value)
{
  Error("Not implemented");
  return s16(0);
}

s32 AtomicExchange(volatile s32* target, s32 value)
{
  Error("Not implemented");
  return s32(0);
}

s64 AtomicExchange(volatile s64* target, s64 value)
{
  Error("Not implemented");
  return s64(0);
}

void* AtomicExchange(void* volatile* target, void* value)
{
  Error("Not implemented");
  return nullptr;
}

// AtomicCompareExchange
bool AtomicCompareExchange(volatile s8*  target, s8  value, s8 comparison)
{
  Error("Not implemented");
  return false;
}

bool AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison)
{
  Error("Not implemented");
  return false;
}

bool AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison)
{
  Error("Not implemented");
  return false;
}

bool AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison)
{
  Error("Not implemented");
  return false;
}

bool AtomicCompareExchange(void* volatile* target, void* value, void* comparison)
{
  Error("Not implemented");
  return false;
}

// AtomicFetchAdd
s8  AtomicFetchAdd(volatile s8*  target, s8  value)
{
  Error("Not implemented");
  return s8(0);
}

s16 AtomicFetchAdd(volatile s16* target, s16 value)
{
  Error("Not implemented");
  return s16(0);
}

s32 AtomicFetchAdd(volatile s32* target, s32 value)
{
  Error("Not implemented");
  return s32(0);
}

s64 AtomicFetchAdd(volatile s64* target, s64 value)
{
  Error("Not implemented");
  return s64(0);
}

// AtomicFetchSubtract
s8  AtomicFetchSubtract(volatile s8*  target, s8  value)
{
  Error("Not implemented");
  return s8(0);
}

s16 AtomicFetchSubtract(volatile s16* target, s16 value)
{
  Error("Not implemented");
  return s16(0);
}

s32 AtomicFetchSubtract(volatile s32* target, s32 value)
{
  Error("Not implemented");
  return s32(0);
}

s64 AtomicFetchSubtract(volatile s64* target, s64 value)
{
  Error("Not implemented");
  return s64(0);
}

// AtomicPreIncrement
s8  AtomicPreIncrement(volatile s8*  target)
{
  Error("Not implemented");
  return s8(0);
}

s16 AtomicPreIncrement(volatile s16* target)
{
  Error("Not implemented");
  return s16(0);
}

s32 AtomicPreIncrement(volatile s32* target)
{
  Error("Not implemented");
  return s32(0);
}

s64 AtomicPreIncrement(volatile s64* target)
{
  Error("Not implemented");
  return s64(0);
}

// AtomicPostIncrement
s8  AtomicPostIncrement(volatile s8*  target)
{
  Error("Not implemented");
  return s8(0);
}
s16 AtomicPostIncrement(volatile s16* target)
{
  Error("Not implemented");
  return s16(0);
}
s32 AtomicPostIncrement(volatile s32* target)
{
  Error("Not implemented");
  return s32(0);
}
s64 AtomicPostIncrement(volatile s64* target)
{
  Error("Not implemented");
  return s64(0);
}

// AtomicPreDecrement
s8  AtomicPreDecrement(volatile s8*  target)
{
  Error("Not implemented");
  return s8(0);
}

s16 AtomicPreDecrement(volatile s16* target)
{
  Error("Not implemented");
  return s16(0);
}

s32 AtomicPreDecrement(volatile s32* target)
{
  Error("Not implemented");
  return s32(0);
}

s64 AtomicPreDecrement(volatile s64* target)
{
  Error("Not implemented");
  return s64(0);
}

// AtomicPostDecrement
s8  AtomicPostDecrement(volatile s8*  target)
{
  Error("Not implemented");
  return s8(0);
}
s16 AtomicPostDecrement(volatile s16* target)
{
  Error("Not implemented");
  return s16(0);
}
s32 AtomicPostDecrement(volatile s32* target)
{
  Error("Not implemented");
  return s32(0);
}
s64 AtomicPostDecrement(volatile s64* target)
{
  Error("Not implemented");
  return s64(0);
}

}// namespace Zero