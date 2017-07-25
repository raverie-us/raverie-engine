///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

/// BuildVersion.inl file is updated by the build system based on the latest changeset and the build id files
#include "BuildVersion.inl"


#if ZeroRelease
  #define ZeroConfiguration "Release"
#elif ZeroDebug
  #define ZeroConfiguration "Debug"
#else
  #define ZeroConfiguration "Unknown"
#endif
  
#if _WIN64
  #define ZeroPlatform "Win64"
#else
  #define ZeroPlatform "Win32"
#endif

namespace Zero
{

#define Stringify(s) InnerStringify(s)
#define InnerStringify(s) #s

#define WrapHex(a) InnerWrapHex(a)
#define InnerWrapHex(s) 0x##s##ULL

// Helper to stringify the build id's name conditionally on if there's an experimental branch
#ifdef ZeroExperimentalBranchName
  #define ZeroBuildIdString() \
    ZeroExperimentalBranchName "." Stringify(ZeroMajorVersion) "." Stringify(ZeroMinorVersion) "." Stringify(ZeroPatchVersion) "." Stringify(ZeroRevisionId)
#else
  #define ZeroBuildIdString() \
    Stringify(ZeroMajorVersion) "." Stringify(ZeroMinorVersion) "." Stringify(ZeroPatchVersion) "." Stringify(ZeroRevisionId)
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
  // Hardcoded to 1 for all legacy versions. The first version on the new server will be 2
  return 3;
}

uint GetMajorVersion()
{
  return ZeroMajorVersion;
}

uint GetMinorVersion()
{
  return ZeroMinorVersion;
}

uint GetPatchVersion()
{
  return ZeroPatchVersion;
}

uint GetRevisionNumber()
{
  return ZeroRevisionId;
}

u64 GetShortChangeSet()
{
  return WrapHex(ZeroShortChangeSet);
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
  return Stringify(ZeroMajorVersion);
}

cstr GetMinorVersionString()
{
  return Stringify(ZeroMinorVersion);
}

cstr GetPatchVersionString()
{
  return Stringify(ZeroPatchVersion);
}

cstr GetRevisionNumberString()
{
  return Stringify(ZeroRevisionId);
}

cstr GetBuildIdString()
{
  return ZeroBuildIdString();
}

cstr GetShortChangeSetString()
{
  return Stringify(ZeroShortChangeSet);
}

cstr GetChangeSetString()
{
  return Stringify(ZeroChangeSet);
}

cstr GetChangeSetDateString()
{
  return ZeroChangeSetDate;
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
  return ZeroBuildIdString() " " Stringify(ZeroChangeSet) " " ZeroChangeSetDate " " ZeroConfiguration " " ZeroPlatform;
}

}// namespace Zero
