///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "String/String.hpp"

namespace Zero
{

// This is very much a stubbed api
bool GetRegistryValue(StringParam key, StringParam subKey, StringParam value, String& result);
bool GetRegistryValueFromCommonInstallPaths(StringParam programGuid, StringParam keyName, String& result);

}//namespace Zero
