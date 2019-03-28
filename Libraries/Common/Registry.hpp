// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

// This is very much a stubbed api
bool GetRegistryValue(StringParam key, StringParam subKey, StringParam value, String& result);
bool GetRegistryValueFromCommonInstallPaths(StringParam programGuid, StringParam keyName, String& result);

} // namespace Zero
