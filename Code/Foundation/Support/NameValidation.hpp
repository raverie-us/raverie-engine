// MIT Licensed (see LICENSE.md).
#pragma once
#include "String/String.hpp"
#include "Utility/Status.hpp"

namespace Raverie
{

bool IsValidFilename(StringParam filename, Status& status);

String ConvertToValidName(StringParam source);

} // namespace Raverie
