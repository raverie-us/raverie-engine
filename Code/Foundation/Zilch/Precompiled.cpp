// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zilch
{
String GetDocumentationStringOrEmpty(StringParam string)
{
  ZilchErrorIfNotStarted(Documentation);

  if (ZilchSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
    return String();

  return string;
}

String GetDocumentationCStringOrEmpty(cstr string)
{
  ZilchErrorIfNotStarted(Documentation);

  if (ZilchSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
    return String();

  return string;
}
} // namespace Zilch
