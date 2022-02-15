// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

#define ZeroPlatform WelderTargetOsName "_" WelderArchitectureName

extern const String sWelderOrganization;
extern const String sEditorGuid;
extern const String sEditorName;
extern const String sLauncherGuid;
extern const String sLauncherName;

String GetEditorFullName();
String GetEditorExecutableFileName();
String GetLauncherFullName();
String GetLauncherExecutableFileName();

// Get an executable that may be built as a sibling to our own.
// For example if we were executing:
// - Libraries/EditorExecutable/EditorExecutable.exe
// Then this may return:
// - ../../LauncherExecutable/LauncherExecutable.exe
String GetRelativeExecutable(StringParam organization, StringParam name);

/// Sets all the application version and name information.
void SetupApplication(uint configVersion, StringParam organization, StringParam guid, StringParam name);

/// Get the guid (primarily for crashes).
cstr GetGuidString();

/// Gets the name of the current running application.
StringParam GetApplicationName();

/// Gets the orgnaization or name-space of an application.
StringParam GetOrganization();

/// Gets the full name: GetOrganization() + GetApplicationName();
String GetOrganizationApplicationName();

/// Version of the configuration file we're loading (bumped for breaking changes).
uint GetConfigVersion();

/// The major version number denotes breaking changes to the build.
uint GetMajorVersion();

/// The minor version number denotes new features.
uint GetMinorVersion();

/// The patch version number denotes bug fixes.
uint GetPatchVersion();

/// The revision number is an ever changing id for the build that is typically
/// tied to source control (local numbers may vary). Primarily used to
/// differentiate development builds.
uint GetRevisionNumber();

/// Get the changeset id that is a unique identifier for the commit this build
/// came from.
u64 GetShortChangeSet();

/// If this is a non-mainline branch, then this is the experimental branch's
/// name used to identify the build. Returns null if this is not an experimental
/// branch.
cstr GetExperimentalBranchName();

/// The major version number denotes breaking changes to the build.
cstr GetMajorVersionString();

/// The minor version number denotes new features.
cstr GetMinorVersionString();

/// The patch version number denotes bug fixes.
cstr GetPatchVersionString();

/// The revision number is an ever changing id for the build that is typically
/// tied to source control (local numbers may vary). Primarily used to
/// differentiate development builds.
cstr GetRevisionNumberString();

/// The full build id of Major.Minor.Patch.Revision. The id will be prefaced
/// with the experimental branch name if it exists.
String GetBuildIdString();

/// The shorter changeset as a string
cstr GetShortChangeSetString();

/// Get changeset as a string.
cstr GetChangeSetString();

/// Get the changeset date as a string.
cstr GetChangeSetDateString();

/// Get the configuration as a string.
cstr GetConfigurationString();

/// Get the platform as a string.
cstr GetPlatformString();

/// Get the full string description of the build version.
String GetBuildVersionName();

/// Read a version from a specified text file.
int GetVersionId(StringParam versionIdFilePath);

} // namespace Zero
