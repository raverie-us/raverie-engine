// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"
#include "String.hpp"
#include "Status.hpp"

namespace Zero
{

bool IsValidFilename(StringParam filename, Status& status);

String ConvertToValidName(StringParam source);

} // namespace Zero
