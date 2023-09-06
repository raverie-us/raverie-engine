// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

extern const String sRaverieOrganization;
extern const String sEditorGuid;
extern const String sEditorName;

String GetEditorFullName();
String GetEditorExecutableFileName();

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

/// Get the full string description of the build version.
String GetBuildVersionName();

/// Read a version from a specified text file.
int GetVersionId(StringParam versionIdFilePath);

} // namespace Zero
