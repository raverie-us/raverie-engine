// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

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

String GetEditorFullName()
{
  return sWelderOrganization + sEditorName;
}

String GetEditorExecutableFileName()
{
  return GetEditorFullName() + cExecutableExtensionWithDot;
}

String GetLauncherFullName()
{
  return sWelderOrganization + sLauncherName;
}

String GetLauncherExecutableFileName()
{
  return GetLauncherFullName() + cExecutableExtensionWithDot;
}

String GetRelativeExecutable(StringParam organization, StringParam name)
{
  String fullName = organization + name;
  return FilePath::Combine("..", "..", fullName, fullName + cExecutableExtensionWithDot);
}

uint gConfigVersion = 1;
String gAppGuid;
String gAppOrganization;
String gAppName;

void SetupApplication(uint configVersion, StringParam organization, StringParam guid, StringParam name)
{
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

String GetOrganizationApplicationName()
{
  return GetOrganization() + GetApplicationName();
}

uint GetConfigVersion()
{
  return gConfigVersion;
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
  return WelderConfigName;
}

cstr GetPlatformString()
{
  return ZeroPlatform;
}

String GetBuildVersionName()
{
  /*
   * This needs to match
   * index.js:pack/Standalone.cpp:BuildId::Parse/BuildId::GetFullId/BuildVersion.cpp:GetBuildVersionName.
   * Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.TargetOs.Architecture.Config.Extension
   * Example: WelderEditor.master.1.5.0.1501.fb02756c46a4.1574702096290.Windows.x86.Release.zip
   */
  StringBuilder builder;
  builder.AppendFormat("%s.", GetApplicationName().c_str()); // Application [WelderEditor]
  builder.AppendFormat("%s.", WelderBranchName);             // Branch [master]
  builder.AppendFormat("%d.", GetMajorVersion());            // Major [1]
  builder.AppendFormat("%d.", GetMinorVersion());            // Minor [5]
  builder.AppendFormat("%d.", GetPatchVersion());            // Patch [0]
  builder.AppendFormat("%d.", GetRevisionNumber());          // Revision [1501]
  builder.AppendFormat("%s.", GetShortChangeSetString());    // ShortChangeset [fb02756c46a4]
  builder.AppendFormat("%llu.", WelderMsSinceEpoch);         // MsSinceEpoch [1574702096290]
  builder.AppendFormat("%s.", WelderTargetOsName);           // TargetOs [Windows]
  builder.AppendFormat("%s.", WelderArchitectureName);       // Architecture [x86]
  builder.AppendFormat("%s.", WelderConfigName);             // Config [Release]
  builder.Append("zip");
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
