// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

#define Stringify(s) InnerStringify(s)
#define InnerStringify(s) #s

#define WrapHex(a) InnerWrapHex(a)
#define InnerWrapHex(s) 0x##s##ULL

const String sRaverieOrganization = "Raverie";
const String sEditorName = "Editor";

StringParam GetApplicationName()
{
  return sEditorName;
}

StringParam GetOrganization()
{
  return sRaverieOrganization;
}

String GetOrganizationApplicationName()
{
  return GetOrganization() + GetApplicationName();
}

uint GetConfigVersion()
{
  return 1;
}

uint GetMajorVersion()
{
  return RaverieMajorVersion;
}

uint GetMinorVersion()
{
  return RaverieMinorVersion;
}

uint GetPatchVersion()
{
  return RaveriePatchVersion;
}

uint GetRevisionNumber()
{
  return RaverieRevisionId;
}

u64 GetShortChangeSet()
{
  return WrapHex(RaverieShortChangeSet);
}

cstr GetMajorVersionString()
{
  return Stringify(RaverieMajorVersion);
}

cstr GetMinorVersionString()
{
  return Stringify(RaverieMinorVersion);
}

cstr GetPatchVersionString()
{
  return Stringify(RaveriePatchVersion);
}

cstr GetRevisionNumberString()
{
  return Stringify(RaverieRevisionId);
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
  return Stringify(RaverieShortChangeSet);
}

cstr GetChangeSetString()
{
  return Stringify(RaverieChangeSet);
}

cstr GetChangeSetDateString()
{
  return RaverieChangeSetDate;
}

cstr GetConfigurationString()
{
  return RaverieConfigName;
}

String GetBuildVersionName()
{
  /*
   * This needs to match
   * index.js:pack/Standalone.cpp:BuildId::Parse/BuildId::GetFullId/BuildVersion.cpp:GetBuildVersionName.
   * Application.Branch.Major.Minor.Patch.Revision.ShortChangeset.MsSinceEpoch.Config.Extension
   * Example: RaverieEditor.master.1.5.0.1501.fb02756c46a4.1574702096290.Windows.x86.Release.zip
   */
  StringBuilder builder;
  builder.AppendFormat("%s.", GetApplicationName().c_str()); // Application [RaverieEditor]
  builder.AppendFormat("%s.", RaverieBranchName);             // Branch [master]
  builder.AppendFormat("%d.", GetMajorVersion());            // Major [1]
  builder.AppendFormat("%d.", GetMinorVersion());            // Minor [5]
  builder.AppendFormat("%d.", GetPatchVersion());            // Patch [0]
  builder.AppendFormat("%d.", GetRevisionNumber());          // Revision [1501]
  builder.AppendFormat("%s.", GetShortChangeSetString());    // ShortChangeset [fb02756c46a4]
  builder.AppendFormat("%llu.", RaverieMsSinceEpoch);         // MsSinceEpoch [1574702096290]
  builder.AppendFormat("%s.", RaverieConfigName);             // Config [Release]
  builder.Append("zip");
  String result = builder.ToString();
  return result;
}

} // namespace Zero
