///////////////////////////////////////////////////////////////////////////////
///
/// \file NameValidation.hpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "String/String.hpp"
#include "Utility/Status.hpp"

namespace Zero
{

const uint cMaxClassNameLength = 32;

//------------------------------------------------------------------- Validation
bool IsValidFilename(StringParam filename, Status& status);
inline bool IsValidName(StringParam name, Status& status)
{
  if(!IsValidFilename(name,status))
    return false;

  if(name.SizeInBytes() > cMaxClassNameLength)
  {
    String err = String::Format("Class name is too long. "
      "Max class name length is %d characters",cMaxClassNameLength);
    status.SetFailed(err);
    return false;
  }
  return true;
}

String ConvertToValidName(StringParam source);

}//namespace Zero
