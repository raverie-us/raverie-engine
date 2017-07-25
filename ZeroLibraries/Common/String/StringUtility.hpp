///////////////////////////////////////////////////////////////////////////////
///
/// \file StringUtility.hpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/String.hpp"
#include "Containers/Array.hpp"

namespace Zero
{

//----------------------------------------------------------------------- String
bool CaseInsensitiveStringLess(StringParam a, StringParam b);

// Split string into to two parts not including the delimiter
// if the delimiter is not found it will return an empty range as the second range.
Pair<StringRange,StringRange> SplitOnFirst(StringRange filename, Rune delimiter);

// Split string into to two parts not including the delimiter
// if the delimiter is not found it will return an empty range as the second range.
Pair<StringRange,StringRange> SplitOnLast(StringRange filename, Rune delimiter);

// Strip all of string before last of delimiter
StringRange StripBeforeLast(StringRange filename, char delimiter);

//Join strings together using a delimiter
String JoinStrings(const Array<String>& strings, StringParam delimiter);

char OnlyAlphaNumeric(char c);

// Fills a given string array with all permutations of a given string
void Permute(StringParam src, Array<String>& perms);

// Fills a given string array with all permutations of a given string
void SuperPermute(StringParam src, Array<String>& perms);

template<typename transformFunc>
String TransformString(StringRange string, transformFunc f)
{
  uint size = string.SizeInBytes();
  uint bufferSize = size+1;
  char* buffer = (char*)alloca(bufferSize);
  ZeroStrCpy(buffer, bufferSize, string.Data());
  for (size_t i = 0; i < size; ++i)
    buffer[i] = f(buffer[i]);
  buffer[size] = '\0';
  return buffer;
}

template<typename Predicate>
StringRange RangeUntilFirst(StringParam string, Predicate predicate)
{
  StringRange stringRange = string.All();
  for (;!stringRange.Empty(); stringRange.PopFront())
  {
    //we want to include the character the string range is on so + 1
    if(predicate(stringRange.Front()))
      return string.SubString(string.Begin(), stringRange.Begin() + 1);
  }
  return string.All();
}

}//namespace Zero
