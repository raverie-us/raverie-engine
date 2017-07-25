///////////////////////////////////////////////////////////////////////////////
///
/// \file NameValidation.cpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include <ctype.h>
#include "NameValidation.hpp"
#include "String/StringBuilder.hpp"
#include "String/CharacterTraits.hpp"
#include "String/StringUtility.hpp"

namespace Zero
{

const char cExtensionDelimiter = '.';

//------------------------------------------------------------------- Validation 

bool IsValidFileNameRune(Rune r)
{
  return IsAlphaNumeric(r) ||  r.value == '.' || r.value  == '_' || r.value == '(' || r.value == ')';
}

const uint cMaxNameLength = 100;
const uint cMinNameLength = 2;

bool IsValidFilename(StringParam name, Status& status)
{
  if(name.Empty())
  {
    status.SetFailed("Empty name is not valid.");
    return false;
  }

  size_t runeCount = name.ComputeRuneCount();

  if(runeCount > cMaxNameLength)
  {
    String err = String::Format("Name is too long. "
      "Max name length is %d characters",cMaxNameLength);
    status.SetFailed(err);
    return false;
  }

  if(runeCount < cMinNameLength)
  {
    String err = String::Format("Name is too short. "
      "Min name length is %d characters",cMinNameLength);
      status.SetFailed(err);
    return false;
  }

  //Must start with alpha character 
  if(!IsAlpha(name.Front()))
  {
    status.SetFailed("Name must start with an alpha character.");
    return false;
  }

  //Must contain only valid characters
  StringRange r = name.All();
  r.PopFront();
  for(;!r.Empty();r.PopFront())
  {
    if(!IsValidFileNameRune(r.Front()))
    {
      String err = String::Format("Invalid character '%c' contained in name.",r.Front().value);
      status.SetFailed(err);
      return false;
    }
  }

  return true;
}


String ConvertToValidName(StringParam source)
{
  StringBuilder builder;
  StringIterator endIt = source.End();
  for(StringIterator curRune = source.Begin(); curRune < endIt; ++curRune)
  {
    Rune r = curRune.ReadCurrentRune();
    //Replace all invalid character with '_'
    if(IsValidFileNameRune(r))
      builder.Append(r);
    else
      builder.Append('_');
  }
  return builder.ToString();
}


}//namespace Zero
