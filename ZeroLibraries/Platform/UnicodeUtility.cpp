///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- UTF8 Utilities
namespace UTF8
{
static const char cTotalBytesToReadUTF8[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4
};
// the offset values are bit shifted by 6 positions so the markers do no line up with the start of each byte
static const int cOffsetsFromUTF8[5] = { 0x00000000,  // bytes read will (should) never be 0
                                         0x00000000,  // 0000 0000 | 0000 0000 | 0000 0000 | 0000 0000
                                         0x00003080,  // 0000 0000 | 0000 0000 | 0011 0000 | 1000 0000
                                         0x000E2080,  // 0000 0000 | 0000 1110 | 0010 0000 | 1000 0000
                                         0x03C82080 };// 0011 1100 | 0000 1000 | 0010 0000 | 1000 0000

size_t UnpackUtf8RuneIntoBufferInternal(Rune uft8Rune, byte(&utf8Bytes)[4]);

bool IsLower(Rune rune)
{
// temporary fix for windows lack of any real UTF8 support
  if (rune.value > 0xC0)
    return false;
  return islower(rune.value);
}

bool IsUpper(Rune rune)
{
// temporary fix for windows lack of any real UTF8 support
  if (rune.value > 0xC0)
    return false;
  return isupper(rune.value);
}

bool IsWhiteSpace(Rune rune)
{
// temporary fix for windows lack of any real UTF8 support
  if(rune.value > 0xC0)
    return false;
  return isspace(rune.value);
}

bool IsAlphaNumeric(Rune rune)
{
  // temporary fix for windows lack of any real UTF8 support
  if (rune.value > 0xC0)
    return false;
  return isalnum(rune.value);
}

Rune ToLower(Rune rune)
{
  return Rune(tolower(rune.value));
}

Rune ToUpper(Rune rune)
{
  return Rune(toupper(rune.value));
}

// Does not validate rune 
uint Utf8ToUtf32(Rune utf8)
{
  if (utf8.value < 128)
    return utf8.value;

  uint utf32 = 0;

  // separate out the uft8 codepoints into separate bytes
  byte utf8Bytes[4];

  int totalBytesToRead = UnpackUtf8RuneIntoBufferInternal(utf8, utf8Bytes);

  char currentByteToRead = totalBytesToRead - 1;
  // this switch case falling through is intended behavior, it is simply an
  // unwrapped while loop with the removed case of not bit shifting on the first byte
  switch (currentByteToRead)
  {
    case 3: utf32 += utf8Bytes[currentByteToRead--]; utf32 <<= 6;
    case 2: utf32 += utf8Bytes[currentByteToRead--]; utf32 <<= 6;
    case 1: utf32 += utf8Bytes[currentByteToRead--]; utf32 <<= 6;
    case 0: utf32 += utf8Bytes[currentByteToRead];
  }

  // subtract out the utf8 information byte values which are the lead bytes value and the following
  // continuation byte markers
  utf32 -= cOffsetsFromUTF8[totalBytesToRead];
  
  return utf32;
}

// return the total bytes read
size_t UnpackUtf8RuneIntoBufferInternal(Rune uft8Rune, byte (&utf8Bytes)[4])
{
  memcpy(utf8Bytes, &uft8Rune.value, sizeof(uint));

  for (uint i = 3; i >= 0; --i)
  {
    // either we found the leading byte value or reached the end of the codepoint
    if (utf8Bytes[i] || i == 0)
     return EncodedCodepointLength(utf8Bytes[i]);
  }
  // this should never be hit, but just in case
  return 0;
}

size_t UnpackUtf8RuneIntoBuffer(Rune uft8Rune, byte(&utf8Bytes)[4])
{
  memset(utf8Bytes, 0, sizeof(uint));
  byte bytes[4];
  int bytesRead = UnpackUtf8RuneIntoBufferInternal(uft8Rune, bytes);

  for (int i = 0, utf8Pos = bytesRead - 1; utf8Pos >= 0; ++i, --utf8Pos)
    utf8Bytes[i] = bytes[utf8Pos];

  return bytesRead;
}

// Only pass in valid pointers to read, no internal guarantees
Rune ReadUtf8Rune(byte* firstByte)
{
  int totalBytesToRead = EncodedCodepointLength(firstByte[0]);
  int currentByteToRead = 0;
  int rune = 0;

  do {
    // bit shift the character into the byte started furtherest left 
    // and counting down until the next byte starts
    rune = rune << 8;
    rune |= firstByte[currentByteToRead++];
  } while (currentByteToRead < totalBytesToRead);

  return Rune(rune);
}

//Only pass in the first utf8 byte to get how long the encoded codepoint is in bytes
size_t EncodedCodepointLength(byte utf8FirstByte)
{
  return cTotalBytesToReadUTF8[utf8FirstByte];
}

}// namespace UTF8

}// namespace Zero
