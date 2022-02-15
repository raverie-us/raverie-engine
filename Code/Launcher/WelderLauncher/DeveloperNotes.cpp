// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void DeveloperNotes::Serialize(Serializer& stream)
{
  SerializeNameDefault(mFileName, String());
  SerializeNameDefault(mNotes, String());
  SerializeNameDefault(mDate, String());
}

} // namespace Zero
