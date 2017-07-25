///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------- Tweakables
namespace LauncherUi
{
  const cstr cLocation = "LauncherUi/";
  Tweakable(Vec4, CenterBackground, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, MenuText, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, MenuTextHighlight, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, MenuTextClicked, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, MenuTextSelected, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, MenuTextSelectedHighlight, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SettingsColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SettingsHoverColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SettingsClickedColor, Vec4(1,1,1,1), cLocation);
}

namespace LaunchButtonUi
{
  const cstr cLocation = "LauncherUi/LaunchButton";
  Tweakable(Vec4, DisabledBackground, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, TitleText,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildText,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildTextDisabled,  Vec4(1,1,1,1), cLocation);
}

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(MenuDisplayed);
  DefineEvent(MenuHidden);
  DefineEvent(VersionListLoaded);
  DefineEvent(TemplateListLoaded);
  DefineEvent(CheckForUpdates);
}

//------------------------------------------------------------------ Menu Button
//******************************************************************************
MenuButton::MenuButton(Composite* parent) : Composite(parent)
{
  mIcon = CreateAttached<Element>("OptionsIcon");
  mIcon->SetTranslation(Pixels(0,8,0));
}

//******************************************************************************
Vec2 MenuButton::GetMinSize()
{
  return mIcon->GetSize();
}

//------------------------------------------------------------- Menu Text Button
//******************************************************************************
MenuTextButton::MenuTextButton(Composite* parent, StringParam style,
                               LauncherMenu::Type menu) :
  TextButton(parent, style),
  mMenu(menu)
{
  SetStyle(TextButtonStyle::Modern);
  DeSelect();
}

//******************************************************************************
void MenuTextButton::Select()
{
  mTextColor = ToByteColor(LauncherUi::MenuTextSelected);
  mTextHoverColor = ToByteColor(LauncherUi::MenuTextSelectedHighlight);
  mTextClickedColor = ToByteColor(LauncherUi::MenuTextSelectedHighlight);
  SetIgnoreInput(true);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MenuTextButton::DeSelect()
{
  mTextColor = ToByteColor(LauncherUi::MenuText);
  mTextHoverColor = ToByteColor(LauncherUi::MenuTextHighlight);
  mTextClickedColor = ToByteColor(LauncherUi::MenuTextClicked);
  SetIgnoreInput(false);
  MarkAsNeedsUpdate();
}

//------------------------------------------------------------------ Main Button
//******************************************************************************
MainButton::MainButton(Composite* parent) : 
  Composite(parent)
{
  SetSizing(SizePolicy::Fixed, Pixels(156, 49));

  mBackgroundDisabled = CreateAttached<Element>(cWhiteSquare);
  mBackgroundDisabled->SetColor(LaunchButtonUi::DisabledBackground);

  mBackground = CreateAttached<Element>("LauncherButtonBackground");
  mBackground->SetColor(Vec4(0.9f, 0.9f, 0.9f,1));
  
  mTitle = new Text(this, "LaunchButtonTitle");
  mTitle->SetText("LAUNCH");
  mTitle->SizeToContents();
  mTitle->SetColor(LaunchButtonUi::TitleText);

  mSubText = new Text(this, mLauncherRegularFont, 11);
  mSubText->SetText("BUILD 8850");
  mSubText->SizeToContents();
  mSubText->SetColor(LaunchButtonUi::BuildText);

  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseDown, OnMouseDown);
  ConnectThisTo(this, Events::MouseUp, OnMouseUp);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  mMouseDown = false;
  mEnabled = true;
}

//******************************************************************************
void MainButton::UpdateTransform()
{
  mBackgroundDisabled->SetSize(mSize);
  mBackground->SetSize(mSize);

  float x = mSize.x * 0.5f - mTitle->mSize.x * 0.5f;
  mTitle->SetTranslation(Vec3(x,0,0));
  mSubText->SetTranslation(Vec3(x + Pixels(2), Pixels(31), 0));

  if(mMouseDown)
    mBackground->SetColor(Vec4(0.8f, 0.8f, 0.8f, 1));
  else if(IsMouseOver())
    mBackground->SetColor(Vec4(1));
  else
    mBackground->SetColor(Vec4(0.9f, 0.9f, 0.9f, 1));

  Composite::UpdateTransform();
}

//******************************************************************************
void MainButton::SetVersionAndProject(ZeroBuild* version, Project* project)
{
  // If there is no build then disable the button
  if(version == nullptr)
  {
    SetEnabled(false);
    return;
  }

  // Set the build number on the button
  SetBuildId(version->GetBuildId());

  if(project == nullptr)
  {
    SetText("INSTALL");
    // Only enable the button if the build is not currently installed
    if(version->mInstallState == InstallState::NotInstalled)
      SetEnabled(true);
    else
      SetEnabled(false);
  }
  else
  {
    // We're always going to try to launch the current project
    SetText("LAUNCH");
    // Choose whether or not the button is enabled based upon if the build is installed
    if(version->mInstallState == InstallState::Installed)
      SetEnabled(true);
    else
      SetEnabled(false);
  }
}

//******************************************************************************
void MainButton::SetText(StringParam text)
{
  mTitle->SetText(text);
  mTitle->SizeToContents();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::SetSubText(StringParam text)
{
  mSubText->SetText(text);
  mSubText->SizeToContents();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::SetBuildId(const BuildId& buildId)
{
  mSubText->SetText(buildId.ToDisplayString());
  mSubText->SizeToContents();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::SetEnabled(bool state)
{
  mEnabled = state;
  mBackground->SetVisible(state);

  if(state)
    mSubText->SetColor(LaunchButtonUi::BuildText);
  else
    mSubText->SetColor(LaunchButtonUi::BuildTextDisabled);
}

//******************************************************************************
void MainButton::OnLeftClick(MouseEvent* e)
{
  if(mEnabled)
    GetDispatcher()->Dispatch(Events::ButtonPressed, e);
}

//******************************************************************************
void MainButton::OnMouseEnter(MouseEvent* e)
{
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::OnMouseDown(MouseEvent* e)
{
  mMouseDown = true;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::OnMouseUp(MouseEvent* e)
{
  mMouseDown = false;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void MainButton::OnMouseExit(MouseEvent* e)
{
  mMouseDown = false;
  MarkAsNeedsUpdate();
}

//-------------------------------------------------------------- Launcher Window
//******************************************************************************
LauncherWindow::LauncherWindow(MainWindow* parent, Cog* launcherConfigCog)
  : Composite(parent)
{
  mSelectedMenu = nullptr;
  for(uint i = 0; i < LauncherMenu::MenuCount; ++i)
    mMenus[i] = nullptr;

  mConfigCog = launcherConfigCog;

  LauncherConfig* launcherConfig = launcherConfigCog->has(LauncherConfig);
  mVersionSelector = new VersionSelector(launcherConfig);
  mProjectCache = new ProjectCache(mConfigCog);

  // Finds which version are currently installed
  mVersionSelector->FindInstalledVersions();
  mVersionSelector->FindDownloadedTemplates();
  
  // Download what builds and templates are available
  CheckForUpdates();
  ConnectThisTo(this, Events::CheckForUpdates, OnCheckForUpdates);

  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness::cZero));

  mTopBar = new Composite(this);
  mTopBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(115));
  mTopBar->SetLayout(CreateRowLayout());
  {
    mWindowGripper = new Gripper(mTopBar, GetRootWidget(), DockMode::DockFill);
    mWindowGripper->SetNotInLayout(true);

    Composite* leftSide = new Composite(mTopBar);
    leftSide->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(650));
    leftSide->SetLayout(CreateStackLayout());
    {
      mButtonArea = new Composite(leftSide);
      mButtonArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(24, 0), Thickness(Pixels(30, 30, 0, 0))));
      mButtonArea->SetName("Main Tabs");
      mButtonArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
      mButtonArea->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(69));
      {
        mSettingsButton = new IconButton(mButtonArea);
        mSettingsButton->SetIcon("OptionsIcon");
        mSettingsButton->mBackground->SetVisible(false);
        mSettingsButton->mBorder->SetVisible(false);
        mSettingsButton->mIconColor = ToByteColor(LauncherUi::SettingsColor);
        mSettingsButton->mIconHoverColor = ToByteColor(LauncherUi::SettingsHoverColor);
        mSettingsButton->mIconClickedColor = ToByteColor(LauncherUi::SettingsClickedColor);
        ConnectThisTo(mSettingsButton, Events::LeftClick, OnSettingsPressed);

        // Main menu buttons will be registered here
      }

      new Spacer(leftSide, SizePolicy::Fixed, Pixels(1, 10));

      // Custom Ui for each page should be attached to this
      mCustomMenuArea = new Composite(leftSide);
    }

    new Spacer(mTopBar, SizePolicy::Flex, Vec2(1));

    Composite* rightSide = new Composite(mTopBar);
    rightSide->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(0, 70, 35, 0)));
    {
      mSearch = new TagChainTextBox(rightSide);
      mSearch->SetSizing(SizePolicy::Fixed, Pixels(318, 16));
      mSearch->mAddTagsOnEnter = true;
      mSearch->mSearchBar->SetStyle(TextBoxStyle::Modern);
      mSearch->SetSearchIconSide(SearchIconSide::Left);
    }
  }

  mCenterPanelBackground = CreateAttached<Element>(cWhiteSquare);
  mCenterPanelBackground->SetColor(LauncherUi::CenterBackground);

  mCenterPanel = new Composite(this);
  mCenterPanel->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  mCenterPanel->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1.0f);
  
  mBottomBar = new Composite(this);
  mBottomBar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(78));
  mBottomBar->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness(Pixels(35, 14, 35, 0))));
  mBottomBar->SetName("BottomBar");
  {
    Text* zeroText = new Text(mBottomBar, "MainTabText");
    zeroText->SetText("ZERO ENGINE");
    zeroText->SetName("Zero Text");

    Composite* spacer = new Composite(mBottomBar);
    spacer->SetSizing(SizePolicy::Flex, Vec2(1));
    spacer->SetName("Spacer");

    mMainButton = new MainButton(mBottomBar);
    mMainButton->SetName("Main Button");
  }

  // Create all menus and register them
  Composite* discoverMenu = new DiscoverMenu(mCenterPanel, this);
  Composite* recentProjects = new RecentProjectsMenu(mCenterPanel, this);
  Composite* newProject = new NewProjectMenu(mCenterPanel, this);
  Composite* buildsMenu = new BuildsMenu(mCenterPanel, this);

  // Discover page
  RegisterMenu(LauncherMenu::Discover, discoverMenu);
  RegisterMenu(LauncherMenu::Projects, nullptr);
  RegisterMenu(LauncherMenu::NewProject, newProject, LauncherMenu::Projects);
  RegisterMenu(LauncherMenu::RecentProjects, recentProjects, LauncherMenu::Projects);
  RegisterMenu(LauncherMenu::Builds, buildsMenu);

  AddMenuButton(LauncherMenu::Discover, "DISCOVER", "MainTabText");
  AddMenuButton(LauncherMenu::Projects, "PROJECTS", "MainTabText");
  AddMenuButton(LauncherMenu::NewProject, "NEW", "SubTabText");
  AddMenuButton(LauncherMenu::RecentProjects, "RECENT", "SubTabText");
  AddMenuButton(LauncherMenu::Builds, "BUILDS", "MainTabText");

  SetDefaultSubMenu(LauncherMenu::Projects, LauncherMenu::RecentProjects);

  // Add the browse button to the projects menu
  Composite* projectsCustomUi = mMenus[LauncherMenu::Projects]->mCustomUi;
  TextButton* browseButton = new TextButton(projectsCustomUi, "SubTabText");
  browseButton->SetStyle(TextButtonStyle::Modern);
  browseButton->SetText("BROWSE");
  browseButton->mTextColor = ToByteColor(LauncherUi::MenuText);
  browseButton->mTextHoverColor = ToByteColor(LauncherUi::MenuTextHighlight);
  browseButton->mTextClickedColor = ToByteColor(LauncherUi::MenuTextHighlight);
  ConnectThisTo(browseButton, Events::ButtonPressed, OnBrowsePressed);

  mButtonArea->SizeToContents();

  mFileBugTextButton = new TextButton(this, mLauncherRegularFont, 11);
  mFileBugTextButton->SetNotInLayout(true);
  mFileBugTextButton->SetText("FILE BUG REPORT");
  mFileBugTextButton->SetStyle(TextButtonStyle::Modern);
  mFileBugTextButton->mTextColor = ToByteColor(LauncherUi::MenuText);
  mFileBugTextButton->mTextHoverColor = ToByteColor(LauncherUi::MenuTextHighlight);
  mFileBugTextButton->mTextClickedColor = ToByteColor(LauncherUi::MenuTextSelected);
  mFileBugTextButton->SizeToContents();
  ConnectThisTo(mFileBugTextButton, Events::ButtonPressed, OnFileBugPressed);

  // Select the discover page by default
  SelectMenu(LauncherMenu::Discover);

  // Start listening for inter-process communication
  StartListening();

  ConnectThisTo(mVersionSelector, Events::NewBuildAvailable, OnNewBuildAvailable);
  ConnectThisTo(parent, Events::Closing, OnClosing);

  ConnectThisTo(this, Events::MouseDown, OnMouseDown);

  OsWindow* osWindow = GetRootWidget()->GetOsWindow();
  ConnectThisTo(osWindow, Events::OsMouseFileDrop, OnOsMouseDrop);

  // Check to see if a new installer is available for the launcher
  if(launcherConfig->mAutoCheckForMajorUpdates)
    CheckForMajorLauncherUpdates();
}

//******************************************************************************
LauncherWindow::~LauncherWindow()
{
  for(size_t i = 0; i < mDummyCommunicators.Size(); ++i)
    delete mDummyCommunicators[i];
  mDummyCommunicators.Clear();

  delete mVersionSelector;
  delete mProjectCache;
}

//******************************************************************************
void LauncherWindow::CheckForUpdates()
{
  ZPrint("Checking for updates\n");

  // Start the task to get the version listing
  BackgroundTask* task = mVersionSelector->GetServerListing();
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnPackageListing);
  ConnectThisTo(task, Events::BackgroundTaskFailed, OnPackageListing);

  //start the task to get the version listing
  BackgroundTask* templateTask = mVersionSelector->GetTemplateListing();
  ConnectThisTo(templateTask, Events::BackgroundTaskCompleted, OnTemplateListing);
  ConnectThisTo(templateTask, Events::BackgroundTaskFailed, OnTemplateListing);
}

//******************************************************************************
void LauncherWindow::OnCheckForUpdates(Event* e)
{
  CheckForUpdates();
}

//******************************************************************************
void LauncherWindow::CheckForLauncherUpdates()
{
  CheckForMajorLauncherUpdates();
}

//******************************************************************************
void LauncherWindow::CheckForMajorLauncherUpdates()
{
  BackgroundTask* task = mVersionSelector->CheckForMajorLauncherUpdate();
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnCheckForMajorLauncherUpdates);
}

//******************************************************************************
void LauncherWindow::OnCheckForMajorLauncherUpdates(BackgroundTaskEvent* e)
{
  // If the task failed then something bad happened, just return
  if(!e->mTask->IsCompleted())
    return;

  // Check the job to see if there's a new major version, if not then
  // queue up a job to check for a new minor version (ignored for now, needs further
  // testing and minor versions are auto-downloaded on launcher).
  CheckForLauncherMajorInstallerJob* job = (CheckForLauncherMajorInstallerJob*)e->mTask->GetFinishedJob();
  if(!job->mIsNewInstallerAvailable)
  {
    //CheckForMinorLauncherUpdates();
    return;
  }

  // There is a new major version. Notify the user and ask them to install.
  String msg = "New Launcher update available. Close and run installer?";
  msg = msg.ToUpper();
  ModalConfirmAction* modal = new ModalConfirmAction(GetRootWidget(), msg);
  modal->mCloseOnSelection = true;
  mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnInstallMajorVersion);
}

//******************************************************************************
void LauncherWindow::CheckForLauncherPatch()
{
  String url = BuildString(VersionSelector::GetLauncherPhpUrl(), "?Commands=ListVersionId");
  DownloadTaskJob* job = new DownloadTaskJob(url);
  job->mName = "Version Id Check";
  BackgroundTask* task = Z::gBackgroundTasks->Execute(job, job->mName);

  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnCheckForLauncherPatch);
}

//******************************************************************************
void LauncherWindow::OnCheckForLauncherPatch(BackgroundTaskEvent* e)
{
  if(e->mTask->IsCompleted())
  {
    // Load the available builds into the version selector
    DownloadTaskJob* job = (DownloadTaskJob*)e->mTask->GetFinishedJob();
    int serverVersionId;
    ToValue(job->mData.c_str(), serverVersionId);

    // Load the id of the current running dll
    MainConfig* mainConfig = mConfigCog->has(MainConfig);
    String localIdFile = FilePath::Combine(mainConfig->ApplicationDirectory, "ZeroLauncherVersionId.txt");
    int localId = GetVersionId(localIdFile);

    // If the server has a newer package then ask the user if they want to update
    if(localId < serverVersionId)
    {
      String msg = "Launcher update available. Restart Launcher?";
      msg = msg.ToUpper();
      ModalConfirmAction* modal = new ModalConfirmAction(GetRootWidget(), msg);
      modal->mCloseOnSelection = true;
      mActiveModal = modal;
      ConnectThisTo(modal, Events::ModalConfirmResult, OnRestartPrompt);
    }
  }
}

//******************************************************************************
void LauncherWindow::OnInstallMajorVersion(ModalConfirmEvent* e)
{
  // If the user said no then just close the dialog (kinda bad that
  // they can continue through, but whatever for now...)
  if(!e->mConfirmed)
    return;

  BackgroundTask* task = mVersionSelector->DownloadMajorLauncherUpdate();
  ConnectThisTo(task, Events::BackgroundTaskCompleted, OnMajorLauncherUpdateDownloaded);

  ModalBackgroundTaskProgessBar* modal = new ModalBackgroundTaskProgessBar(GetRootWidget(), "Downloading", task);
  mActiveModal = modal;
}

//******************************************************************************
void LauncherWindow::OnMajorLauncherUpdateDownloaded(BackgroundTaskEvent* e)
{
  // If the task failed then something bad happened, just return
  if(!e->mTask->IsCompleted())
    return;

  // Check the job to see if there's a new major version, if not then
  // queue up a job to check for a new minor version (ignored for now, needs further
  // testing and minor versions are auto-downloaded on launcher).
  DownloadLauncherMajorInstallerJob* job = (DownloadLauncherMajorInstallerJob*)e->mTask->GetFinishedJob();
  if(!job->mIsNewInstallerAvailable)
  {
    //CheckForMinorLauncherUpdates();
    return;
  }

  // Invoke the installer in such a way that it'll run silently (not very silent, this still
  // shows progress but has no prompts) and close and re-open the launcher.
  Status status;
  Os::SystemOpenFile(status, job->mInstallerPath.c_str(), Os::Verb::Default, "/SILENT /CLOSEAPPLICATIONS /RESTARTAPPLICATIONS");
}

//******************************************************************************
void LauncherWindow::OnRestartPrompt(ModalConfirmEvent* e)
{
  // If the user confirmed that they want to restart
  if(e->mConfirmed)
  {
    // Mark that we should restart when closing and terminate the engine
    mConfigCog->has(LauncherConfig)->mRestartOnClose = true;
    Z::gEngine->Terminate();
  }
}

//******************************************************************************
void LauncherWindow::ForceUpdateBuilds()
{
  bool versionIsRunning = false;
  for(size_t i = 0; i < mVersionSelector->mVersions.Size(); ++i)
  {
    ZeroBuild* build = mVersionSelector->mVersions[i];
    if(build->mInstallState != InstallState::NotInstalled && mVersionSelector->CheckForRunningBuild(build))
      versionIsRunning = true;
  }

  if(!versionIsRunning)
  {
    ForceUpdateBuildsAndUpdateConfig();
    return;
  }

  String msg = "An update is required";
  String extraMsg = "Please close all open builds of zero and try again";
  // Create the buttons that we will display and listen for responses from
  Array<String> buttons;
  buttons.PushBack("RETRY");
  buttons.PushBack("FORCE CLOSE");
  // Create the modal and connect to whenever a button is pressed
  ModalButtonsAction* modal = new ModalButtonsAction(this, msg.ToUpper(), buttons, extraMsg);
  modal->SetStripHeight(ModalSizeMode::Percentage, 0.145f);
  ConnectThisTo(modal, Events::ModalButtonPressed, OnForcedBuildsModal);
  modal->mCloseOnBackgroundClicked = false;
  modal->TakeFocus();
  // Make sure to set what the active modal is on the launcher window (so escape can cancel)
  mActiveModal = modal;
}

//******************************************************************************
void LauncherWindow::ForceUpdateBuildsAndUpdateConfig()
{
  LauncherConfig* launcherConfig = mConfigCog->has(LauncherConfig);
  mVersionSelector->ForceUpdateAllBuilds();
  launcherConfig->mForcedUpdateVersion = LauncherConfig::mCurrentForcedUpdateVersionNumber;
  SaveLauncherConfig(mConfigCog);
}

//******************************************************************************
void LauncherWindow::OnForcedBuildsModal(ModalButtonEvent* e)
{
  if(e->mButtonName == "RETRY")
    ForceUpdateBuilds();
  else
    ForceUpdateBuildsAndUpdateConfig();
}

//******************************************************************************
void LauncherWindow::UpdateTransform()
{
  // Center the background
  Rect centerRect = mCenterPanel->GetRectInParent();
  PlaceWithRect(centerRect, mCenterPanelBackground);

  mWindowGripper->SetSize(mTopBar->GetSize());

  // Update the size of the current menu
  if(mSelectedMenu && mSelectedMenu->mClientArea)
    mSelectedMenu->mClientArea->SetSize(centerRect.GetSize());

  float rightEdge = centerRect.SizeX;
  float buttonSize = mFileBugTextButton->GetSize().x;
  float buttonLeftEdge = rightEdge - 36 - buttonSize;
  mFileBugTextButton->SetTranslation(Pixels(buttonLeftEdge, 33, 0));

  Composite::UpdateTransform();
}

//******************************************************************************
MenuData* LauncherWindow::RegisterMenu(LauncherMenu::Type menu,
                      Composite* clientComposite, LauncherMenu::Type parentMenu)
{
  // The menu is disabled by default
  if(clientComposite)
    clientComposite->SetActive(false);

  // Create the menu
  MenuData* menuData = new MenuData();
  menuData->mMenu = menu;
  menuData->mParentMenu = parentMenu;
  menuData->mMenuButton = nullptr;

  // Create custom Ui for this menu
  menuData->mCustomUi = new Composite(mCustomMenuArea);
  menuData->mCustomUi->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(16, 0), Thickness(Pixels(32, -15, 0, 0))));
  new Spacer(menuData->mCustomUi, SizePolicy::Fixed, Pixels(35, 1));
  menuData->mClientArea = clientComposite;

  // No sub menu selected by default
  menuData->mSelectedSubMenu = LauncherMenu::None;

  // Disable all custom ui until the menu is selected
  menuData->mCustomUi->SetActive(false);

  mMenus[menu] = menuData;

  // If we have a parent menu, add buttons
  if(parentMenu != LauncherMenu::None)
  {
    MenuData* parent = mMenus[parentMenu];
    ErrorIf(parent == nullptr, "Parent menu not registered.");

    parent->mSubMenus.PushBack(menuData);
  }

  return menuData;
}

//******************************************************************************
void LauncherWindow::SetDefaultSubMenu(LauncherMenu::Type parentMenu,
                                       LauncherMenu::Type defaultSubMenu)
{
  mMenus[parentMenu]->mSelectedSubMenu = defaultSubMenu;
}

//******************************************************************************
void LauncherWindow::AddMenuButton(LauncherMenu::Type menu, StringParam name,
                                   StringParam style)
{
  MenuData* menuData = mMenus[menu];
  
  Composite* parentComposite = mButtonArea;

  // If we have a parent menu, we want to attach the button to the parents
  // custom ui area (underneath the main buttons).
  if(menuData->mParentMenu != LauncherMenu::None)
  {
    MenuData* parentMenu = mMenus[menuData->mParentMenu];
    parentComposite = parentMenu->mCustomUi;
  }

  // Create a button at the top of the launcher
  MenuTextButton* button = new MenuTextButton(parentComposite, style, menu);
  button->SetText(name);
  ConnectThisTo(button, Events::ButtonPressed, OnMenuButtonPressed);

  menuData->mMenuButton = button;
}

//******************************************************************************
void LauncherWindow::DisplayInClientArea(Composite* composite)
{
  // Fade the old menu if it exists
  if(mSelectedMenu && mSelectedMenu->mClientArea)
  {
    Composite* menu = mSelectedMenu->mClientArea;

    // Disable the menu
    Event eventToSend;
    menu->DispatchEvent(Events::MenuHidden, &eventToSend);

    ActionSequence* seq = new ActionSequence(menu);
    seq->Add(Fade(menu, Vec4(1,1,1,0), 0.1f));
    seq->Add(new CallParamAction<Widget, bool, &Widget::SetActive>(menu, false));
  }

  // Do nothing if we weren't given anything
  if(composite == nullptr)
    return;

  // Enable the menu
  Event eventToSend;
  composite->DispatchEvent(Events::MenuDisplayed, &eventToSend);

  // Activate the new menu and fade it in
  composite->SetActive(true);
  composite->SetColor(Vec4(1,1,1,0));

  ActionSequence* seq = new ActionSequence(composite);
  seq->Add(Fade(composite, Vec4(1,1,1,1), 0.2f));

  // Make sure it's sized properly (we may have changed size)
  composite->SetSize(mCenterPanel->GetSize());
}

//******************************************************************************
void LauncherWindow::CloseActiveModals()
{
  if(Modal* modal = mActiveProjectModal)
  {
    modal->Close();
    mActiveProjectModal = nullptr;
  }

  if(Modal* modal = mSettingsModal)
  {
    modal->Close();
    mSettingsModal = nullptr;
  }

  if(Modal* modal = mActiveModal)
  {
    modal->Close();
    mActiveModal = nullptr;
  }
}

//******************************************************************************
void LauncherWindow::SelectMenu(LauncherMenu::Type menu)
{
  MenuData* menuData = mMenus[menu];
  SelectMenu(menuData);
}

//******************************************************************************
void LauncherWindow::SelectMenu(MenuData* menu)
{
  if(Modal* modal = mActiveProjectModal)
  {
    modal->Close();
    mActiveProjectModal = nullptr;
  }

  if(Modal* modal = mSettingsModal)
  {
    modal->Close();
    mSettingsModal = nullptr;
  }

  // Don't select a menu if it's already selected (in theory we need to check
  // for sub-menus, but this hasn't been an issues so far)
  if(mSelectedMenu == menu)
    return;

  DeSelectAllButton();

  MenuData* selected = menu;

  // Highlight the selected menu button
  if(selected->mMenuButton)
    selected->mMenuButton->Select();

  // Highlight the parent menu if it exists
  if(selected->mParentMenu != LauncherMenu::None)
    mMenus[selected->mParentMenu]->mMenuButton->Select();

  // Highlight the default child menu if it exists
  if(selected->mSelectedSubMenu != LauncherMenu::None)
  {
    MenuData* childMenu = mMenus[selected->mSelectedSubMenu];
    if(childMenu->mMenuButton)
      childMenu->mMenuButton->Select();
    
    // We want to select the child menu
    selected = childMenu;
  }

  // Show the menu in the center panel
  DisplayInClientArea(selected->mClientArea);

  // Disable old custom ui
  if(mSelectedMenu)
  {
    // Mark the old menu's custom ui as inactive
    if(mSelectedMenu->mCustomUi)
      mSelectedMenu->mCustomUi->SetActive(false);

    // If the old menu had a parent, make the parents custom ui inactive as well
    if(mSelectedMenu->mParentMenu != LauncherMenu::None)
    {
      MenuData* parentMenuData = mMenus[mSelectedMenu->mParentMenu];
      if(parentMenuData->mCustomUi)
        parentMenuData->mCustomUi->SetActive(false);
    }
  }

  // If we have a parent, we want to display our parents custom ui
  if(selected->mParentMenu != LauncherMenu::None)
  {
    MenuData* parentMenuData = mMenus[selected->mParentMenu];
    parentMenuData->mSelectedSubMenu = selected->mMenu;
    parentMenuData->mCustomUi->SetActive(true);
  }
  // Otherwise, just display the selected menu's custom ui
  if(selected->mCustomUi)
    selected->mCustomUi->SetActive(true);

  // If there is a default sub page, go to that
  if(selected->mSelectedSubMenu != LauncherMenu::None)
    DisplayInClientArea(mMenus[selected->mSelectedSubMenu]->mClientArea);

  // Store it as the selected menu
  mSelectedMenu = selected;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void LauncherWindow::AddToRecentProjects(CachedProject* project)
{
  // Add the project to the recent projects listing
  RecentProjects* recentProjects = mConfigCog->has(RecentProjects);
  recentProjects->AddRecentProject(project->GetProjectPath(), true);
  SaveLauncherConfig(mConfigCog);
}

//******************************************************************************
void LauncherWindow::RemoveFromRecentProjects(CachedProject* project)
{
  // Add the project to the recent projects listing
  RecentProjects* recentProjects = mConfigCog->has(RecentProjects);
  recentProjects->RemoveRecentProject(project->GetProjectPath(), true);
  SaveLauncherConfig(mConfigCog);
}

//******************************************************************************
void LauncherWindow::SelectActiveProject(CachedProject* project, bool overrideUserTags)
{
  SelectMenu(LauncherMenu::RecentProjects);

  mActiveProjectModal = ActiveProjectMenu::OpenProject(mCenterPanel, project, this);
  mActiveModal = mActiveProjectModal;
}

//******************************************************************************
void LauncherWindow::OnMenuButtonPressed(ObjectEvent* e)
{
  MenuTextButton* button = (MenuTextButton*)e->Source;
  LauncherMenu::Type selectedMenu = button->mMenu;

  SelectMenu(selectedMenu);
}

//******************************************************************************
void LauncherWindow::OnSettingsPressed(Event*)
{
  Modal* modal = new Modal(GetRootWidget());

  SettingsMenu* menu = new SettingsMenu(modal, this);
  Vec2 rootSize = GetRootWidget()->GetSize();
  menu->SetNotInLayout(true);

  mSettingsModal = modal;
}

//******************************************************************************
void LauncherWindow::OnPackageListing(BackgroundTaskEvent* taskEvent)
{
  bool installingLatest = false;
  if(taskEvent->mTask->IsCompleted())
  {
    // Load the available builds into the version selector
    GetVersionListingTaskJob* job = (GetVersionListingTaskJob*)taskEvent->mTask->GetFinishedJob();
    mVersionSelector->UpdatePackageListing(job);

    // Notify that the version list has been loaded
    Event e;
    mVersionSelector->GetDispatcher()->Dispatch(Events::VersionListLoaded, &e);

    // If the user doesn't have any builds installed and has no recent projects then install the latest "Stable" version
    RecentProjects* recentProjects = mConfigCog->has(RecentProjects);
    if(mVersionSelector->GetInstalledBuildsCount() == 0 &&
      (recentProjects == nullptr || recentProjects->GetRecentProjectsCount() == 0))
    {
      HashSet<String> requiredTags;
      HashSet<String> rejectionTags;
      requiredTags.Insert("Stable");
      rejectionTags.Insert("Develop");
      ZeroBuild* latest = mVersionSelector->GetLatestBuild(requiredTags, rejectionTags);

      // Install the latest
      if(latest != nullptr)
      {
        mVersionSelector->InstallVersion(latest);
        installingLatest = true;
      }
    }
  }
  
  if(!installingLatest)
    CheckForForcedBuildUpdate();
}

//******************************************************************************
void LauncherWindow::OnTemplateListing(BackgroundTaskEvent* taskEvent)
{
  if(taskEvent->mTask->IsCompleted())
  {
    // Load the available builds into the version selector
    GetTemplateListingTaskJob* job = (GetTemplateListingTaskJob*)taskEvent->mTask->GetFinishedJob();
    mVersionSelector->UpdateTemplateListing(job);

    // Notify that the version list has been loaded
    Event e;
    mVersionSelector->GetDispatcher()->Dispatch(Events::TemplateListLoaded, &e);
  }
}

//******************************************************************************
void LauncherWindow::CheckForForcedBuildUpdate()
{
  LauncherConfig* launcherConfig = mConfigCog->has(LauncherConfig);
  if(launcherConfig->mForcedUpdateVersion != LauncherConfig::mCurrentForcedUpdateVersionNumber)
    ForceUpdateBuilds();
}

//******************************************************************************
void LauncherWindow::OnNewBuildAvailable(Event* e)
{
  // Display some special notification letting them know something new is available?
  ZPrint("New Build Available");
}

//******************************************************************************
void LauncherWindow::OnClosing(Event* e)
{
  Z::gEngine->Terminate();
}

//******************************************************************************
void LauncherWindow::DeSelectAllButton()
{
  // De-select all other buttons and select the one that was pressed
  for(uint i = 0; i < LauncherMenu::MenuCount; ++i)
  {
    if(mMenus[i] == nullptr)
      continue;

    if(MenuTextButton* button = mMenus[i]->mMenuButton)
      button->DeSelect();
  }
}

//******************************************************************************
LauncherConfig* LauncherWindow::GetConfig()
{
  return mVersionSelector->mConfig;
}

//******************************************************************************
void LauncherWindow::OnMouseDown(MouseEvent* e)
{
  // The user pressed back, close the last active modal page if possible. Don't
  // close all modals as currently the active project page can open a modal
  // (for changing project version numbers) and only 1 modal should close.
  if(e->Button == MouseButtons::XOneBack)
  {
    Modal* modal = mActiveModal;
    if(modal)
    {
      modal->Close();
      return;
    }

    modal = mActiveProjectModal;
    if(modal)
    {
      modal->Close();
      return;
    }

    modal = mSettingsModal;
    if(modal)
    {
      modal->Close();
      return;
    }
  }
}

//******************************************************************************
void LauncherWindow::OnOsMouseDrop(OsMouseDropEvent* e)
{
  for(size_t i = 0; i < e->Files.Size(); ++i)
  {
    String filePath = e->Files[i];
    ZPrint("File '%s' was dropped on the launcher\n", filePath.c_str());

    String ext = FilePath::GetExtension(filePath).ToLower();
    // If the file is a zeroproj then display it
    if(ext == "zeroproj")
    {
      LauncherCommunicationEvent toSend;
      toSend.mProjectFile = filePath;
      DispatchEvent(Events::LauncherOpenProject, &toSend);
    }
    // If it is a .zerobuild then install the build file
    else if(ext == ZeroBuild::mExtension)
    {
      LauncherCommunicationEvent toSend;
      toSend.mProjectFile = filePath;
      DispatchEvent(Events::LauncherOpenBuild, &toSend);
    }
    // If this is a .zerotemplate then install the template package
    else if(ext == TemplateProject::mExtensionWithoutDot)
    {
      LauncherCommunicationEvent toSend;
      toSend.mProjectFile = filePath;
      DispatchEvent(Events::LauncherOpenTemplate, &toSend);
    }
    else if(ext == "zeroprojpack")
    {
      LauncherCommunicationEvent toSend;
      toSend.mProjectFile = filePath;
      DispatchEvent(Events::LauncherInstallProject, &toSend);
    }
  }
}

//******************************************************************************
void LauncherWindow::OnInstallLocalBuild(ModalConfirmEvent* e)
{
  if(e->mConfirmed)
  {
    // Select the builds page
    SelectMenu(LauncherMenu::Builds);
    // Add the build and install it
    mVersionSelector->AddCustomBuild(e->mStringUserData, true);
  }
}

//******************************************************************************
void LauncherWindow::OnInstallTemplateProject(ModalConfirmEvent* e)
{
  if(!e->mConfirmed)
    return;
  
  // Select the new project page and install the template project
  SelectMenu(LauncherMenu::NewProject);
  bool success = mVersionSelector->InstallLocalTemplateProject(e->mStringUserData);
  if(!success)
  {
    String msg = "Invalid template project file";
    String extraMsg = "Template must contain a meta file or be of the format Name[BuildId]";
    ModalButtonsAction* modal = new ModalButtonsAction(this, msg.ToUpper(), "OK", extraMsg);
    mActiveModal = modal;
  }
}

//******************************************************************************
void LauncherWindow::OnInstallProjectPack(ModalConfirmEvent* e)
{
  if(!e->mConfirmed)
    return;

  // Unzip the project to the current default project save location using the zip file's name
  LauncherConfig* launcherConfig = mConfigCog->has(LauncherConfig);
  String projectLocation = launcherConfig->mDefaultProjectSaveLocation;
  String exportDirectory = FilePath::Combine(projectLocation, FilePath::GetFileNameWithoutExtension(e->mStringUserData));

  File file;
  file.Open(e->mStringUserData, FileMode::Read, FileAccessPattern::Random);

  Archive archive(ArchiveMode::Decompressing);
  archive.ReadZip(ArchiveReadFlags::All, file);
  archive.ExportToDirectory(ArchiveExportMode::Overwrite, exportDirectory);

  // Find the zeroproj file
  Array<String> files;
  ExtensionFilterFile filter("zeroproj");
  FindFilesRecursively(exportDirectory, files, &filter);

  // If there's no zeroproj then return (prob should front-load this during drag-drop)
  if(files.Empty())
    return;

  String zeroProjPath = files[0];

  // Load the project and display it
  CachedProject* project = mProjectCache->LoadProjectFile(zeroProjPath);
  if(project == nullptr)
  {
    String msg = "Invalid Project File";
    ModalButtonsAction* modal = new ModalButtonsAction(this, msg.ToUpper(), "CLOSE");
    modal->mStringUserData = zeroProjPath;
    mActiveModal = modal;
    return;
  }

  AddToRecentProjects(project);
  SelectActiveProject(project, true);
}

//******************************************************************************
void LauncherWindow::OnBrowsePressed(Event* e)
{
  //Set up the callback for when project file is selected
  const String cCallBackEvent = "OpenProjectCallback"; 
  if(!GetDispatcher()->IsConnected(cCallBackEvent, this))
    ConnectThisTo(this, cCallBackEvent, OnOpenProjectFile);

  LauncherConfig* config = mConfigCog->has(LauncherConfig);
  
  //Open the open file dialog
  FileDialogConfig fileConfig;
  fileConfig.EventName = cCallBackEvent;
  fileConfig.CallbackObject = this;
  fileConfig.Title = "Open a project";
  fileConfig.AddFilter("Zero Project File", "*.zeroproj");
  fileConfig.StartingDirectory = config->mDefaultProjectSaveLocation;
  Z::gEngine->has(OsShell)->OpenFile(fileConfig);
}

//******************************************************************************
void LauncherWindow::OnOpenProjectFile(OsFileSelection* e)
{
  if(!e->Success)
    return;

  String filePath = e->Files[0];
  CachedProject* project = mProjectCache->LoadProjectFile(filePath);
  if(project == nullptr)
  {
    String msg = "Invalid Project File";
    ModalButtonsAction* modal = new ModalButtonsAction(this, msg.ToUpper(), "CLOSE");
    modal->mStringUserData = filePath;
    mActiveModal = modal;
    return;
  }

  AddToRecentProjects(project);
  SelectActiveProject(project, true);
}

//******************************************************************************
void LauncherWindow::OnFileBugPressed(Event* e)
{
  Os::SystemOpenNetworkFile("https://www.zeroengine.io/u/reportbug");
}

//******************************************************************************
void LauncherWindow::OsWindowTakeFocus()
{
  OsWindow* window = GetRootWidget()->GetOsWindow();
  window->TakeFocus();
}

//******************************************************************************
void LauncherWindow::StartListening()
{
  mListener = new TcpSocket(Protocol::Events | Protocol::Chunks, "IPC-Listener");

  // Listen for interprocess communication from zero or another launcher
  ConnectThisTo(mListener, Events::LauncherUpdateTags, OnLauncherUpdateTags);
  ConnectThisTo(mListener, Events::LauncherNewProject, OnLauncherNewProject);
  ConnectThisTo(mListener, Events::LauncherOpenProject, OnLauncherOpenProject);
  ConnectThisTo(mListener, Events::LauncherRunProject, OnLauncherRunProject);
  ConnectThisTo(mListener, Events::LauncherOpenRecentProjects, OnLauncherOpenRecentProjects);
  ConnectThisTo(mListener, Events::LauncherRunCommand, OnLauncherRunCommand);
  ConnectThisTo(mListener, Events::LauncherOpenBuild, OnLauncherOpenBuild);
  ConnectThisTo(mListener, Events::LauncherOpenTemplate, OnLauncherOpenTemplate);
  ConnectThisTo(mListener, Events::LauncherInstallProject, OnLauncherInstallProject);
  // Also listen for the same events on ourself (to make parsing command-line arguments easier)
  ConnectThisTo(this, Events::LauncherUpdateTags, OnLauncherUpdateTags);
  ConnectThisTo(this, Events::LauncherNewProject, OnLauncherNewProject);
  ConnectThisTo(this, Events::LauncherOpenProject, OnLauncherOpenProject);
  ConnectThisTo(this, Events::LauncherRunProject, OnLauncherRunProject);
  ConnectThisTo(this, Events::LauncherOpenRecentProjects, OnLauncherOpenRecentProjects);
  ConnectThisTo(this, Events::LauncherRunCommand, OnLauncherRunCommand);
  ConnectThisTo(this, Events::LauncherOpenBuild, OnLauncherOpenBuild);
  ConnectThisTo(this, Events::LauncherOpenTemplate, OnLauncherOpenTemplate);
  ConnectThisTo(this, Events::LauncherInstallProject, OnLauncherInstallProject);
  mListener->Listen(LauncherCommunicationEvent::DesiredPort, TcpSocket::MaxPossibleConnections, TcpSocketBind::Loopback);
}

//******************************************************************************
void LauncherWindow::OnLauncherUpdateTags(LauncherCommunicationEvent* e)
{
  // If the event provided any new tags then load them (should only be at the
  // moment from another launcher opening with a different mode, such as the ProjectFun shortcut)
  LauncherConfig* launcherConfig = mConfigCog->has(LauncherConfig);

  if(e->mExtraData == LauncherStartupArguments::Names[LauncherStartupArguments::DebuggerMode])
    launcherConfig->mRunDebuggerMode = true;
}

//******************************************************************************
void LauncherWindow::OnLauncherNewProject(LauncherCommunicationEvent* e)
{
  ZPrint("New Project invoked by another process\n");
  // This needs to be before selection so focus doesn't change
  OsWindowTakeFocus();

  // Update our tags (if any were passed to us)
  OnLauncherUpdateTags(e);

  SelectMenu(LauncherMenu::NewProject);
}

//******************************************************************************
void LauncherWindow::OnLauncherOpenProject(LauncherCommunicationEvent* e)
{
  ZPrint("Open Project invoked by another process\n");
  // This needs to be before selection so focus doesn't change
  OsWindowTakeFocus();
  // Update our tags (if any were passed to us)
  OnLauncherUpdateTags(e);

  // If the config settings say to always run the project when installed then
  // run the project (and show it in the active project page)
  LauncherConfig* config = mConfigCog->has(LauncherConfig);

  if(config->mAutoRunMode == LauncherAutoRunMode::IfInstalled)
    OnLauncherRunProject(e);

  CachedProject* project = mProjectCache->LoadProjectFile(e->mProjectFile);
  // We were told to open a project but it didn't exist so don't do anything
  if(project == nullptr)
    return;

  AddToRecentProjects(project);
  SelectActiveProject(project, true);
}

//******************************************************************************
void LauncherWindow::OnLauncherRunProject(LauncherCommunicationEvent* e)
{
  ZPrint("Run Project invoked by another process\n");
  // This needs to be before selection so focus doesn't change
  OsWindowTakeFocus();
  // Update our tags (if any were passed to us)
  OnLauncherUpdateTags(e);

  CachedProject* project = mProjectCache->LoadProjectFile(e->mProjectFile);
  // We were told to run a project but it didn't exist so don't do anything
  if(project == nullptr)
    return;

  AddToRecentProjects(project);

  String projectFilePath = e->mProjectFile;
  ZeroBuild* standalone = mVersionSelector->FindExactVersion(project);
  if(standalone != nullptr)
  {
    if(standalone->mInstallState == InstallState::Installed)
      mVersionSelector->RunProject(standalone, project);
  }
  SaveLauncherConfig(mConfigCog);

  SelectActiveProject(project, true);
}

//******************************************************************************
void LauncherWindow::OnLauncherOpenRecentProjects(LauncherCommunicationEvent* e)
{
  ZPrint("Recent Projects invoked by another process\n");
  // This needs to be before selection so focus doesn't change
  OsWindowTakeFocus();
  // Update our tags (if any were passed to us)
  OnLauncherUpdateTags(e);

  SelectMenu(LauncherMenu::RecentProjects);
}

//******************************************************************************
void LauncherWindow::OnLauncherRunCommand(LauncherCommunicationEvent* e)
{
  ZPrint("Run Command invoked by another process\n");
  // This needs to be before selection so focus doesn't change
  OsWindowTakeFocus();

  CommandManager* commands = CommandManager::GetInstance();
  Command* command = commands->GetCommand(e->mExtraData);
  if(command != nullptr)
    command->Execute();
}

//******************************************************************************
void LauncherWindow::OnLauncherOpenBuild(LauncherCommunicationEvent* e)
{
  OsWindowTakeFocus();
  CloseActiveModals();
  // Add a modal dialog to ask if they want to install the local build
  String msg = "Install build?";
  ModalConfirmAction* modal = new ModalConfirmAction(this, msg.ToUpper());
  modal->mStringUserData = e->mProjectFile;
  mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnInstallLocalBuild);
}

//******************************************************************************
void LauncherWindow::OnLauncherOpenTemplate(LauncherCommunicationEvent* e)
{
  OsWindowTakeFocus();
  CloseActiveModals();
  // Add a modal dialog to ask if they want to install the template locally
  String msg = "Install template project?";
  ModalConfirmAction* modal = new ModalConfirmAction(this, msg.ToUpper());
  modal->mStringUserData = e->mProjectFile;
  mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnInstallTemplateProject);
}

//******************************************************************************
void LauncherWindow::OnLauncherInstallProject(LauncherCommunicationEvent* e)
{
  OsWindowTakeFocus();
  CloseActiveModals();
  // Add a modal dialog to ask if they want to install the template locally
  String msg = "Install project package?";
  ModalConfirmAction* modal = new ModalConfirmAction(this, msg.ToUpper());
  modal->mStringUserData = e->mProjectFile;
  mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnInstallProjectPack);
}

}//namespace Zero
