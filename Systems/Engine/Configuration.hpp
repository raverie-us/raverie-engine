///////////////////////////////////////////////////////////////////////////////
///
/// \file Configuration.hpp
/// Declaration of the engine configuration classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(RecentProjectsUpdated);
}//namespace Events

//------------------------------------------------------------------------------
/// Main configuration component
class MainConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  String GetBuildDate();
  String GetBuildVersion();

  /// Name of the application for separating config files.
  String ApplicationName;
  /// Directory source was built from.
  String SourceDirectory;
  /// Directory for built in data files
  String DataDirectory;
  /// The location the app is running from. Stored here so the launcher can override it to the dll location.
  String ApplicationDirectory;
  /// Did the config file or using default.
  bool mConfigDidNotExist;
  /// Should the config be saved? Used in stress tester.
  bool mSave;
  /// Command line Parameters
  StringMap Parameters;
  /// If we were locally built from the users machine
  bool mLocallyBuilt;
};

//-------------------------------------------------------------------------
/// Configuration for the editor.
class EditorConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  /// Project to open on load.
  String EditingProject;

  /// Project level to load in project.
  String EditingLevel;
};

//-----------------------------------------------------------------------------
/// Configuration component that Contains user info.
class UserConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UserConfig();

  void Serialize(Serializer& stream) override;

  /// Name of the User.
  String UserName;

  /// Email of the User.
  String UserEmail;

  /// Authentication
  String Authentication;

  /// Logged In
  bool LoggedIn;

  /// Last version the user has been informed that is available
  /// for download.  Used to show version dialog
  uint LastVersionKnown;

  /// Last version the user has used locally. Used to 
  /// show release notes.
  uint LastVersionUsed;

  /// NOTE* the TimeType is casted to a u64 to be saved. This can possibly be
  /// an issue depending how time_t is defined on some compilers.
  u64 LastAcceptedEula;
};

//------------------------------------------------------------------------------
/// Configuration component for content system. Used to find content paths and what 
/// default libraries to load.
class ContentConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  void Serialize(Serializer& stream) override;

  /// Content output directory.
  String ContentOutput;
  /// Content tools directory.
  String ToolsDirectory;
  /// Content system console verbosity used for resolving content issues.
  Verbosity::Enum ContentVerbosity;
  /// Directories to search for shared content
  /// libraries.
  Array<String> LibraryDirectories;
  /// History stores files instead of deleting them
  bool HistoryEnabled;
};

//------------------------------------------------------------------------------
/// Configuration component that Contains developer settings. Used to indicate a 
/// user is a developer.
class DeveloperConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DeveloperConfig();

  /// Component Interface.
  void Serialize(Serializer& stream) override;

  /// Double escape to close the engine.
  bool mDoubleEscapeQuit;

  /// Whether or not script objects are proxied in the preview windows.
  bool mProxyObjectsInPreviews;

  /// Allows editing and saving of read only resources.
  bool mCanModifyReadOnlyResources;

  /// This is a random collection of flags so we can check one-off
  /// things without having to create new variables.
  HashSet<String> mGenericFlags;
};

//-------------------------------------------------------------------ZilchPluginConfig
// We attach this component to Zero's editor configuration file
class ZilchPluginConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ZilchPluginConfig();
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer);

  /// If on this machine we attempted to install IDE tools for plugins
  bool mAttemptedIdeToolsInstall;
};

//------------------------------------------------------------------------------
DeclareEnum2(TabWidth, TwoSpaces, FourSpaces);

class TextEditorConfig : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  void Serialize(Serializer& stream) override;

  /// Default Font Size
  uint FontSize;

  /// Number of spaces inserted for tabs
  TabWidth::Enum TabWidth;

  /// If we show whitespace as special symbols in the text editor
  bool ShowWhiteSpace;

  /// When the auto-complete is confident in its results (green), this controls whether or not
  /// we will finish completion on any symbol rather than just Tab
  /// Non-confident results (red) always require the user to press Tab (or Enter if AutoCompleteOnEnter is set)
  bool ConfidentAutoCompleteOnSymbols;

  /// Whether we include local words from the current document / language
  bool LocalWordCompletion;

  /// Whether we include keywords and types from the languages
  bool KeywordAndTypeCompletion;

  /// Whether or not the auto-complete allows enter (similar to Tab) to be used as an auto-completer
  /// If the user manually scrolls through the list of suggestions, Enter will always complete regardless of this option
  bool AutoCompleteOnEnter;

  /// Is code folding enabled?
  bool CodeFolding;

  /// Show Line numbers
  bool LineNumbers;

  /// Name of color scheme to use
  String ColorScheme;
};

//----------------------------------------------------------------------------
class RecentProjects : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Adds the given project.
  void AddRecentProject(StringParam projectFile, bool sendsEvent = false);

  /// Removes the given project.
  void RemoveRecentProject(StringParam projectFile, bool sendsEvent = false);

  /// Returns all objects sorted by date (most recent first).
  void GetProjectsByDate(Array<String>& projects);

  /// Copy one set of recent projects to the other (for the launcher)
  void CopyProjects(RecentProjects* source);

  /// Returns how many recent projects there are.
  /// The launcher uses this to special case what screen is displayed on launch.
  size_t GetRecentProjectsCount() const;

  /// Updates the max number of recent projects we store (and prunes any old items from the list)
  void UpdateMaxNumberOfProjects(uint maxRecentProjects, bool sendsEvent);
  uint mMaxRecentProjects;

  static uint mAbsoluteMaxRecentProjects;

private:
  /// Returns the project in this list that was opened the longest time ago.
  String GetOldestProject();

  /// Remove all projects that no longer exist.
  void RemoveMissingProjects();

  HashSet<String> mRecentProjects;
};

//----------------------------------------------------------------------------

/// Build Info is generated by the build system.
class BuildInfo : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  void Serialize(Serializer& stream) override;

  String ProjectName;
  /// Source directory from build.
  String Source;
  /// Output Directory from build.
  String Output;
};

//Load the config file for under a application name.
Cog* LoadConfig(StringParam applicationName, bool useDefault, ZeroStartupSettings& settings);
//Save the configuration file.
void SaveConfig(Cog* config);
//Remove config file
void RemoveConfig(Cog* config);

}//namespace Zero
