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
#include "String.hpp"
#include "Status.hpp"

namespace Zero
{

//------------------------------------------------------------------- Validation
bool IsValidFilename(StringParam filename, Status& status);

String ConvertToValidName(StringParam source);

}//namespace Zero
