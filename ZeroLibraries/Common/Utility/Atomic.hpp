///////////////////////////////////////////////////////////////////////////////
///
/// \file Atomic.hpp
/// Declaration of the atomic functions.
///
/// Authors: Chris Peters, Andrew Colean
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "BitMath.hpp"

namespace Zero
{

//
// Atomic Operations
//

/// Atomic operations provide fast, lockless, indivisible, thread-safe operations
/// These methods are used to implement multithreaded lock-free data structures with well defined behavior

/// Sets the value stored in target
void AtomicStore(volatile s8*  target, s8  value);
void AtomicStore(volatile s16* target, s16 value);
void AtomicStore(volatile s32* target, s32 value);
void AtomicStore(volatile s64* target, s64 value);

/// Returns the value stored in target
s8  AtomicLoad(volatile s8*  target);
s16 AtomicLoad(volatile s16* target);
s32 AtomicLoad(volatile s32* target);
s64 AtomicLoad(volatile s64* target);

/// Sets the value stored in target
/// Returns the previous value stored in target
s8  AtomicExchange(volatile s8*  target, s8  value);
s16 AtomicExchange(volatile s16* target, s16 value);
s32 AtomicExchange(volatile s32* target, s32 value);
s64 AtomicExchange(volatile s64* target, s64 value);

/// Sets the value stored in target if target is bit-wise equal to comparison
/// Returns the previous value stored in target
s8  AtomicCompareExchange(volatile s8*  target, s8  value, s8  comparison);
s16 AtomicCompareExchange(volatile s16* target, s16 value, s16 comparison);
s32 AtomicCompareExchange(volatile s32* target, s32 value, s32 comparison);
s64 AtomicCompareExchange(volatile s64* target, s64 value, s64 comparison);

/// Sets the value stored in target if target is bit-wise equal to comparison
/// Returns true if the exchange took place, else false
bool AtomicCompareExchangeBool(volatile s8*  target, s8  value, s8  comparison);
bool AtomicCompareExchangeBool(volatile s16* target, s16 value, s16 comparison);
bool AtomicCompareExchangeBool(volatile s32* target, s32 value, s32 comparison);
bool AtomicCompareExchangeBool(volatile s64* target, s64 value, s64 comparison);

/// Adds the specified value to the value stored in target
/// Returns the previous value stored in target
s8  AtomicFetchAdd(volatile s8*  target, s8  value);
s16 AtomicFetchAdd(volatile s16* target, s16 value);
s32 AtomicFetchAdd(volatile s32* target, s32 value);
s64 AtomicFetchAdd(volatile s64* target, s64 value);

/// Subtracts the specified value from the value stored in target
/// Returns the previous value stored in target
s8  AtomicFetchSubtract(volatile s8*  target, s8  value);
s16 AtomicFetchSubtract(volatile s16* target, s16 value);
s32 AtomicFetchSubtract(volatile s32* target, s32 value);
s64 AtomicFetchSubtract(volatile s64* target, s64 value);

/// Pre-increments the value stored in target
/// Returns the current value stored in target
s8  AtomicPreIncrement(volatile s8*  target);
s16 AtomicPreIncrement(volatile s16* target);
s32 AtomicPreIncrement(volatile s32* target);
s64 AtomicPreIncrement(volatile s64* target);

/// Post-increments the value stored in target
/// Returns the previous value stored in target
s8  AtomicPostIncrement(volatile s8*  target);
s16 AtomicPostIncrement(volatile s16* target);
s32 AtomicPostIncrement(volatile s32* target);
s64 AtomicPostIncrement(volatile s64* target);

/// Pre-decrements the value stored in target
/// Returns the current value stored in target
s8  AtomicPreDecrement(volatile s8*  target);
s16 AtomicPreDecrement(volatile s16* target);
s32 AtomicPreDecrement(volatile s32* target);
s64 AtomicPreDecrement(volatile s64* target);

/// Post-decrements the value stored in target
/// Returns the previous value stored in target
s8  AtomicPostDecrement(volatile s8*  target);
s16 AtomicPostDecrement(volatile s16* target);
s32 AtomicPostDecrement(volatile s32* target);
s64 AtomicPostDecrement(volatile s64* target);

//
// Atomic Primitives
//

/// Atomic access object
/// Provides lockless thread-safe access to a primitive type
template <typename T, typename Enable = void>
class Atomic;

/// Integral specialization
/// Able to provide atomic arithmetic
template <typename T>
class Atomic<T, TC_ENABLE_IF(is_integral<T>::value && !is_bool<T>::value)>
{
public:
  /// Typedefs
  typedef typename remove_reference_const_and_volatile<T>::type value_type;
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(value_type)))          underlying_type;

  /// Constructors
  Atomic()                 : mValue()                                          {}
  Atomic(value_type value) : mValue(reinterpret_cast<underlying_type&>(value)) {}

  /// Assignment Operator
  /// Sets the current value (equivalent to Store())
  value_type operator =(value_type value)          { Store(value); return value; }
  value_type operator =(value_type value) volatile { Store(value); return value; }

  /// Conversion Operator
  /// Returns the current value (equivalent to Load())
  operator value_type() const          { return Load(); }
  operator value_type() const volatile { return Load(); }

  /// Sets the current value
  void Store(value_type value)          { AtomicStore(&mValue, reinterpret_cast<underlying_type&>(value)); }
  void Store(value_type value) volatile { AtomicStore(&mValue, reinterpret_cast<underlying_type&>(value)); }

  /// Returns the current value
  value_type Load() const          { underlying_type result = AtomicLoad(const_cast<underlying_type*>(&mValue)); return reinterpret_cast<value_type&>(result); }
  value_type Load() const volatile { underlying_type result = AtomicLoad(const_cast<underlying_type*>(&mValue)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value
  /// Returns the previous value
  value_type Exchange(value_type value)          { underlying_type result = AtomicExchange(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }
  value_type Exchange(value_type value) volatile { underlying_type result = AtomicExchange(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value if it is bit-wise equal to comparison
  /// Returns the previous value
  value_type CompareExchange(value_type value, value_type comparison)          { underlying_type result = AtomicCompareExchange(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); return reinterpret_cast<value_type&>(result); }
  value_type CompareExchange(value_type value, value_type comparison) volatile { underlying_type result = AtomicCompareExchange(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value if it is bit-wise equal to comparison
  /// Returns true if the exchange took place, else false
  bool CompareExchangeBool(value_type value, value_type comparison)          { return AtomicCompareExchangeBool(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); }
  bool CompareExchangeBool(value_type value, value_type comparison) volatile { return AtomicCompareExchangeBool(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); }

  //
  // Arithmetic Operations
  //

  /// Adds the specified value to the current value
  /// Returns the previous value
  value_type FetchAdd(value_type value)          { underlying_type result = AtomicFetchAdd(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }
  value_type FetchAdd(value_type value) volatile { underlying_type result = AtomicFetchAdd(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }

  /// Subtracts the specified value to the current value
  /// Returns the previous value
  value_type FetchSubtract(value_type value)          { underlying_type result = AtomicFetchSubtract(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }
  value_type FetchSubtract(value_type value) volatile { underlying_type result = AtomicFetchSubtract(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }

  /// Adds the specified value to the current value (equivalent to FetchAdd())
  /// Returns the current value
  value_type operator +=(value_type value)          { return FetchAdd(value); }
  value_type operator +=(value_type value) volatile { return FetchAdd(value); }

  /// Subtracts the specified value to the current value (equivalent to FetchSubtract())
  /// Returns the current value
  value_type operator -=(value_type value)          { return FetchSubtract(value); }
  value_type operator -=(value_type value) volatile { return FetchSubtract(value); }

  /// Pre-increments the value
  /// Returns the current value
  value_type operator ++()          { underlying_type result = AtomicPreIncrement(&mValue); return reinterpret_cast<value_type&>(result); }
  value_type operator ++() volatile { underlying_type result = AtomicPreIncrement(&mValue); return reinterpret_cast<value_type&>(result); }

  /// Post-increments the value
  /// Returns the previous value
  value_type operator ++(int)          { underlying_type result = AtomicPostIncrement(&mValue); return reinterpret_cast<value_type&>(result); }
  value_type operator ++(int) volatile { underlying_type result = AtomicPostIncrement(&mValue); return reinterpret_cast<value_type&>(result); }

  /// Pre-decrements the value
  /// Returns the current value
  value_type operator --()          { underlying_type result = AtomicPreDecrement(&mValue); return reinterpret_cast<value_type&>(result); }
  value_type operator --() volatile { underlying_type result = AtomicPreDecrement(&mValue); return reinterpret_cast<value_type&>(result); }

  /// Post-decrements the value
  /// Returns the previous value
  value_type operator --(int)          { underlying_type result = AtomicPostDecrement(&mValue); return reinterpret_cast<value_type&>(result); }
  value_type operator --(int) volatile { underlying_type result = AtomicPostDecrement(&mValue); return reinterpret_cast<value_type&>(result); }

private:
  /// No Copy Constructor
  Atomic(const Atomic&);
  /// No Copy Assignment Operator
  Atomic& operator =(const Atomic&);

  /// Value object
  volatile underlying_type mValue;
};

/// Floating-point and boolean specialization
/// Unable to provide atomic arithmetic
template <typename T>
class Atomic<T, TC_ENABLE_IF(is_floating_point<T>::value || is_bool<T>::value)>
{
public:
  /// Typedefs
  typedef typename remove_reference_const_and_volatile<T>::type value_type;
  typedef EXACT_INT(BYTES_TO_BITS(sizeof(value_type)))          underlying_type;

  /// Constructors
  Atomic()                 : mValue()                                          {}
  Atomic(value_type value) : mValue(reinterpret_cast<underlying_type&>(value)) {}

  /// Assignment Operator
  /// Sets the current value (equivalent to Store())
  value_type operator =(value_type value)          { Store(value); return value; }
  value_type operator =(value_type value) volatile { Store(value); return value; }

  /// Conversion Operator
  /// Returns the current value (equivalent to Load())
  operator value_type() const          { return Load(); }
  operator value_type() const volatile { return Load(); }

  /// Sets the current value
  void Store(value_type value)          { AtomicStore(&mValue, reinterpret_cast<underlying_type&>(value)); }
  void Store(value_type value) volatile { AtomicStore(&mValue, reinterpret_cast<underlying_type&>(value)); }

  /// Returns the current value
  value_type Load() const          { underlying_type result = AtomicLoad(const_cast<underlying_type*>(&mValue)); return reinterpret_cast<value_type&>(result); }
  value_type Load() const volatile { underlying_type result = AtomicLoad(const_cast<underlying_type*>(&mValue)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value
  /// Returns the previous value
  value_type Exchange(value_type value)          { underlying_type result = AtomicExchange(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }
  value_type Exchange(value_type value) volatile { underlying_type result = AtomicExchange(&mValue, reinterpret_cast<underlying_type&>(value)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value if it is bit-wise equal to comparison
  /// Returns the previous value
  value_type CompareExchange(value_type value, value_type comparison)          { underlying_type result = AtomicCompareExchange(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); return reinterpret_cast<value_type&>(result); }
  value_type CompareExchange(value_type value, value_type comparison) volatile { underlying_type result = AtomicCompareExchange(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); return reinterpret_cast<value_type&>(result); }

  /// Sets the current value if it is bit-wise equal to comparison
  /// Returns true if the exchange took place, else false
  bool CompareExchangeBool(value_type value, value_type comparison)          { return AtomicCompareExchangeBool(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); }
  bool CompareExchangeBool(value_type value, value_type comparison) volatile { return AtomicCompareExchangeBool(&mValue, reinterpret_cast<underlying_type&>(value), reinterpret_cast<underlying_type&>(comparison)); }

private:
  /// No Copy Constructor
  Atomic(const Atomic&);
  /// No Copy Assignment Operator
  Atomic& operator =(const Atomic&);

  /// Value object
  volatile underlying_type mValue;
};

} // namespace Zero
