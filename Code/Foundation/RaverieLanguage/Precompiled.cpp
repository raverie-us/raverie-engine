// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
String GetDocumentationStringOrEmpty(StringParam string)
{
  RaverieErrorIfNotStarted(Documentation);

  if (RaverieSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
    return String();

  return string;
}

String GetDocumentationCStringOrEmpty(cstr string)
{
  RaverieErrorIfNotStarted(Documentation);

  if (RaverieSetup::Instance->Flags & SetupFlags::NoDocumentationStrings)
    return String();

  return string;
}
} // namespace Raverie
