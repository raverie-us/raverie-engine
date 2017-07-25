///////////////////////////////////////////////////////////////////////////////
///
/// \file StringReplacement.cpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "String/StringBuilder.hpp"
#include "Containers/Algorithm.hpp"
#include "StringReplacement.hpp"

namespace Zero
{

struct TestLetterIndex
{
  uint index;
  TestLetterIndex(uint i)
    :index(i)
  {}

  template<typename arrayType>
  bool operator()(const arrayType& a, Rune b)
  {
    if(index < a.SizeInBytes())
      return b > a[index];
    else
      return true;
  }

  template<typename arrayType>
  bool operator()(Rune a, const arrayType& b)
  {
    if(index < b.SizeInBytes())
      return a < b[index];
    else
      return false;
  }
};

void Replace(StringBuilder& output, Replacements& replacements, StringParam source)
{
  // This algorithm assumes the replacements are sorted by match string
  Sort(replacements.All());

  StringRange sourceText = source.All();
  while(!sourceText.Empty())
  {
    ReplaceRange possibleMatches = replacements.All();
    int letterIndex = 0;

    while(!possibleMatches.Empty())
    {
      //Ran out of letters
      if(letterIndex == sourceText.SizeInBytes())
      {
        for(ReplaceRange range = replacements.All(); !range.Empty(); range.PopFront())
        {
          if(range.Front().MatchString == sourceText)
          {
            output << possibleMatches.Front().ReplaceString;
            sourceText = StringRange();
            return;
          }
        }
        output << sourceText;
        sourceText = StringRange();
        break;
      }

      Rune currentLetter = sourceText.Front();
      ReplaceRange lower = LowerBound(possibleMatches, currentLetter, TestLetterIndex(letterIndex));
      ReplaceRange upper = UpperBound(possibleMatches, currentLetter, TestLetterIndex(letterIndex));
      ReplaceRange refinedMatches = ReplaceRange(lower.Begin(), upper.Begin());

      if(refinedMatches.Empty())
      {
        String& match = possibleMatches.Front().MatchString;
        StringRange lastText = StringRange(sourceText.Begin(), sourceText.Begin() + letterIndex);
        //The front will always be the shortest
        if(match == lastText)
        {
          output << possibleMatches.Front().ReplaceString;
          sourceText = StringRange(sourceText.mOriginalString, sourceText.Data() + match.SizeInBytes(), sourceText.End().Data());
          break;
        }
        else
        {
          output << sourceText.Front();
          sourceText = StringRange(sourceText.Begin() + 1, sourceText.End());
          break;
        }
      }
      else
      {
        if(refinedMatches.Length() == 1)
        {
          //Only one possible match
          String& match = refinedMatches.Front().MatchString;
          if(sourceText.SizeInBytes() >= match.SizeInBytes() && 
            match == StringRange(sourceText.Data(), sourceText.Data(), sourceText.Data() + match.SizeInBytes()))
          {
            output << refinedMatches.Front().ReplaceString ;
            sourceText = StringRange(sourceText.Data() + match.SizeInBytes(), sourceText.Data() + match.SizeInBytes(), sourceText.End().Data());
            break;
          }
          else
          {
            output << sourceText.Front();
            sourceText = StringRange(sourceText.Begin() + 1, sourceText.End());
            break;
          }
        }
        else
        {
          //Next letter
          possibleMatches = refinedMatches;
          ++letterIndex;
        }
      }
    }
  }
}

String Replace(Replacements& replacements, StringParam source)
{
  StringBuilder output;
  Replace(output, replacements, source);
  return output.ToString();
}

String Replace(StringParam matchString, StringParam replaceString, StringParam source)
{
  Replacements replacements;

  Replacement& replacement = replacements.PushBack();
  replacement.MatchString = matchString;
  replacement.ReplaceString = replaceString;

  return Replace(replacements, source);
}

}
