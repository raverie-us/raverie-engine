/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

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
