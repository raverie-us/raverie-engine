///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Get the guid (primarily for crashes).
cstr GetGuidString();

/// Get the launcher's guid (for the mutex name)
cstr GetLauncherGuidString();

/// The major version for the launcher. This is currently hard-coded and not part of the build system.
/// This is used to denote if a new installer should be downloaded and run.
uint GetLauncherMajorVersion();

/// The major version number denotes breaking changes to the build.
uint GetMajorVersion();

/// The minor version number denotes new features.
uint GetMinorVersion();

/// The patch version number denotes bug fixes.
uint GetPatchVersion();

/// The revision number is an ever changing id for the build that is typically tied
/// to source control (local numbers may vary). Primarily used to differentiate development builds.
uint GetRevisionNumber();

/// Get the changeset id that is a unique identifier for the commit this build came from.
u64 GetShortChangeSet();

/// If this is a non-mainline branch, then this is the experimental branch's name
/// used to identify the build. Returns null if this is not an experimental branch.
cstr GetExperimentalBranchName();

/// The major version number denotes breaking changes to the build.
cstr GetMajorVersionString();

/// The minor version number denotes new features.
cstr GetMinorVersionString();

/// The patch version number denotes bug fixes.
cstr GetPatchVersionString();

/// The revision number is an ever changing id for the build that is typically tied
/// to source control (local numbers may vary). Primarily used to differentiate development builds.
cstr GetRevisionNumberString();

/// The full build id of Major.Minor.Patch.Revision. The id will be prefaced with the experimental branch name if it exists.
cstr GetBuildIdString();

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
cstr GetBuildVersionName();

}// namespace Zero
