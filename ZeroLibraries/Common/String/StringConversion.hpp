///////////////////////////////////////////////////////////////////////////////
///
/// \file StringConversion.hpp
/// Conversion to and from strings.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/Typedefs.hpp"
#include "FixedString.hpp"
#include "String.hpp"

namespace Zero
{

// Convert Integers into hexadecimal format "4a34"
uint WriteToHex(char* buffer, uint bufferSize, u64 integerValue, bool exclude0x = false);
uint WriteToHex(char* buffer, uint bufferSize, u32 integerValue, bool exclude0x = false);
StringRange StripHex0x(StringRange hexString);
bool StringStartsWith0x(StringRange hexString);
const uint cHex64Size = 16;

// Write out 'places' number of hex digits from integerValue
uint WriteToHexSize(char* buffer, uint bufferSize, uint places, u64 integerValue, bool exclude0x = false);

// Read 64 bit hex string
Guid ReadHexString(StringRange range);

//Basic conversion function (input must be UTF-16/2) DestAscii must unicodeLength +1 (for null terminator)
void ConvertUnicodeToAscii(char* destAscii, uint destAsciiLength, const wchar_t* unicodeData, size_t unicodeLength);

//
// ToValue Functions
//

/// Reads a value from a character buffer
/// (Takes value by reference so functions can be overloaded)
void ToValue(StringRange range, String& value);
void ToValue(StringRange range, StringRange& value);

void ToValue(StringRange range, bool& value);

void ToValue(StringRange range, char& value);

void ToValue(StringRange range, int8& value, int base = 0);
void ToValue(StringRange range, int16& value, int base = 0);
void ToValue(StringRange range, int32& value, int base = 0);
void ToValue(StringRange range, int64& value, int base = 0);

void ToValue(StringRange range, uint8& value, int base = 0);
void ToValue(StringRange range, uint16& value, int base = 0);
void ToValue(StringRange range, uint32& value, int base = 0);
void ToValue(StringRange range, uint64& value, int base = 0);

void ToValue(StringRange range, float& value);
void ToValue(StringRange range, double& value);

void ToValue(StringRange range, Guid& value);

//
// ToBuffer Functions
//

/// Writes a value to a character buffer
/// Returns the number of bytes written to the buffer
uint ToBuffer(char* buffer, uint bufferSize, String value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, StringRange value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, bool value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, char value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, int8 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, int16 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, int32 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, int64 value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, uint8 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, uint16 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, uint32 value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, uint64 value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, float value, bool shortFormat = false);
uint ToBuffer(char* buffer, uint bufferSize, double value, bool shortFormat = false);

uint ToBuffer(char* buffer, uint bufferSize, Guid value, bool shortFormat = false);

//
// has_global_to_value
//

/// has_global_to_value_helper helper class
template<typename T>
struct ZeroSharedTemplate has_global_to_value_helper
{
  template<typename T2>
  static inline yes Test1(static_verify_function_signature< typename void(*)(StringRange, T2&), &ToValue >*);
  template<typename T2>
  static inline no Test1(...);

  // Third parameter (base) must have a default argument, so ToValue may be invoked without it
  template<typename T2>
  static inline yes Test2(static_verify_function_signature< typename void(*)(StringRange, T2&, int), &ToValue >*);
  template<typename T2>
  static inline no Test2(...);

  static const bool value =  (sizeof(Test1<T>(0)) == sizeof(yes))
                          || (sizeof(Test2<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a global "ToValue" function, else defined as false
template<typename T>
struct ZeroSharedTemplate has_global_to_value : public integral_constant<bool, has_global_to_value_helper<T>::value> {};

//
// has_global_to_buffer
//

/// has_global_to_buffer_helper helper class
template<typename T>
struct ZeroSharedTemplate has_global_to_buffer_helper
{
  // Third parameter (shortFormat) must have a default argument, so ToValue may be invoked without it
  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename uint(*)(char*, uint, T2, bool) , &ToBuffer >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a global "ToBuffer" function, else defined as false
template<typename T>
struct ZeroSharedTemplate has_global_to_buffer : public integral_constant<bool, has_global_to_buffer_helper<T>::value> {};

} // namespace Zero
