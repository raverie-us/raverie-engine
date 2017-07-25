///////////////////////////////////////////////////////////////////////////////
///
/// \file StringConversion.cpp
/// Used to convert strings into values.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool StringStartsWith0x(StringRange hexString)
{
  return (hexString.SizeInBytes() > 1 && hexString.Data()[0] == '0' && hexString.Data()[1] == 'x');
}

#define TextTrue "true"
#define TextFalse "false"

StringRange StripHex0x(StringRange hexString)
{
  if(StringStartsWith0x(hexString))
  {
    hexString.PopFront();
    hexString.PopFront();
  }

  return hexString;
}

Guid ReadHexString(StringRange range)
{
  // Skip the "0x" if it exists
  range = StripHex0x(range);

  u64 result = 0;
  for(int i = 0; !range.Empty(); range.PopBack(), ++i)
  {
    // Process the string in reverse
    Rune r = range.Back();
    u64 val = 0;
    if(r >= '0' && r <= '9')
      val = r.value - '0';
    else if(r >= 'a' && r <= 'f')
      val = r.value - 'a' + 10;
    else if(r >= 'A' && r <= 'F')
      val = r.value - 'A' + 10;
    result += val << i * 4;
  }
  return (Guid)result;
}

uint WriteToHexSize(char* buffer, uint bufferSize, uint places, u64 integerValue, bool exclude0x)
{
  // + 2 for '0x' at the start of the hex string if we're including the '0x'
  uint offset0x = exclude0x ? 0 : 2;
  if(bufferSize < places + 1 + offset0x)
    return 0;

  // All hex values should have an '0x' at the start to signify that they're in hex
  if(!exclude0x)
  {
    buffer[0] = '0';
    buffer[1] = 'x';
  }

  for(uint i = 0; i < places; ++i)
  {
    // Get the value of the right most hex value
    uint indexVal = uint(integerValue & 0x0000000F);

    // Convert the hex value to the correct character
    uint charVal = 0;
    if(indexVal <= 9)
      charVal = indexVal + '0';
    else if(10 <= indexVal && indexVal <= 16)
      charVal = indexVal + ('a'-10);

    // Store the character in the buffer (we're processing back to front, so start at the back)
    buffer[places - 1 - i + offset0x] = (char)charVal;

    // Process the next hex value (the next 4 bits)
    integerValue = integerValue >> 4;
  }

  // Null terminate
  buffer[places + offset0x] = '\0';
  return places + offset0x;
}

uint WriteToHex(char* buffer, uint bufferSize, u64 integerValue, bool exclude0x)
{
  return WriteToHexSize(buffer, bufferSize, cHex64Size, integerValue, exclude0x);
}

uint WriteToHex(char* buffer, uint bufferSize, u32 integerValue, bool exclude0x)
{
  return WriteToHexSize(buffer, bufferSize, 8, (u64)integerValue, exclude0x);
}

//Max 4294967295
const uint cMaxIntSize = 12;

void ReverseString(char* start, char* end)
{
  --end;
  while(start<end)
  {
    Swap(*start, *end);
    ++start;
    --end;
  }
}

String ReverseString(StringParam string)
{
  size_t size = string.SizeInBytes();

  // Allocate a buffer on the stack
  char* reverseString = (char*)alloca(size * sizeof(char));

  // Null terminate the buffer
  reverseString[size] = '\0';

  // Copy the string into the reverse string
  memcpy(reverseString, (void*)string.Data(), size * sizeof(char));

  // Reverse the string
  ReverseString(reverseString, reverseString + size);

  return String(reverseString);
}

uint ToString(char* buffer, uint bufferSize, s64 value)
{
  if(bufferSize < cMaxIntSize)
    return 0;

  bool valueIsNegative = false;
  if(value < 0)
  {
    value = -value;
    valueIsNegative = true;
  }

  uint index = 0;
  do 
  {
    char c = value % 10 + '0';
    buffer[index] = c;
    ++index;
    value /= 10;
  } while (value!=0);

  if(valueIsNegative)
  {
    buffer[index] = '-';
    ++index;
  }

  buffer[index] = '\0';
  ReverseString(buffer, buffer+index);
  return index;
}

bool IsCharacter(char c, cstr search)
{
  while(*search!=0)
  {
    if(c == *search)
      return true;
    ++search;
  }
  return false;
}

//
// ToValue Functions
//

void ToValue(StringRange range, String& value)
{
  value = range;
}
void ToValue(StringRange range, StringRange& value)
{
  value = range;
}

void ToValue(StringRange range, bool& value)
{
  // True if string contains 'true', 'True', or '1', else false
  value = (IsCharacter(range.Front().value, "tT1"));
}

void ToValue(StringRange range, char& value)
{
  if(range.Empty())
    value = char(0);
  else
    value = range.mBegin[0];
}

void ToValue(StringRange range, int8& value, int base)
{
  value = (int8)strtol(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, int16& value, int base)
{
  value = (int16)strtol(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, int32& value, int base)
{
  value = (int32)strtol(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, int64& value, int base)
{
  value = (int64)strtoll(range.mBegin, nullptr, base);
}

void ToValue(StringRange range, uint8& value, int base)
{
  value = (uint8)strtoul(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, uint16& value, int base)
{
  value = (uint16)strtoul(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, uint32& value, int base)
{
  value = (uint32)strtoul(range.mBegin, nullptr, base);
}
void ToValue(StringRange range, uint64& value, int base)
{
  value = (uint64)strtoull(range.mBegin, nullptr, base);
}

void ToValue(StringRange range, float& value)
{
  value = (float)atof(range.mBegin);
}
void ToValue(StringRange range, double& value)
{
  value = (double)atof(range.mBegin);
}

void ToValue(StringRange range, Guid& value)
{
  value = ReadHexString(range);
}

//
// ToBuffer Functions
//

uint ToBuffer(char* buffer, uint bufferSize, String value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%s", value.c_str());
}
uint ToBuffer(char* buffer, uint bufferSize, StringRange value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%s", value.mBegin);
}

uint ToBuffer(char* buffer, uint bufferSize, bool value, bool shortFormat)
{
  if(value)
  {
    ZeroStrCpy(buffer, bufferSize, TextTrue);
    return sizeof(TextTrue) - 1;
  }
  else
  {
    ZeroStrCpy(buffer, bufferSize, TextFalse);
    return sizeof(TextFalse) - 1;
  }
}

uint ToBuffer(char* buffer, uint bufferSize, char value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%c", value);
}

uint ToBuffer(char* buffer, uint bufferSize, int8 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%hhd", value);
}
uint ToBuffer(char* buffer, uint bufferSize, int16 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%hd", value);
}
uint ToBuffer(char* buffer, uint bufferSize, int32 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%d", value);
}
uint ToBuffer(char* buffer, uint bufferSize, int64 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%lld", value);
}

uint ToBuffer(char* buffer, uint bufferSize, uint8 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%hhu", value);
}
uint ToBuffer(char* buffer, uint bufferSize, uint16 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%hu", value);
}
uint ToBuffer(char* buffer, uint bufferSize, uint32 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%u", value);
}
uint ToBuffer(char* buffer, uint bufferSize, uint64 value, bool shortFormat)
{
  return ZeroSPrintf(buffer, bufferSize, "%llu", value);
}

uint ToBuffer(char* buffer, uint bufferSize, float value, bool shortFormat)
{
  if(shortFormat)
    return ZeroSPrintf(buffer, bufferSize, "%g", value);
  else
    return ZeroSPrintf(buffer, bufferSize, "%.9g", value);
}
uint ToBuffer(char* buffer, uint bufferSize, double value, bool shortFormat)
{
  if(shortFormat)
    return ZeroSPrintf(buffer, bufferSize, "%f", value);
  else
    return ZeroSPrintf(buffer, bufferSize, "%.9f", value);
}

uint ToBuffer(char* buffer, uint bufferSize, Guid value, bool shortFormat)
{
  // 16 characters for Hex, 2 for the '0x' at the start
  const uint cMinSize = cHex64Size + 2;
  ErrorIf(bufferSize < cMinSize, "Buffer is not large enough for hex value.");
  if(bufferSize > cMinSize)
    return WriteToHex(buffer, bufferSize, value.mValue, shortFormat);
  else
    return 0;
}

//Basic conversion function (input must be UTF-16/2) DestAscii must be unicodeLength +1
void ConvertUnicodeToAscii(char* destAscii, uint bufferSize, 
                           const wchar_t* unicodeData, size_t unicodeLength)
{
  if(bufferSize < unicodeLength +1)
  {
    ErrorIf(true, "Ascii Buffer is not large enough.");
    destAscii[0] = '\0';
  }
  else
  {
    for(uint i=0;i<unicodeLength;++i)
    {
      if(unicodeData[i] > 128)
      {
        //Can not be display in ascii
        destAscii[i] = '?';
      }
      else
      {
        destAscii[i] = (char)unicodeData[i];
      }
    }
    destAscii[unicodeLength] = '\0';
  }
}

} // namespace Zero
