// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Main configuration component
class MainConfig : public Component
{
public:
  RaverieDeclareType(MainConfig, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  String GetBuildDate();
  String GetBuildVersion();

  /// Directory source was built from.
  String SourceDirectory;
  /// Directory for built in data files
  String DataDirectory;
  /// Should the config be saved? Used in stress tester.
  bool mSave;

  /// A global flag for whether we save the config or not. This is used in unit
  /// test mode where we want to clear the config but not have it overwrite the
  /// user's config.
  static bool sConfigCanSave;
};

/// Configuration for the editor.
class EditorConfig : public Component
{
public:
  RaverieDeclareType(EditorConfig, TypeCopyMode::ReferenceType);

  void Serialize(Serializer& stream) override;

  /// Project to open on load.
  String EditingProject;

  /// Project level to load in project.
  String EditingLevel;

  /// Username that's used by the bug reporter.
  String BugReportUsername;
};

/// Configuration component that Contains user info.
class UserConfig : public Component
{
public:
  RaverieDeclareType(UserConfig, TypeCopyMode::ReferenceType);

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
};

/// Configuration component for content system. Used to find content paths and
/// what default libraries to load.
class ContentConfig : public Component
{
public:
  RaverieDeclareType(ContentConfig, TypeCopyMode::ReferenceType);
  void Serialize(Serializer& stream) override;

  /// Content output directory.
  String ContentOutput;
  /// Content tools directory.
  String ToolsDirectory;
  /// Directories to search for shared content
  /// libraries.
  Array<String> LibraryDirectories;
  /// History stores files instead of deleting them
  bool HistoryEnabled;
};

/// Configuration component that Contains developer settings. Used to indicate a
/// user is a developer.
class DeveloperConfig : public Component
{
public:
  RaverieDeclareType(DeveloperConfig, TypeCopyMode::ReferenceType);

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

DeclareEnum2(TabWidth, TwoSpaces, FourSpaces);

class TextEditorConfig : public Component
{
public:
  RaverieDeclareType(TextEditorConfig, TypeCopyMode::ReferenceType);
  void Serialize(Serializer& stream) override;

  /// Default Font Size
  uint FontSize;

  /// Number of spaces inserted for tabs
  TabWidth::Enum TabWidth;

  /// If we show whitespace as special symbols in the text editor
  bool ShowWhiteSpace;

  /// When the auto-complete is confident in its results (green), this controls
  /// whether or not we will finish completion on any symbol rather than just
  /// Tab Non-confident results (red) always require the user to press Tab (or
  /// Enter if AutoCompleteOnEnter is set)
  bool ConfidentAutoCompleteOnSymbols;

  /// Whether we include local words from the current document / language
  bool LocalWordCompletion;

  /// Whether we include keywords and types from the languages
  bool KeywordAndTypeCompletion;

  /// Whether or not the auto-complete allows enter (similar to Tab) to be used
  /// as an auto-completer If the user manually scrolls through the list of
  /// suggestions, Enter will always complete regardless of this option
  bool AutoCompleteOnEnter;

  /// Is code folding enabled?
  bool CodeFolding;

  /// Show Line numbers
  bool LineNumbers;

  /// Turn on/off highlighting all instances of text matching current text
  /// selection.
  bool TextMatchHighlighting;

  /// Highlight mode is either partial text match, or whole text match.
  bool HighlightPartialTextMatch;

  /// Name of color scheme to use
  String ColorScheme;
};

typedef void (*ModifyConfigFn)(Cog* config, void* userData);
// Load the config for another application. Be careful because all components must be registered or proxies.
Cog* LoadRemoteConfig(StringParam organization, StringParam applicationName);
// Load the config file for under a application name.
Cog* LoadConfig(ModifyConfigFn modifier, void* userData);
// Save the configuration file.
void SaveConfig();
// Remove config file
void RemoveConfig();
// Find the source directory by walking up from the application path looking for
// '.raverie'. If it is not found, it returns the application directory.
String FindSourceDirectory();

} // namespace Raverie
