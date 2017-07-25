///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(MenuDisplayed);
  DeclareEvent(MenuHidden);
  DeclareEvent(VersionListLoaded);
  DeclareEvent(TemplateListLoaded);
  DeclareEvent(CheckForUpdates);
}

//------------------------------------------------------------------ Menu Button
/// The Settings button next to the main tab buttons.
class MenuButton : public Composite
{
public:
  /// Constructor.
  MenuButton(Composite* parent);

  Vec2 GetMinSize() override;
  Element* mIcon;
};

//------------------------------------------------------------------------ Menus
DeclareEnum7(LauncherMenu, Discover, Projects, RecentProjects, NewProject,
                           Builds, MenuCount, None);

//---------------------------------------------------------------- Launcher Menu
class MenuData
{
public:
  LauncherMenu::Type mMenu;

  /// The parent menu if it exists.
  LauncherMenu::Type mParentMenu;

  /// Can be null.
  MenuTextButton* mMenuButton;

  /// Sub menu buttons.
  Array<MenuData*> mSubMenus;
  LauncherMenu::Type mSelectedSubMenu;

  /// Custom Ui placed below the main menu buttons.
  Composite* mCustomUi;

  /// The menu that is displayed in the center of the launcher.
  Composite* mClientArea;
};

//------------------------------------------------------------- Menu Text Button
class MenuTextButton : public TextButton
{
public:
  /// Typedefs.
  typedef MenuTextButton ZilchSelf;

  /// Constructor.
  MenuTextButton(Composite* parent, StringParam style, LauncherMenu::Type menu);

  /// Updates the visuals to represent a selected button.
  void Select();

  /// Updates the visuals to represent a de-selected button.
  void DeSelect();

  LauncherMenu::Type mMenu;
};

//------------------------------------------------------------------ Main Button
/// Will be used for running projects / installing builds.
class MainButton : public Composite
{
public:
  /// Typedefs.
  typedef MainButton ZilchSelf;

  /// Constructor.
  MainButton(Composite* parent);

  /// Composite Interface.
  void UpdateTransform() override;

  /// Set the version that the button is looking at
  /// (which sets up the install/uninstall, build number, etc... text)
  void SetVersionAndProject(ZeroBuild* version, Project* project);

  /// The primary text of the button.
  void SetText(StringParam text);
  /// Sets the text underneath the title.
  void SetSubText(StringParam text);

  /// The build id string underneath the main text.
  void SetBuildId(const BuildId& buildId);

  /// Whether or not the button can be clicked (will gray out if not).
  void SetEnabled(bool state);

  /// Event Response.
  void OnLeftClick(MouseEvent* e);
  void OnMouseEnter(MouseEvent* e);
  void OnMouseDown(MouseEvent* e);
  void OnMouseUp(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  bool mMouseDown;

  /// Whether or not the button can currently be clicked.
  bool mEnabled;

  /// The background when disabled.
  Element* mBackgroundDisabled;

  /// The background when enabled.
  Element* mBackground;

  /// The primary title text.
  Text* mTitle;

  /// The build number underneath the title.
  Text* mSubText;
};

//-------------------------------------------------------------- Launcher Window
class LauncherWindow : public Composite
{
public:
  /// Typedefs.
  typedef LauncherWindow ZilchSelf;

  /// Constructor.
  LauncherWindow(MainWindow* parent, Cog* launcherConfigCog);
  ~LauncherWindow();

  /// Check for the server for templates and builds.
  void CheckForUpdates();
  void OnCheckForUpdates(Event* e);
  /// Check to see if there is a new launcher available
  void CheckForLauncherUpdates();
  /// Check for launcher major updates (requires running a new installer)
  void CheckForMajorLauncherUpdates();
  /// Response for check if a new installer is available. May invoke the installer to close and restart the launcher.
  void OnCheckForMajorLauncherUpdates(BackgroundTaskEvent* e);
  /// Check if there is a new patch to the launcher (dll to download)
  void CheckForLauncherPatch();
  /// Response for a launcher patch (dll)
  void OnCheckForLauncherPatch(BackgroundTaskEvent* e);
  /// Prompt response for asking if a user wants to install a new major version (runs installer).
  void OnInstallMajorVersion(ModalConfirmEvent* e);
  /// Response for a launcher installer being downloaded.
  void OnMajorLauncherUpdateDownloaded(BackgroundTaskEvent* e);
  /// After trying to download the version id from the server, check if the id is newer.
  void OnLauncherIdCheck(BackgroundTaskEvent* e);
  /// Callback from prompting the user to restart and update the launcher.
  void OnRestartPrompt(ModalConfirmEvent* e);
  /// Tell the user that all of their builds must be updated. Needed for emergency patches to old builds.
  void ForceUpdateBuilds();
  void ForceUpdateBuildsAndUpdateConfig();
  void OnForcedBuildsModal(ModalButtonEvent* e);
  /// Composite Interface.
  void UpdateTransform() override;

  /// Links the given composite to the given menu type.
  MenuData* RegisterMenu(LauncherMenu::Type menu,
                         Composite* clientComposite = nullptr,
                         LauncherMenu::Type parentMenu = LauncherMenu::None);

  void SetDefaultSubMenu(LauncherMenu::Type parentMenu,
                         LauncherMenu::Type defaultSubMenu);

  void AddMenuButton(LauncherMenu::Type menu, StringParam name, StringParam style);

  /// Displays the given composite in the menu area.
  void DisplayInClientArea(Composite* composite);

  void CloseActiveModals();
  /// Shows the given menu in the menu area.
  void SelectMenu(LauncherMenu::Type menu);
  void SelectMenu(MenuData* menu);
  void AddToRecentProjects(CachedProject* project);
  void RemoveFromRecentProjects(CachedProject* project);
  void SelectActiveProject(CachedProject* project, bool overrideUserTags);

  /// When a menu button is pressed, we want to select it and show its menu.
  void OnMenuButtonPressed(ObjectEvent* e);

  /// Display the settings menu when the settings icon is pressed.
  void OnSettingsPressed(Event*);

  /// When the version list has been notified, we need to give the data
  /// to the version selector, and notify that they have been loaded.
  void OnPackageListing(BackgroundTaskEvent* taskEvent);
  void OnTemplateListing(BackgroundTaskEvent* taskEvent);
  void OnNewBuildAvailable(Event* e);
  void CheckForForcedBuildUpdate();

  /// When the OS window was closed, we want to terminate the engine.
  void OnClosing(Event* e);
  
  /// De-selects all buttons for all menus.
  void DeSelectAllButton();

  /// Returns the configuration object for the launcher.
  LauncherConfig* GetConfig();

  void OnMouseDown(MouseEvent* e);
  /// Check for drag and drop of .zeroproj, zero builds and templates
  void OnOsMouseDrop(OsMouseDropEvent* e);
  /// Callback function for the modal dialog after installing a local build.
  void OnInstallLocalBuild(ModalConfirmEvent* e);
  /// Callback function for the modal dialog after installing a template project.
  void OnInstallTemplateProject(ModalConfirmEvent* e);
  void OnInstallProjectPack(ModalConfirmEvent* e);

  /// Search for a .zeroproj when the browse button is pressed.
  void OnBrowsePressed(Event* e);
  void OnOpenProjectFile(OsFileSelection* e);

  void OnFileBugPressed(Event* e);
  
  // Tell the OS window to take focus (deals with minimization as well
  void OsWindowTakeFocus();

  /// Start listening for inter-process communication
  void StartListening();
  void OnLauncherUpdateTags(LauncherCommunicationEvent* e);
  void OnLauncherNewProject(LauncherCommunicationEvent* e);
  void OnLauncherOpenProject(LauncherCommunicationEvent* e);
  void OnLauncherRunProject(LauncherCommunicationEvent* e);
  void OnLauncherOpenRecentProjects(LauncherCommunicationEvent* e);
  void OnLauncherRunCommand(LauncherCommunicationEvent* e);
  void OnLauncherOpenBuild(LauncherCommunicationEvent* e);
  void OnLauncherOpenTemplate(LauncherCommunicationEvent* e);
  void OnLauncherInstallProject(LauncherCommunicationEvent* e);

  /// Different areas in the launcher.
  Composite* mTopBar;
  Composite* mButtonArea;
  /// Custom Ui for each page should be attached to this.
  Composite* mCustomMenuArea;
  Composite* mCenterPanel;
  Composite* mBottomBar;
  
  /// We want to allow the entire user to drag the launcher window
  /// by dragging anywhere in the entire top bar of the launcher.
  /// This widget is filled in the top bar to allow that.
  Gripper* mWindowGripper;

  IconButton* mSettingsButton;

  /// The background for the menu panel.
  Element* mCenterPanelBackground;

  /// The different menu's in the launcher.
  MenuData* mSelectedMenu;
  MenuData* mMenus[LauncherMenu::MenuCount];

  /// The main button in the lower right corner.
  MainButton* mMainButton;

  /// Search box used on various windows.
  TagChainTextBox* mSearch;

  TextButton* mFileBugTextButton;

  VersionSelector* mVersionSelector;
  ProjectCache* mProjectCache;
  Cog* mConfigCog;

  /// Listen for communications from other launcher or the engine
  TcpSocket* mListener;

  // The last activated modal.
  HandleOf<Modal> mActiveModal;

  HandleOf<Modal> mActiveProjectModal;
  HandleOf<Modal> mSettingsModal;

  /// Various event objects created to send data via tcpsockets. They need to live for
  /// an undetermined amount of time so we're just storing them to clean up at the end.
  Array<EventObject*> mDummyCommunicators;
};

}//namespace Zero
