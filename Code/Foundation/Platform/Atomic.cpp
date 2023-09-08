// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// AtomicStore
void AtomicStore(volatile s8* target, s8 value)
{
  *target = value;
}
void AtomicStore(volatile s16* target, s16 value)
{
  *target = value;
}
void AtomicStore(volatile s32* target, s32 value)
{
  *target = value;
}
void AtomicStore(volatile s64* target, s64 value)
{
  *target = value;
}
void AtomicStore(void* volatile* target, void* value)
{
  *target = value;
}

// AtomicLoad
s8 AtomicLoad(volatile s8* target)
{
  return *target;
}
s16 AtomicLoad(volatile s16* target)
{
  return *target;
}
s32 AtomicLoad(volatile s32* target)
{
  return *target;
}
s64 AtomicLoad(volatile s64* target)
{
  return *target;
}
void* AtomicLoad(void* volatile* target)
{
  return *target;
}

// AtomicExchange
s8 AtomicExchange(volatile s8* target, s8 value)
{
  s8 previous = *target;
  *target = value;
  return previous;
}

s16 AtomicExchange(volatile s16* target, s16 value)
{
  s16 previous = *target;
  *target = value;
  return previous;
}

s32 AtomicExchange(volatile s32* target, s32 value)
{
  s32 previous = *target;
  *target = value;
  return previous;
}

s64 AtomicExchange(volatile s64* target, s64 value)
{
  s64 previous = *target;
  *target = value;
  return previous;
}

void* AtomicExchange(void* volatile* target, void* value)
{
  void* previous = *target;
  *target = value;
  return previous;
}

// AtomicCompareExchange
bool AtomicCompareExchange(volatile s8* target, s8 value, s8 comparison)
{
  if (*target == comparison)
  {
    *target = value;
    return true;
  }
  return false;
}

bool AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison)
{
  if (*target == comparison)
  {
    *target = value;
    return true;
  }
  return false;
}

bool AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison)
{
  if (*target == comparison)
  {
    *target = value;
    return true;
  }
  return false;
}

bool AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison)
{
  if (*target == comparison)
  {
    *target = value;
    return true;
  }
  return false;
}

bool AtomicCompareExchange(void* volatile* target, void* value, void* comparison)
{
  if (*target == comparison)
  {
    *target = value;
    return true;
  }
  return false;
}

// AtomicFetchAdd
s8 AtomicFetchAdd(volatile s8* target, s8 value)
{
  s8 previous = *target;
  *target += value;
  return previous;
}

s16 AtomicFetchAdd(volatile s16* target, s16 value)
{
  s16 previous = *target;
  *target += value;
  return previous;
}

s32 AtomicFetchAdd(volatile s32* target, s32 value)
{
  s32 previous = *target;
  *target += value;
  return previous;
}

s64 AtomicFetchAdd(volatile s64* target, s64 value)
{
  s64 previous = *target;
  *target += value;
  return previous;
}

// AtomicFetchSubtract
s8 AtomicFetchSubtract(volatile s8* target, s8 value)
{
  s8 previous = *target;
  *target -= value;
  return previous;
}

s16 AtomicFetchSubtract(volatile s16* target, s16 value)
{
  s16 previous = *target;
  *target -= value;
  return previous;
}

s32 AtomicFetchSubtract(volatile s32* target, s32 value)
{
  s32 previous = *target;
  *target -= value;
  return previous;
}

s64 AtomicFetchSubtract(volatile s64* target, s64 value)
{
  s64 previous = *target;
  *target -= value;
  return previous;
}

// AtomicPreIncrement
s8 AtomicPreIncrement(volatile s8* target)
{
  return ++(*target);
}

s16 AtomicPreIncrement(volatile s16* target)
{
  return ++(*target);
}

s32 AtomicPreIncrement(volatile s32* target)
{
  return ++(*target);
}

s64 AtomicPreIncrement(volatile s64* target)
{
  return ++(*target);
}

// AtomicPostIncrement
s8 AtomicPostIncrement(volatile s8* target)
{
  return (*target)++;
}
s16 AtomicPostIncrement(volatile s16* target)
{
  return (*target)++;
}
s32 AtomicPostIncrement(volatile s32* target)
{
  return (*target)++;
}
s64 AtomicPostIncrement(volatile s64* target)
{
  return (*target)++;
}

// AtomicPreDecrement
s8 AtomicPreDecrement(volatile s8* target)
{
  return --(*target);
}

s16 AtomicPreDecrement(volatile s16* target)
{
  return --(*target);
}

s32 AtomicPreDecrement(volatile s32* target)
{
  return --(*target);
}

s64 AtomicPreDecrement(volatile s64* target)
{
  return --(*target);
}

// AtomicPostDecrement
s8 AtomicPostDecrement(volatile s8* target)
{
  return (*target)++;
}
s16 AtomicPostDecrement(volatile s16* target)
{
  return (*target)++;
}
s32 AtomicPostDecrement(volatile s32* target)
{
  return (*target)++;
}
s64 AtomicPostDecrement(volatile s64* target)
{
  return (*target)++;
}

} // namespace Zero
