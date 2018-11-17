///////////////////////////////////////////////////////////////////////////////
///
/// \file PartialMatch.cpp
/// Implementation of the PartialMatch.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
String StringContainerAdapter(StringParam element)
{
  return element;
}

bool MatchesAcronym(StringRangeParam text, StringRangeParam acronym)
{
  // First, check to make sure that the acronym is actually an acronym
  // The acronym must be two or more letters and all uppercase
  if (acronym.ComputeRuneCount() <= 1 || !acronym.IsAllUpper())
    return false;

  StringIterator acronymCur = acronym.Begin();

  // We're going to walk over all the text we're matching looking for uppercase letters
  StringIterator end = text.End();
  for (StringIterator textCur = text.Begin(); textCur < end; ++textCur)
  {
    Rune textRune = textCur.ReadCurrentRune();

    // If the text character is uppercase...
    if (textCur.IsUpper())
    {
      Rune acronymRune = acronymCur.ReadCurrentRune();

      // If we differed in character, then there's no match
      if (textRune != acronymRune)
      {
        return false;
      }

      ++acronymCur;

      // If we reached the end of the acronym and everything matched...
      if (acronymCur.Empty())
      {
        // We have a match! (though there may be more text later...)
        return true;
      }
    }
  }

  // If we got here, the acronym wasn't exhausted so there was no match
  return false;
}

bool MatchesPartial(StringParam text, StringParam partial, RuneComparer compare)
{
  // We can't possibly match if the partial text is longer than our own
  if (partial.SizeInBytes() > text.SizeInBytes())
    return false;
  
  StringIterator textEnd = text.End();
  StringIterator partialEnd = partial.End();

  for (StringIterator textCur = text.Begin(); textCur < textEnd; ++textCur)
  {
    StringIterator textTemp = textCur;
    for (StringIterator partialCur = partial.Begin(); partialCur <= partialEnd; ++partialCur, ++textTemp)
    {
      // If we reached the end of the partial string, then we match!
      if (partialCur == partialEnd)
        return true;
      
      // If the partial match is going to start matching outside the original text string, then we stop
      if (textTemp >= textEnd)
        return false;
      
      if (!compare(textTemp.ReadCurrentRune(), partialCur.ReadCurrentRune()))
      {
        break;
      }
    }
  }

  return false;
}


}//namespace Zero
