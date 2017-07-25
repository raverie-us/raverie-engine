///////////////////////////////////////////////////////////////////////////////
///
/// \file StringReplacement.hpp
/// 
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//Replacement Entry
struct Replacement
{
  Replacement()
  {}

  Replacement(StringParam matchString, StringParam replaceString)
    :MatchString(matchString), ReplaceString(replaceString)
  {}

  String MatchString;
  String ReplaceString;

  char operator[](uint index)const{ return *(MatchString.Data() + index); }
  uint SizeInBytes()const {return MatchString.SizeInBytes(); };

  bool operator<(const Replacement& right)const{return MatchString < right.MatchString;}
};

typedef Array<Replacement> Replacements;
typedef Replacements::range ReplaceRange;

// Quickly replace any matching string with replace string in source string outputting
// the replace text to the string builder.
void Replace(StringBuilder& output, Replacements& replacements, StringParam source);
// Helper for replace
String Replace(Replacements& replacements, StringParam source);
String Replace(StringParam replaceThis, StringParam withThis, StringParam source);

}
