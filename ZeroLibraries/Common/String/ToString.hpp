///////////////////////////////////////////////////////////////////////////////
///
/// \file ToString.hpp
/// Conversion to and from strings.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Containers/TypeTraits.hpp"
#include "Containers/ContainerCommon.hpp"
#include "StringConversion.hpp"

namespace Zero
{

// ToString buffer size
static const int cToStringBufferSize = 256;

//
// has_member_to_string
//

/// has_member_to_string_helper helper class
template<typename T>
struct ZeroSharedTemplate has_member_to_string_helper
{
  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename String(T2::*)(bool) const, &T2::ToString >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a to string function, else defined as false
template<typename T>
struct ZeroSharedTemplate has_member_to_string : public integral_constant<bool, has_member_to_string_helper<T>::value> {};

//
// ToString Functions
//

// Optimized global ToString overloads
ZeroShared inline String ToString(StringParam value, bool shortFormat = false)
{
  return value;
}
ZeroShared inline String ToString(StringRangeParam value, bool shortFormat = false)
{
  return value;
}
ZeroShared inline String ToString(cstr value, bool shortFormat = false)
{
  return value;
}

// Calls member function "ToString" on specified value
// (Enabled for types with a member "ToString" function)
template<typename T, TF_ENABLE_IF(has_member_to_string<T>::value)>
ZeroSharedTemplate String ToString(const T& value, bool shortFormat = false)
{
  return value.ToString(shortFormat);
}
template<typename T, TF_ENABLE_IF(has_member_to_string<T>::value)>
ZeroSharedTemplate String ToString(T*const value, bool shortFormat = false)
{
  return value->ToString(shortFormat);
}

// Calls global function "ToBuffer" with specified value
// (Enabled for types without a member "ToString" function, but with a global "ToBuffer" function)
template<typename T, TF_ENABLE_IF(!has_member_to_string<T>::value && has_global_to_buffer<T>::value)>
ZeroSharedTemplate String ToString(const T& value, bool shortFormat = false)
{
  // Create zeroed character buffer
  // (Note: Ideally this would be statically allocated, but then calling ToString wouldn't be thread safe)
  char buffer[cToStringBufferSize] = {};

  // Write value to character buffer
  ToBuffer(buffer, cToStringBufferSize, value, shortFormat);
  return buffer;
}

//
// has_global_to_string
//

/// has_global_to_string_helper helper class
template<typename T>
struct ZeroSharedTemplate has_global_to_string_helper
{
  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename String(*)(const T2&, bool), &ToString >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a global to string function, else defined as false
template<typename T>
struct ZeroSharedTemplate has_global_to_string : public integral_constant<bool, has_global_to_string_helper<T>::value> {};

} // namespace Zero
