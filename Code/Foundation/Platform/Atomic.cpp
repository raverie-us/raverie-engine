// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include <atomic>

namespace Zero
{

static_assert(sizeof(s8) == sizeof(std::atomic<s8>),
              "For STL atomics to work as implemented these sizes must be the same");
static_assert(sizeof(s16) == sizeof(std::atomic<s16>),
              "For STL atomics to work as implemented these sizes must be the same");
static_assert(sizeof(s32) == sizeof(std::atomic<s32>),
              "For STL atomics to work as implemented these sizes must be the same");
static_assert(sizeof(s64) == sizeof(std::atomic<s64>),
              "For STL atomics to work as implemented these sizes must be the same");

// AtomicStore
void AtomicStore(volatile s8* target, s8 value)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  std::atomic_store(t, value);
}
void AtomicStore(volatile s16* target, s16 value)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  std::atomic_store(t, value);
}
void AtomicStore(volatile s32* target, s32 value)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  std::atomic_store(t, value);
}
void AtomicStore(volatile s64* target, s64 value)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  std::atomic_store(t, value);
}
void AtomicStore(void* volatile* target, void* value)
{
  volatile std::atomic<void*>* t = reinterpret_cast<volatile std::atomic<void*>*>(target);
  std::atomic_store(t, value);
}

// AtomicLoad
s8 AtomicLoad(volatile s8* target)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  return std::atomic_load(t);
}
s16 AtomicLoad(volatile s16* target)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  return std::atomic_load(t);
}
s32 AtomicLoad(volatile s32* target)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  return std::atomic_load(t);
}
s64 AtomicLoad(volatile s64* target)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  return std::atomic_load(t);
}
void* AtomicLoad(void* volatile* target)
{
  volatile std::atomic<void*>* t = reinterpret_cast<volatile std::atomic<void*>*>(target);
  return std::atomic_load(t);
}

// AtomicExchange
s8 AtomicExchange(volatile s8* target, s8 value)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  return std::atomic_exchange(t, value);
}

s16 AtomicExchange(volatile s16* target, s16 value)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  return std::atomic_exchange(t, value);
}

s32 AtomicExchange(volatile s32* target, s32 value)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  return std::atomic_exchange(t, value);
}

s64 AtomicExchange(volatile s64* target, s64 value)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  return std::atomic_exchange(t, value);
}

void* AtomicExchange(void* volatile* target, void* value)
{
  volatile std::atomic<void*>* t = reinterpret_cast<volatile std::atomic<void*>*>(target);
  return std::atomic_exchange(t, value);
}

// AtomicCompareExchange
bool AtomicCompareExchange(volatile s8* target, s8 value, s8 comparison)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  return std::atomic_compare_exchange_strong(t, &comparison, value);
}

bool AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  return std::atomic_compare_exchange_strong(t, &comparison, value);
}

bool AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  return std::atomic_compare_exchange_strong(t, &comparison, value);
}

bool AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  return std::atomic_compare_exchange_strong(t, &comparison, value);
}

bool AtomicCompareExchange(void* volatile* target, void* value, void* comparison)
{
  volatile std::atomic<void*>* t = reinterpret_cast<volatile std::atomic<void*>*>(target);
  return std::atomic_compare_exchange_strong(t, &comparison, value);
}

// AtomicFetchAdd
s8 AtomicFetchAdd(volatile s8* target, s8 value)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  return std::atomic_fetch_add(t, value);
}

s16 AtomicFetchAdd(volatile s16* target, s16 value)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  return std::atomic_fetch_add(t, value);
}

s32 AtomicFetchAdd(volatile s32* target, s32 value)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  return std::atomic_fetch_add(t, value);
}

s64 AtomicFetchAdd(volatile s64* target, s64 value)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  return std::atomic_fetch_add(t, value);
}

// AtomicFetchSubtract
s8 AtomicFetchSubtract(volatile s8* target, s8 value)
{
  volatile std::atomic<s8>* t = reinterpret_cast<volatile std::atomic<s8>*>(target);
  return std::atomic_fetch_sub(t, value);
}

s16 AtomicFetchSubtract(volatile s16* target, s16 value)
{
  volatile std::atomic<s16>* t = reinterpret_cast<volatile std::atomic<s16>*>(target);
  return std::atomic_fetch_sub(t, value);
}

s32 AtomicFetchSubtract(volatile s32* target, s32 value)
{
  volatile std::atomic<s32>* t = reinterpret_cast<volatile std::atomic<s32>*>(target);
  return std::atomic_fetch_sub(t, value);
}

s64 AtomicFetchSubtract(volatile s64* target, s64 value)
{
  volatile std::atomic<s64>* t = reinterpret_cast<volatile std::atomic<s64>*>(target);
  return std::atomic_fetch_sub(t, value);
}

// AtomicPreIncrement
s8 AtomicPreIncrement(volatile s8* target)
{
  return AtomicFetchAdd(target, s8(1)) + s8(1);
}

s16 AtomicPreIncrement(volatile s16* target)
{
  return AtomicFetchAdd(target, s16(1)) + s16(1);
}

s32 AtomicPreIncrement(volatile s32* target)
{
  return AtomicFetchAdd(target, s32(1)) + s32(1);
}

s64 AtomicPreIncrement(volatile s64* target)
{
  return AtomicFetchAdd(target, s64(1)) + s64(1);
}

// AtomicPostIncrement
s8 AtomicPostIncrement(volatile s8* target)
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
s8 AtomicPreDecrement(volatile s8* target)
{
  return AtomicFetchSubtract(target, s8(1)) - s8(1);
}

s16 AtomicPreDecrement(volatile s16* target)
{
  return AtomicFetchSubtract(target, s16(1)) - s16(1);
}

s32 AtomicPreDecrement(volatile s32* target)
{
  return AtomicFetchSubtract(target, s32(1)) - s32(1);
}

s64 AtomicPreDecrement(volatile s64* target)
{
  return AtomicFetchSubtract(target, s64(1)) - s64(1);
}

// AtomicPostDecrement
s8 AtomicPostDecrement(volatile s8* target)
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
