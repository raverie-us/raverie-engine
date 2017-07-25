///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Platform/Registry.hpp"

namespace Zero
{

bool GetRegistryValue(void* key, StringParam subKey, StringParam value, String& result)
{
  return false;
}

bool GetRegistryValueFromCommonInstallPaths(StringParam programGuid, StringParam keyName, String& result)
{
  return false;
}

}//namespace Zero
