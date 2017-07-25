/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

#ifdef __clang__
  // At the moment, these are missing from the clang libc (no idea what they are!)
  void* __gxx_personality_v0 = NULL;
  void* _Unwind_Resume = NULL;
#endif

namespace Zilch
{
  //***************************************************************************
  String GetDocumentationStringOrEmpty(StringParam string)
  {
    ZilchErrorIfNotStarted(Documentation);

    if (ZilchSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
      return String();

    return string;
  }
  
  //***************************************************************************
  String GetDocumentationCStringOrEmpty(cstr string)
  {
    ZilchErrorIfNotStarted(Documentation);

    if (ZilchSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
      return String();

    return string;
  }
}
