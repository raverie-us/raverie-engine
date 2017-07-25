///////////////////////////////////////////////////////////////////////////////
///
/// \file Hashing.hpp
/// HahsedContainer Container used to implement of HashMap and HashSet.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "TypeTraits.hpp"
#include "ContainerCommon.hpp"

namespace Zero
{

size_t HashString(const char* str, size_t size);

inline size_t HashUint(size_t a)
{
  a = (a ^ 61) ^ (a >> 16);
  a = a + (a << 3);
  a = a ^ (a >> 4);
  a = a * 0x27d4eb2d;
  a = a ^ (a >> 15);
  return a;
}

//From Thomas Wang, Jan 1997
inline size_t Hash64to32Shift(u64 key)
{
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (uint) key;
}

//
// has_hash_function
//

/// has_hash_function_helper helper class
template<typename T>
struct ZeroSharedTemplate has_hash_function_helper
{
  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename size_t(T2::*)() const, &T2::Hash >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a hash function, else defined as false
template<typename T>
struct ZeroSharedTemplate has_hash_function : public integral_constant<bool, has_hash_function_helper<T>::value> {};

//
// HashPolicy
//

/// Policy for how values are hashed
template <typename T, typename Enable = void>
struct ZeroSharedTemplate HashPolicy
{
  // (Dummy operator() required for has_valid_hash_policy<T> to compile properly)
  inline void operator()(void)
  {
  }
};

/// Default hash policy
/// Available for instantiation if T has an accessible hash function
template <typename T>
struct ZeroSharedTemplate HashPolicy<T, TC_ENABLE_IF(has_hash_function<T>::value)> : public ComparePolicy<T>
{
public:
  typedef HashPolicy<T> this_type;

  // Calls: size_t T::Hash() const;
  inline size_t operator()(const T& value) const
  {
    // Default behavior is to expect the type to have a hashing member function
    return (size_t)value.Hash();
  }
};

/// HashPolicy for bool
template<>
struct ZeroShared HashPolicy<bool> : public ComparePolicy<bool>
{
  inline size_t operator()(const bool& value) const
  {
    size_t valuePromoted = (size_t)value;
    return HashUint(valuePromoted);
  }
};

/// HashPolicy for short
template<>
struct ZeroShared HashPolicy<short> : public ComparePolicy<int>
{
  inline size_t operator()(const short& value) const
  {
    size_t valuePromoted = (size_t)value;
    return HashUint(valuePromoted);
  }
};

/// HashPolicy for int
template<>
struct ZeroShared HashPolicy<int> : public ComparePolicy<int>
{
  inline size_t operator()(const int& value) const
  {
    return HashUint(*(unsigned int*)&value);
  }
};

/// HashPolicy for unsigned int
template<>
struct ZeroShared HashPolicy<unsigned int> : public ComparePolicy<unsigned int>
{
  inline size_t operator()(const unsigned int& value) const
  {
    return HashUint(*(unsigned int*)&value);
  }
};

/// HashPolicy for long
template<>
struct ZeroShared HashPolicy<long> : public ComparePolicy<long>
{
  inline size_t operator()(const long& value) const
  {
    return HashUint((size_t)value);
  }
};

/// HashPolicy for unsigned long
template<>
struct ZeroShared HashPolicy<unsigned long> : public ComparePolicy<unsigned long>
{
  inline size_t operator()(const unsigned long& value) const
  {
    return HashUint((size_t)value);
  }
};

/// HashPolicy for pointers
template<typename type>
struct ZeroSharedTemplate HashPolicy<type*> : public ComparePolicy<type*>
{
  inline size_t operator()(const type* value) const
  {
    return HashUint(*(unsigned int*)&value);
  }
};

/// HashPolicy for float
template<>
struct ZeroShared HashPolicy<float> : public ComparePolicy<float>
{
  inline size_t operator()(const float& value) const
  {
    return HashUint(*(unsigned int*)&value);
  }
};

/// HashPolicy for const char*
template<>
struct ZeroShared HashPolicy<const char*> : public ComparePolicy<const char*>
{
  inline size_t operator()(const char* value) const
  {
    return HashString(value, strlen(value));
  }
};

/// HashPolicy for u64
template<>
struct ZeroShared HashPolicy<u64> : public ComparePolicy<u64>
{
  inline size_t operator()(const u64& value) const
  {
    return Hash64to32Shift(value);
  }
};

/// HashPolicy for Guid
template<>
struct ZeroShared HashPolicy<Guid> : public ComparePolicy<Guid>
{
  inline size_t operator()(const Guid& value) const
  {
    return Hash64to32Shift(value.mValue);
  }
};

/// HashPolicy for s64
template<>
struct ZeroShared HashPolicy<s64> : public ComparePolicy<s64>
{
  inline size_t operator()(const s64& value) const
  {
    return Hash64to32Shift((u64)value);
  }
};

//
// Pair::Hash
//

/// Implementation of Pair's Hash function (relies on HashPolicy, so must be defined here)
template<typename type0, typename type1>
size_t Pair<type0, type1>::Hash() const
{
  return HashPolicy<first_type>()(first) ^ HashPolicy<second_type>()(second) * 7187;
}

//
// has_valid_hash_policy
//

/// has_valid_hash_policy_helper helper class
template<typename T>
struct ZeroSharedTemplate has_valid_hash_policy_helper
{
  typedef typename HashPolicy<T> HashPolicyT;

  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename size_t(HashPolicyT::*)(const T2&) const, &HashPolicyT::operator() >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a valid hash policy, else defined as false
/// A hash policy is valid if it has a function callable as: size_t operator()(const T& value) const;
template<typename T>
struct ZeroSharedTemplate has_valid_hash_policy : public integral_constant<bool, has_valid_hash_policy_helper<T>::value> {};

} // namespace Zero
