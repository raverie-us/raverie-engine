///////////////////////////////////////////////////////////////////////////////
///
/// \file StringUtility.cpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------- Strings
bool CaseInsensitiveStringLess(StringParam a, StringParam b)
{
  StringRange achars = a.All();
  StringRange bchars = b.All();

  while(!achars.Empty() && !bchars.Empty())
  {
    Rune aChar = UTF8::ToLower(achars.Front());
    Rune bChar = UTF8::ToLower(bchars.Front());

    if(aChar < bChar)
      return true;

    if(aChar > bChar)
      return false;

    achars.PopFront();
    bchars.PopFront();
  }

  if(achars.Empty() && !bchars.Empty())
    return true;

  return false;
}

Pair<StringRange,StringRange> SplitOnLast(StringRange input, Rune delimiter)
{
  //With empty just return empty String
  if(input.Empty())
    return Pair<StringRange,StringRange>(input, input);

  uint numRunes = input.ComputeRuneCount();

  StringRange lastOf = input.FindLastOf(delimiter);

  // Delim found return string and empty
  if(lastOf.Empty())
    return Pair<StringRange,StringRange>(input, StringRange());
  
  if(lastOf.SizeInBytes() == 0)
    return Pair<StringRange, StringRange>(StringRange(), input.SubString(input.Begin(), input.End()));
  if(lastOf.SizeInBytes() == numRunes - 1)
    return Pair<StringRange, StringRange>(input.SubString(input.Begin(), input.End()), StringRange());

  return Pair<StringRange, StringRange>(input.SubString(input.Begin(), lastOf.End()), input.SubString(lastOf.Begin() + 1, lastOf.End()));
}

Pair<StringRange,StringRange> SplitOnFirst(StringRange input, Rune delimiter)
{
  StringTokenRange tokenRange(input, delimiter);
  StringRange left =  tokenRange.Front();
  StringRange right = StringRange(left.End(), input.End());
  return Pair<StringRange, StringRange>(left, right);
}

StringRange StripBeforeLast(StringRange input, Rune delimiter)
{
  Pair<StringRange, StringRange> split = SplitOnLast(input, delimiter);

  // If the delimiter was not found the second will be empty
  if(split.second.Empty())
    return input;
  else
    return split.second;
}

String JoinStrings(const Array<String>& strings, StringParam delimiter)
{
  StringBuilder builder;

  for (size_t i = 0; i < strings.Size(); ++i)
  {
    const String& string = strings[i];

    builder.Append(string);

    bool isNotLast = (i + 1 != strings.Size());

    if (isNotLast)
    {
      builder.Append(delimiter);
    }
  }

  return builder.ToString();
}

char OnlyAlphaNumeric(char c)
{
  if (!isalnum(c))
    return '_';
  else
    return c;
}

//******************************************************************************
// Recursive helper for global string Permute below
static void PermuteRecursive(char *src, uint start, uint end, Array<String>& perms)
{
  // finished a permutation, add it to the list
  if (start == end)
  {
    perms.PushBack(src);
    return;
  }

  for (uint i = start; i < end; ++i)
  {
    // swap to get new head
    Swap(src[start], src[i]);
    // permute
    PermuteRecursive(src, start + 1, end, perms);
    // backtrack
    Swap(src[start], src[i]);
  }

}

//******************************************************************************
void Permute(StringParam src, Array<String>& perms)
{
  // convert to std string which is char writable
  uint srclen = src.SizeInBytes();
  const char *csrc = src.c_str();

  // create a temp buffer on the stack to manipulate src
  char *buf = (char *)alloca(srclen + 1);
  memset(buf, 0, srclen + 1);

  // recursively calculate permutations
  PermuteRecursive(buf, 0, srclen, perms);
}

//******************************************************************************
void SuperPermute(StringParam src, Array<String>& perms)
{
  // convert to std string which is char writable
  uint srclen = src.SizeInBytes();
  const char *csrc = src.c_str();

  // create a temp buffer on the stack to manipulate src
  char *buf = (char *)alloca(srclen + 1);
  memset(buf, 0, srclen + 1);

  // push the individual elements of the source
  StringRange srcRange = src;
  for (; !srcRange.Empty(); srcRange.PopFront())
    perms.PushBack(String(srcRange.Front()));

  for (uint l = 1; l < srclen; ++l)
  {
    for (uint i = 0; i + l < srclen; ++i)
    {
      // initialize buffer
      memcpy(buf, csrc + i, l);
      for (uint j = i + l; j < srclen; ++j)
      {
        buf[l] = csrc[j];
        PermuteRecursive(buf, 0, l + 1, perms);
        buf[l] = '\0';
      }
    }
  }

}


}//namespace Zero
