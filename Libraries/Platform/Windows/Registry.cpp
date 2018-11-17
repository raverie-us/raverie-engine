///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool GetRegistryValue(void* key, StringParam subKey, StringParam value, String& result)
{
  HKEY  hRoot = (HKEY)key;

  // Read OEM Name from the key
  wchar_t nameBuffer[256] = { 0 };
  DWORD length = sizeof(nameBuffer);

  HKEY  hKey;
  LONG queryResult = RegOpenKeyEx(hRoot, Widen(subKey).c_str(), 0, KEY_QUERY_VALUE, &hKey);
  if(queryResult != ERROR_SUCCESS)
    return false;

  queryResult = RegQueryValueEx(hKey, Widen(value).c_str(), 0, 0, (LPBYTE)nameBuffer, &length);
  if(queryResult != ERROR_SUCCESS)
    return false;

  result = Narrow(nameBuffer);
  return true;
}

bool GetRegistryValue(StringParam key, StringParam subKey, StringParam value, String& result)
{
  static const String HkeyCurrentUser("HKEY_CURRENT_USER");
  static const String HkeyLocalMachine("HKEY_LOCAL_MACHINE");

  void* winKey = nullptr;

  if (key == HkeyCurrentUser)
    winKey = HKEY_CURRENT_USER;
  else if (key == HkeyLocalMachine)
    winKey = HKEY_LOCAL_MACHINE;

  ReturnIf(winKey == nullptr, false, "Unable to find KEY by name (or unhandled case)");
  return GetRegistryValue(winKey, subKey, value, result);
}

bool GetRegistryValueFromCommonInstallPaths(StringParam programGuid, StringParam keyName, String& result)
{
  String path1 = String::Format("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{%s}_is1", programGuid.c_str());
  String path2 = String::Format("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{%s}}_is1", programGuid.c_str());

  if(GetRegistryValue(HKEY_CURRENT_USER, path1, keyName, result))
    return true;
  if(GetRegistryValue(HKEY_CURRENT_USER, path2, keyName, result))
    return true;
  if(GetRegistryValue(HKEY_LOCAL_MACHINE, path1, keyName, result))
    return true;
  if(GetRegistryValue(HKEY_LOCAL_MACHINE, path2, keyName, result))
    return true;
  return false;
}

}//namespace Zero
