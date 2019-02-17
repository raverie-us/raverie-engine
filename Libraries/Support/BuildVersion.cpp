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

// Helper to stringify the build id's name conditionally on if there's an
// experimental branch
#ifdef ZeroExperimentalBranchName
#  define ZeroBuildIdString()                                                              \
    ZeroExperimentalBranchName                                                             \
        "." Stringify(WelderMajorVersion) "." Stringify(WelderMinorVersion) "." Stringify( \
            WelderPatchVersion) "." Stringify(WelderRevisionId)
#else
#  define ZeroBuildIdString()                                                      \
    Stringify(WelderMajorVersion) "." Stringify(WelderMinorVersion) "." Stringify( \
        WelderPatchVersion) "." Stringify(WelderRevisionId)
#endif

cstr GetGuidString()
{
  return "F6E3D203-EB81-4B09-983C-31DAD32AE29F";
}

cstr GetLauncherGuidString()
{
  return "295EE6D2-9E03-43A6-8150-388649CC1341";
}

uint GetLauncherMajorVersion()
{
  // Hardcoded to 1 for all legacy versions. The first version on the new server
  // will be 2
  return 4;
}

uint GetMajorVersion()
{
  return WelderMajorVersion;
}

uint GetMinorVersion()
{
  return WelderMinorVersion;
}

uint GetPatchVersion()
{
  return WelderPatchVersion;
}

uint GetRevisionNumber()
{
  return WelderRevisionId;
}

u64 GetShortChangeSet()
{
  return WrapHex(WelderShortChangeSet);
}

cstr GetExperimentalBranchName()
{
#ifdef ZeroExperimentalBranchName
  return ZeroExperimentalBranchName;
#else
  return nullptr;
#endif
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

cstr GetBuildIdString()
{
  return ZeroBuildIdString();
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

cstr GetBuildVersionName()
{
  return ZeroBuildIdString() " " Stringify(
      WelderChangeSet) " " WelderChangeSetDate " " ZeroConfiguration
                       " " ZeroPlatform;
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
