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

#include "Platform/PlatformSelector.hpp"

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

} // namespace Zero

// Microsoft Visual C++ Compiler
#if defined(COMPILER_MICROSOFT)

// Note: The following interlocked functions are unavailable as intrinsics,
//       hence why we need to include the Windows header for replacement functions:
//       _InterlockedExchange64
//       _InterlockedExchangeAdd64
//       _InterlockedIncrement8
//       _InterlockedIncrement64
//       _InterlockedDecrement8
//       _InterlockedDecrement64
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

//
// Intrinsic Declarations
//

extern "C"
{

// InterlockedExchange
char    __cdecl _InterlockedExchange8 (char volatile*    target, char    value);
short   __cdecl _InterlockedExchange16(short volatile*   target, short   value);
long    __cdecl _InterlockedExchange  (long volatile*    target, long    value);
//__int64 __cdecl _InterlockedExchange64(__int64 volatile* target, __int64 value);

// InterlockedCompareExchange
char    __cdecl _InterlockedCompareExchange8 (char volatile*    target, char    value, char    comparison);
short   __cdecl _InterlockedCompareExchange16(short volatile*   target, short   value, short   comparison);
long    __cdecl _InterlockedCompareExchange  (long volatile*    target, long    value, long    comparison);
__int64 __cdecl _InterlockedCompareExchange64(__int64 volatile* target, __int64 value, __int64 comparison);

// InterlockedExchangeAdd
char    __cdecl _InterlockedExchangeAdd8 (char volatile*    target, char    value);
short   __cdecl _InterlockedExchangeAdd16(short volatile*   target, short   value);
long    __cdecl _InterlockedExchangeAdd  (long volatile*    target, long    value);
//__int64 __cdecl _InterlockedExchangeAdd64(__int64 volatile* target, __int64 value);

// InterlockedIncrement
//char    __cdecl _InterlockedIncrement8 (char volatile*    target);
short   __cdecl _InterlockedIncrement16(short volatile*   target);
long    __cdecl _InterlockedIncrement  (long volatile*    target);
//__int64 __cdecl _InterlockedIncrement64(__int64 volatile* target);

// InterlockedDecrement
//char    __cdecl _InterlockedDecrement8 (char volatile*    target);
short   __cdecl _InterlockedDecrement16(short volatile*   target);
long    __cdecl _InterlockedDecrement  (long volatile*    target);
//__int64 __cdecl _InterlockedDecrement64(__int64 volatile* target);

} // extern "C"

//
// Intrinsic Directives
//

// InterlockedExchange
#pragma intrinsic (_InterlockedExchange8 )
#pragma intrinsic (_InterlockedExchange16)
#pragma intrinsic (_InterlockedExchange  )
//#pragma intrinsic (_InterlockedExchange64)

// InterlockedCompareExchange
#pragma intrinsic (_InterlockedCompareExchange8 )
#pragma intrinsic (_InterlockedCompareExchange16)
#pragma intrinsic (_InterlockedCompareExchange  )
#pragma intrinsic (_InterlockedCompareExchange64)

// InterlockedExchangeAdd
#pragma intrinsic (_InterlockedExchangeAdd8 )
#pragma intrinsic (_InterlockedExchangeAdd16)
#pragma intrinsic (_InterlockedExchangeAdd  )
//#pragma intrinsic (_InterlockedExchangeAdd64)

// InterlockedIncrement
//#pragma intrinsic (_InterlockedIncrement8 )
#pragma intrinsic (_InterlockedIncrement16)
#pragma intrinsic (_InterlockedIncrement  )
//#pragma intrinsic (_InterlockedIncrement64)

// InterlockedDecrement
//#pragma intrinsic (_InterlockedDecrement8 )
#pragma intrinsic (_InterlockedDecrement16)
#pragma intrinsic (_InterlockedDecrement  )
//#pragma intrinsic (_InterlockedDecrement64)

namespace Zero
{

// AtomicExchange
s8  AtomicExchange(volatile s8*  target, s8  value)
{
  return (s8)::_InterlockedExchange8((char volatile*)target, (char)value);
}
s16 AtomicExchange(volatile s16* target, s16 value)
{
  return (s16)::_InterlockedExchange16((short volatile*)target, (short)value);
}
s32 AtomicExchange(volatile s32* target, s32 value)
{
  return (s32)::_InterlockedExchange((long volatile*)target, (long)value);
}
s64 AtomicExchange(volatile s64* target, s64 value)
{
  return (s64)::InterlockedExchange64((__int64 volatile*)target, (__int64)value); // _InterlockedExchange64 Unavailable
}

// AtomicCompareExchange
s8  AtomicCompareExchange(volatile s8*  target, s8  value, s8  comparison)
{
  return (s8)::_InterlockedCompareExchange8((char volatile*)target, (char)value, (char)comparison);
}
s16 AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison)
{
  return (s16)::_InterlockedCompareExchange16((short volatile*)target, (short)value, (short)comparison);
}
s32 AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison)
{
  return (s32)::_InterlockedCompareExchange((long volatile*)target, (long)value, (long)comparison);
}
s64 AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison)
{
  return (s64)::_InterlockedCompareExchange64((__int64 volatile*)target, (__int64)value, (__int64)comparison);
}

// AtomicCompareExchangeBool
bool AtomicCompareExchangeBool(volatile s8*  target, s8  value, s8  comparison)
{
  return (s8)::_InterlockedCompareExchange8((char volatile*)target, (char)value, (char)comparison) == comparison;
}
bool AtomicCompareExchangeBool(volatile s16* target, s16 value, s16 comparison)
{
  return (s16)::_InterlockedCompareExchange16((short volatile*)target, (short)value, (short)comparison) == comparison;
}
bool AtomicCompareExchangeBool(volatile s32* target, s32 value, s32 comparison)
{
  return (s32)::_InterlockedCompareExchange((long volatile*)target, (long)value, (long)comparison) == comparison;
}
bool AtomicCompareExchangeBool(volatile s64* target, s64 value, s64 comparison)
{
  return (s64)::_InterlockedCompareExchange64((__int64 volatile*)target, (__int64)value, (__int64)comparison) == comparison;
}

// AtomicFetchAdd
s8  AtomicFetchAdd(volatile s8*  target, s8  value)
{
  return (s8)::_InterlockedExchangeAdd8((char volatile*)target, (char)value);
}
s16 AtomicFetchAdd(volatile s16* target, s16 value)
{
  return (s16)::_InterlockedExchangeAdd16((short volatile*)target, (short)value);
}
s32 AtomicFetchAdd(volatile s32* target, s32 value)
{
  return (s32)::_InterlockedExchangeAdd((long volatile*)target, (long)value);
}
s64 AtomicFetchAdd(volatile s64* target, s64 value)
{
  return (s64)::InterlockedExchangeAdd64((__int64 volatile*)target, (__int64)value); // _InterlockedExchangeAdd64 Unavailable
}

// AtomicFetchSubtract
s8  AtomicFetchSubtract(volatile s8*  target, s8  value)
{
  return (s8)::_InterlockedExchangeAdd8((char volatile*)target, -((char)value));
}
s16 AtomicFetchSubtract(volatile s16* target, s16 value)
{
  return (s16)::_InterlockedExchangeAdd16((short volatile*)target, -((short)value));
}
s32 AtomicFetchSubtract(volatile s32* target, s32 value)
{
  return (s32)::_InterlockedExchangeAdd((long volatile*)target, -((long)value));
}
s64 AtomicFetchSubtract(volatile s64* target, s64 value)
{
  return (s64)::InterlockedExchangeAdd64((__int64 volatile*)target, -((__int64)value)); // _InterlockedExchangeAdd64 Unavailable
}

// AtomicPreIncrement
s8  AtomicPreIncrement(volatile s8*  target)
{
  return AtomicPostIncrement(target) + s8(1); // _InterlockedIncrement8 Unavailable
}
s16 AtomicPreIncrement(volatile s16* target)
{
  return (s16)::_InterlockedIncrement16((short volatile*)target);
}
s32 AtomicPreIncrement(volatile s32* target)
{
  return (s32)::_InterlockedIncrement((long volatile*)target);
}
s64 AtomicPreIncrement(volatile s64* target)
{
  return (s64)::InterlockedIncrement64((__int64 volatile*)target); // _InterlockedIncrement64 Unavailable
}

// AtomicPostIncrement
s8  AtomicPostIncrement(volatile s8*  target)
{
  return AtomicFetchAdd(target, s8(1));
}
s16 AtomicPostIncrement(volatile s16* target)
{
  return AtomicFetchAdd(target, s16(1));
}
s32 AtomicPostIncrement(volatile s32* target)
{
  return AtomicFetchAdd(target, s32(1));
}
s64 AtomicPostIncrement(volatile s64* target)
{
  return AtomicFetchAdd(target, s64(1));
}

// AtomicPreDecrement
s8  AtomicPreDecrement(volatile s8*  target)
{
  return AtomicPostDecrement(target) - s8(1); // _InterlockedDecrement8 Unavailable
}
s16 AtomicPreDecrement(volatile s16* target)
{
  return (s16)::_InterlockedDecrement16((short volatile*)target);
}
s32 AtomicPreDecrement(volatile s32* target)
{
  return (s32)::_InterlockedDecrement((long volatile*)target);
}
s64 AtomicPreDecrement(volatile s64* target)
{
  return (s64)::InterlockedDecrement64((__int64 volatile*)target); // _InterlockedDecrement64 Unavailable
}

// AtomicPostDecrement
s8  AtomicPostDecrement(volatile s8*  target)
{
  return AtomicFetchSubtract(target, s8(1));
}
s16 AtomicPostDecrement(volatile s16* target)
{
  return AtomicFetchSubtract(target, s16(1));
}
s32 AtomicPostDecrement(volatile s32* target)
{
  return AtomicFetchSubtract(target, s32(1));
}
s64 AtomicPostDecrement(volatile s64* target)
{
  return AtomicFetchSubtract(target, s64(1));
}

} // namespace Zero

/// GNU Compiler Collection
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)

namespace Zero
{

// AtomicExchange
s8  AtomicExchange(volatile s8*  target, s8  value)
{
  return __sync_lock_test_and_set(target, value);
}
s16 AtomicExchange(volatile s16* target, s16 value)
{
  return __sync_lock_test_and_set(target, value);
}
s32 AtomicExchange(volatile s32* target, s32 value)
{
  return __sync_lock_test_and_set(target, value);
}
s64 AtomicExchange(volatile s64* target, s64 value)
{
  return __sync_lock_test_and_set(target, value);
}

// AtomicCompareExchange
s8  AtomicCompareExchange(volatile s8*  target, s8  value, s8  comparison)
{
  return __sync_val_compare_and_swap(target, comparison, value);
}
s16 AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison)
{
  return __sync_val_compare_and_swap(target, comparison, value);
}
s32 AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison)
{
  return __sync_val_compare_and_swap(target, comparison, value);
}
s64 AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison)
{
  return __sync_val_compare_and_swap(target, comparison, value);
}

// AtomicCompareExchangeBool
bool AtomicCompareExchangeBool(volatile s8*  target, s8  value, s8  comparison)
{
  return __sync_bool_compare_and_swap(target, comparison, value);
}
bool AtomicCompareExchangeBool(volatile s16* target, s16 value, s16 comparison)
{
  return __sync_bool_compare_and_swap(target, comparison, value);
}
bool AtomicCompareExchangeBool(volatile s32* target, s32 value, s32 comparison)
{
  return __sync_bool_compare_and_swap(target, comparison, value);
}
bool AtomicCompareExchangeBool(volatile s64* target, s64 value, s64 comparison)
{
  return __sync_bool_compare_and_swap(target, comparison, value);
}

// AtomicFetchAdd
s8  AtomicFetchAdd(volatile s8*  target, s8  value)
{
  return __sync_fetch_and_add(target, value);
}
s16 AtomicFetchAdd(volatile s16* target, s16 value)
{
  return __sync_fetch_and_add(target, value);
}
s32 AtomicFetchAdd(volatile s32* target, s32 value)
{
  return __sync_fetch_and_add(target, value);
}
s64 AtomicFetchAdd(volatile s64* target, s64 value)
{
  return __sync_fetch_and_add(target, value);
}

// AtomicFetchSubtract
s8  AtomicFetchSubtract(volatile s8*  target, s8  value)
{
  return __sync_fetch_and_sub(target, value);
}
s16 AtomicFetchSubtract(volatile s16* target, s16 value)
{
  return __sync_fetch_and_sub(target, value);
}
s32 AtomicFetchSubtract(volatile s32* target, s32 value)
{
  return __sync_fetch_and_sub(target, value);
}
s64 AtomicFetchSubtract(volatile s64* target, s64 value)
{
  return __sync_fetch_and_sub(target, value);
}

// AtomicPreIncrement
s8  AtomicPreIncrement(volatile s8*  target)
{
  return __sync_add_and_fetch(target, s8(1));
}
s16 AtomicPreIncrement(volatile s16* target)
{
  return __sync_add_and_fetch(target, s16(1));
}
s32 AtomicPreIncrement(volatile s32* target)
{
  return __sync_add_and_fetch(target, s32(1));
}
s64 AtomicPreIncrement(volatile s64* target)
{
  return __sync_add_and_fetch(target, s64(1));
}

// AtomicPostIncrement
s8  AtomicPostIncrement(volatile s8*  target)
{
  return __sync_fetch_and_add(target, s8(1));
}
s16 AtomicPostIncrement(volatile s16* target)
{
  return __sync_fetch_and_add(target, s16(1));
}
s32 AtomicPostIncrement(volatile s32* target)
{
  return __sync_fetch_and_add(target, s32(1));
}
s64 AtomicPostIncrement(volatile s64* target)
{
  return __sync_fetch_and_add(target, s64(1));
}

// AtomicPreDecrement
s8  AtomicPreDecrement(volatile s8*  target)
{
  return __sync_sub_and_fetch(target, s8(1));
}
s16 AtomicPreDecrement(volatile s16* target)
{
  return __sync_sub_and_fetch(target, s16(1));
}
s32 AtomicPreDecrement(volatile s32* target)
{
  return __sync_sub_and_fetch(target, s32(1));
}
s64 AtomicPreDecrement(volatile s64* target)
{
  return __sync_sub_and_fetch(target, s64(1));
}

// AtomicPostDecrement
s8  AtomicPostDecrement(volatile s8*  target)
{
  return __sync_fetch_and_sub(target, s8(1));
}
s16 AtomicPostDecrement(volatile s16* target)
{
  return __sync_fetch_and_sub(target, s16(1));
}
s32 AtomicPostDecrement(volatile s32* target)
{
  return __sync_fetch_and_sub(target, s32(1));
}
s64 AtomicPostDecrement(volatile s64* target)
{
  return __sync_fetch_and_sub(target, s64(1));
}

} // namespace Zero

#endif
