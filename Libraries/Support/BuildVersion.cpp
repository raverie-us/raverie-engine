// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#if ZeroRelease
#  define ZeroConfiguration "Release"
#elif ZeroDebug
#  define ZeroConfiguration "Debug"
#else
#  define ZeroConfiguration "Unknown"
#endif

namespace Zero
{

#define Stringify(s) InnerStringify(s)
#define InnerStringify(s) #s

#define WrapHex(a) InnerWrapHex(a)
#define InnerWrapHex(s) 0x##s##ULL

const String sWelderOrganization = "Welder";
const String sEditorGuid = "51392222-AEDE-4530-8749-9DFAB5725FD7";
const String sEditorName = "Editor";
const String sLauncherGuid = "7489829B-8A03-4B26-B3AC-FDDC6668BAF7";
const String sLauncherName = "Launcher";

uint gAppMajorVersion = 1;
uint gAppMinorVersion = 0;
uint gAppPatchVersion = 0;
uint gConfigVersion = 1;
String gAppGuid;
String gAppOrganization;
String gAppName;

void SetupApplication(uint major,
                      uint minor,
                      uint patch,
                      uint configVersion,
                      StringParam organization,
                      StringParam guid,
                      StringParam name)
{
  gAppMajorVersion = major;
  gAppMinorVersion = minor;
  gAppPatchVersion = patch;
  gConfigVersion = configVersion;
  gAppGuid = guid;
  gAppOrganization = organization;
  gAppName = name;
}

cstr GetGuidString()
{
  return gAppGuid.c_str();
}

StringParam GetApplicationName()
{
  return gAppName;
}

StringParam GetOrganization()
{
  return gAppOrganization;
}

uint GetConfigVersion()
{
  return gConfigVersion;
}

uint GetMajorVersion()
{
  return gAppMajorVersion;
}

uint GetMinorVersion()
{
  return gAppMinorVersion;
}

uint GetPatchVersion()
{
  return gAppPatchVersion;
}

uint GetRevisionNumber()
{
  return WelderRevisionId;
}

u64 GetShortChangeSet()
{
  return WrapHex(WelderShortChangeSet);
}

cstr GetMajorVersionString()
{
  return Stringify(WelderMajorVersion);
}

cstr GetMinorVersionString()
{
  return Stringify(WelderMinorVersion);
}

cstr GetPatchVersionString()
{
  return Stringify(WelderPatchVersion);
}

cstr GetRevisionNumberString()
{
  return Stringify(WelderRevisionId);
}

String GetBuildIdString()
{
  StringBuilder builder;
  builder.Append(ToString(GetMajorVersion()));
  builder.Append('.');
  builder.Append(ToString(GetMinorVersion()));
  builder.Append('.');
  builder.Append(ToString(GetPatchVersion()));
  builder.Append('.');
  builder.Append(GetRevisionNumberString());
  String result = builder.ToString();
  return result;
}

cstr GetShortChangeSetString()
{
  return Stringify(WelderShortChangeSet);
}

cstr GetChangeSetString()
{
  return Stringify(WelderChangeSet);
}

cstr GetChangeSetDateString()
{
  return WelderChangeSetDate;
}

cstr GetConfigurationString()
{
  return ZeroConfiguration;
}

cstr GetPlatformString()
{
  return ZeroPlatform;
}

String GetBuildVersionName()
{
  StringBuilder builder;
  builder.Append(GetBuildIdString());
  builder.Append(' ');
  builder.Append(GetChangeSetString());
  builder.Append(' ');
  builder.Append(GetChangeSetDateString());
  builder.Append(' ');
  builder.Append(ZeroConfiguration);
  builder.Append(' ');
  builder.Append(ZeroPlatform);
  String result = builder.ToString();
  return result;
}

int GetVersionId(StringParam versionIdFilePath)
{
  int localVersionId = -99;
  // make sure the file exists, if it doesn't assume the version is 0
  //(aka, the lowest and most likely to be replaced)
  if (FileExists(versionIdFilePath))
  {
    size_t fileSize;
    byte* data = ReadFileIntoMemory(versionIdFilePath.c_str(), fileSize, 1);
    data[fileSize] = 0;
    if (data == nullptr)
      return localVersionId;

    ToValue(String((char*)data), localVersionId);
    delete data;
  }
  return localVersionId;
}

} // namespace Zero
