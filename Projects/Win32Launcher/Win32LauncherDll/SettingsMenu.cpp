///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(LauncherConfigChanged);
}//namespace Events

//------------------------------------------------------------------- Tweakables
namespace SettingsUi
{
  const cstr cLocation = "LauncherUi/Settings";
  Tweakable(Vec4, BackgroundColor, Vec4(1,1,1,1), cLocation);
  Tweakable(float, SlideInTime, 0.25f, cLocation);
  Tweakable(Vec4, SettingsColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SettingsHoverColor, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, SettingsClickedColor, Vec4(1,1,1,1), cLocation);

  Tweakable(Vec4, TextBoxButtonHover, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, TextBoxButtonClicked, Vec4(1,1,1,1), cLocation);
}

//******************************************************************************
SettingsMenu::SettingsMenu(Modal* parent, LauncherWindow* launcher) :
  Composite(parent),
  mLauncher(launcher),
  mParentModal(parent)
{
  ZPrint("Settings Menu Displayed\n");
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 5), Thickness(32, 52, 32, 22)));
  
  // Create the background and slide it in
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(SettingsUi::BackgroundColor);
  mBackground->SetTranslation(Pixels(-500, 0, 0));

  ActionGroup* group = new ActionGroup(this);

  LauncherConfig* config = launcher->mConfigCog->has(LauncherConfig);

  Composite* topRow = new Composite(this);
  topRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(23, 0), Thickness(-2,-3,0,0)));
  {
    IconButton* settingsButton = new IconButton(topRow);
    settingsButton->SetIcon("OptionsIcon");
    settingsButton->mBackground->SetVisible(false);
    settingsButton->mBorder->SetVisible(false);
    settingsButton->mIconColor = ToByteColor(SettingsUi::SettingsColor);
    settingsButton->mIconHoverColor = ToByteColor(SettingsUi::SettingsHoverColor);
    settingsButton->mIconClickedColor = ToByteColor(SettingsUi::SettingsClickedColor);
    settingsButton->SetColor(Vec4(1,1,1,0));
    ConnectThisTo(settingsButton, Events::ButtonPressed, OnSettingsPressed);

    group->Add(Fade(settingsButton, Vec4(1,1,1,1), SettingsUi::SlideInTime));

    Text* settingsText = new Text(topRow, "MainTabText");
    settingsText->SetText("SETTINGS");
    settingsText->SizeToContents();
    settingsText->SetColor(Vec4(1,1,1,0));

    group->Add(Fade(settingsText, SettingsUi::SettingsColor, SettingsUi::SlideInTime * 2.0f));
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 28));

  // AutoRunMode
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("PROJECT SETTINGS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    StringSource* source = new StringSource();
    source->Strings.Resize(2);
    source->Strings[LauncherAutoRunMode::None] = "Always open launcher";
    source->Strings[LauncherAutoRunMode::IfInstalled] = "Auto run project if build installed";
    // Don't do this option for now, maybe later
    //source->Strings[LauncherAutoRunMode::InstallAndRun] = "Auto run project";
    mAutoRunMode = new ComboBox(this);
    mAutoRunMode->SetListSource(source);
    mAutoRunMode->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(200));
    //mAutoRunMode->mBackgroundColor = ToByteColor(ModernTextBoxUi::BackgroundColor);
    ConnectThisTo(mAutoRunMode, Events::ItemSelected, OnAutoRunModeSelected);
    mWidgetsToAnimate.PushBack(mAutoRunMode);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 10));

  // Default Project Location
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("DEFAULT PROJECT LOCATION");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mDefaultProjectLocation = new TextBoxButton(this, "OpenFolderIcon");
    mDefaultProjectLocation->SetStyle(TextBoxStyle::Modern);
    mDefaultProjectLocation->SetEditable(true);
    mDefaultProjectLocation->mEditTextField->mFont->mFontHeight = 10;
    mDefaultProjectLocation->mButton->mBackgroundColor = ToByteColor(SettingsUi::BackgroundColor);
    mDefaultProjectLocation->mButton->mBackgroundHoverColor = ToByteColor(SettingsUi::TextBoxButtonHover);
    mDefaultProjectLocation->mButton->mBackgroundClickedColor = ToByteColor(SettingsUi::TextBoxButtonClicked);
    ConnectThisTo(mDefaultProjectLocation, Events::TextSubmit, OnProjectLocationTextSubmit);
    mWidgetsToAnimate.PushBack(mDefaultProjectLocation);
    ConnectThisTo(mDefaultProjectLocation->mButton, Events::ButtonPressed, OnBrowseProjectLocation);
  }

  // Download location
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("DEFAULT DOWNLOAD LOCATION");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mDownloadLocation = new TextBoxButton(this, "OpenFolderIcon");
    mDownloadLocation->SetStyle(TextBoxStyle::Modern);
    mDownloadLocation->SetEditable(true);
    mDownloadLocation->mButton->mBackgroundColor = ToByteColor(SettingsUi::BackgroundColor);
    mDownloadLocation->mButton->mBackgroundHoverColor = ToByteColor(SettingsUi::TextBoxButtonHover);
    mDownloadLocation->mButton->mBackgroundClickedColor = ToByteColor(SettingsUi::TextBoxButtonClicked);
    ConnectThisTo(mDownloadLocation, Events::TextChanged, OnDownloadLocationTextChanged);
    ConnectThisTo(mDownloadLocation, Events::TextSubmit, OnDownloadLocationTextSubmit);
    mWidgetsToAnimate.PushBack(mDownloadLocation);
    ConnectThisTo(mDownloadLocation->mButton, Events::ButtonPressed, OnBrowseDownloadLocation);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 10));

  // Max recent projects
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("MAX RECENT PROJECTS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mMaxNumberOfRecentProjects = new TextBox(this);
    mMaxNumberOfRecentProjects->SetEditable(true);
    mMaxNumberOfRecentProjects->SetStyle(TextBoxStyle::Modern);
    mMaxNumberOfRecentProjects->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(160));
    ConnectThisTo(mMaxNumberOfRecentProjects, Events::TextSubmit, OnMaxRecentProjectsModified);
    mWidgetsToAnimate.PushBack(mMaxNumberOfRecentProjects);
  }
  
  // Auto check for major updates
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("AUTO CHECK FOR MAJOR UPDATES");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mAutoCheckForMajorUpdatesCheckBox = new CheckBox(this);
    mAutoCheckForMajorUpdatesCheckBox->SetChecked(config->mAutoCheckForMajorUpdates);
    mAutoCheckForMajorUpdatesCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mAutoCheckForMajorUpdatesCheckBox, Events::ValueChanged, OnAutoCheckForMajorUpdatesModified);
    mWidgetsToAnimate.PushBack(mAutoCheckForMajorUpdatesCheckBox);
  }

  // Show development builds
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("SHOW DEVELOPMENT BUILDS");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mShowDevelopCheckBox = new CheckBox(this);
    mShowDevelopCheckBox->SetChecked(false);
    mShowDevelopCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mShowDevelopCheckBox, Events::ValueChanged, OnShowDevelopModified);
    mWidgetsToAnimate.PushBack(mShowDevelopCheckBox);
  }

  // Show experimental branches
  {
    Text* text = new Text(this, mLauncherRegularFont, 11);
    text->SetText("SHOW EXPERIMENTAL BRANCHES");
    text->SetColor(SettingsUi::SettingsColor);
    mWidgetsToAnimate.PushBack(text);

    new Spacer(this, SizePolicy::Fixed, Pixels(0, -9));

    mShowExperimentalBranchesCheckBox = new CheckBox(this);
    mShowExperimentalBranchesCheckBox->SetChecked(false);
    mShowExperimentalBranchesCheckBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));
    ConnectThisTo(mShowExperimentalBranchesCheckBox, Events::ValueChanged, OnShowExperimentalBranchesModified);
    mWidgetsToAnimate.PushBack(mShowExperimentalBranchesCheckBox);
  }

  new Spacer(this, SizePolicy::Flex, Vec2(1, 1));

  if(Z::gEngine->GetConfigCog()->has(DeveloperConfig))
  {
    // Check for updates (might go away)
    TextButton* checkForUpdatesButton = new TextButton(this);
    checkForUpdatesButton->SetText("Check for Updates");
    ConnectThisTo(checkForUpdatesButton, Events::ButtonPressed, OnCheckForUpdates);
    mWidgetsToAnimate.PushBack(checkForUpdatesButton);

    // Check for updates (might go away)
    TextButton* checkForLauncherUpdatesButton = new TextButton(this);
    checkForLauncherUpdatesButton->SetText("Check for Launcher Updates");
    ConnectThisTo(checkForLauncherUpdatesButton, Events::ButtonPressed, OnCheckForLauncherUpdates);
    mWidgetsToAnimate.PushBack(checkForLauncherUpdatesButton);
  }

  // Revert settings
  TextButton* revertSettingsButton = new TextButton(this);
  revertSettingsButton->SetText("Revert to default");
  revertSettingsButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(160));
  ConnectThisTo(revertSettingsButton, Events::ButtonPressed, OnRevertToDefaults);
  mWidgetsToAnimate.PushBack(revertSettingsButton);

  ConnectThisTo(this, Events::Activated, OnActivated);
  ConnectThisTo(this, Events::Deactivated, OnDeactivated);

  // Update all of the ui to display the current config values
  LoadFromConfig();

  ConnectThisTo(mParentModal, Events::ModalClosed, OnModalClosed);

  AnimateIn();
}

//******************************************************************************
void SettingsMenu::UpdateTransform()
{
  Vec2 parentSize = GetParent()->GetSize();
  SetSize(Vec2(parentSize.x * 0.4f, parentSize.y));

  mBackground->SetSize(mSize);
  mBackground->SetColor(SettingsUi::BackgroundColor);
  Composite::UpdateTransform();
}

//******************************************************************************
void SettingsMenu::OnActivated(Event* e)
{
  mLauncher->mMainButton->SetEnabled(false);
  mLauncher->mSearch->SetVisible(false);

  // The config settings could've been changed (via a project loading for example)
  // so reload the current settings from the config
  LoadFromConfig();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void SettingsMenu::OnDeactivated(Event* e)
{
  ModalConfirmAction* modal = mConfirmModal;
  // If we had an active modal dialog then cancel it
  // (cancel so we revert the settings back to what they are supposed to be)
  if(modal)
    modal->Cancel();
}

//******************************************************************************
void SettingsMenu::OnProjectLocationTextSubmit(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);

  ZPrint("Settings: Changing project location from '%s' to '%s'\n",
    config->mDefaultProjectSaveLocation.c_str(), mDefaultProjectLocation->GetText().c_str());

  config->mDefaultProjectSaveLocation = mDefaultProjectLocation->GetText();
  SaveConfig();

  Event toSend;
  mLauncher->mConfigCog->DispatchEvent(Events::LauncherConfigChanged, &toSend);
}

//******************************************************************************
void SettingsMenu::OnBrowseProjectLocation(Event* e)
{
  //Set up the callback for when project file is selected
  const String CallBackEvent = "ProjectLocationCallback";
  if(!GetDispatcher()->IsConnected(CallBackEvent, this))
    ConnectThisTo(this, CallBackEvent, OnBrowseProjectLocationSelected);

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = CallBackEvent;
  config.CallbackObject = this;
  config.Title = "Select a folder";
  config.AddFilter("Zero Project Folder", "*.none");
  config.DefaultFileName = mDefaultProjectLocation->GetText();
  config.StartingDirectory = mDefaultProjectLocation->GetText();
  config.Flags |= FileDialogFlags::Folder;
  Z::gEngine->has(OsShell)->SaveFile(config);
}

//******************************************************************************
void SettingsMenu::OnBrowseProjectLocationSelected(OsFileSelection* e)
{
  if(e->Files.Size() > 0)
  {
    String path = BuildString(FilePath::GetDirectoryPath(e->Files[0]), cDirectorySeparatorCstr);
    mDefaultProjectLocation->SetText(path);
    OnProjectLocationTextSubmit(nullptr);
  }
}

//******************************************************************************
void SettingsMenu::OnDownloadLocationTextChanged(Event* e)
{
  // If there's an active tooltip then destroy it (the user is changing the text right now)
  ToolTip* toolTip = mToolTip;
  if(toolTip != nullptr)
    toolTip->Destroy();
}

//******************************************************************************
void SettingsMenu::OnDownloadLocationTextSubmit(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  String oldDownloadPath = config->mDownloadPath;
  String oldDownloadPathWithSeparator = BuildString(oldDownloadPath, cDirectorySeparatorCstr);

  // Since we're just moving the folder to the new location, we can't move to a sub-folder
  // of the current location, to detect this we're just seeing if the starting paths are the
  // same (by adding a trailing '\' right now, hopefully that's sufficient) and if they are
  // we're just showing a tooltip to the user
  String newPathWithSeparator = BuildString(FilePath::Normalize(mDownloadLocation->GetText()), cDirectorySeparatorCstr);
  if(newPathWithSeparator.StartsWith(oldDownloadPathWithSeparator))
  {
    mDownloadLocation->SetText(config->mDownloadPath);

    ToolTip* toolTip = new ToolTip(mDownloadLocation, "Cannot move to sub-directory");
    mToolTip = toolTip;
    // Don't have the tooltip destroyed when the mouse leaves the text box
    toolTip->SetDestroyOnMouseExit(false);
    // Queue up an action to destroy the tooltip after a little bit
    ActionSequence* seq = new ActionSequence(toolTip);
    seq->Add(new ActionDelay(5.0f));
    seq->Add(new CallAction<Widget, &Widget::Destroy>(toolTip));
    return;
  }

  // Make a confirmation dialog to see if the user whats to move
  // the downloads folder and contents to the new location
  ModalConfirmAction* modal = new ModalConfirmAction(mParentModal, "Move downloads to new location?");
  ConnectThisTo(modal, Events::ModalConfirmResult, OnChangeDownloadLocation);
  mConfirmModal = modal;
}

//******************************************************************************
void SettingsMenu::OnBrowseDownloadLocation(Event* e)
{
  //Set up the callback for when project file is selected
  const String CallBackEvent = "DownloadLocationCallback";
  if(!GetDispatcher()->IsConnected(CallBackEvent, this))
    ConnectThisTo(this, CallBackEvent, OnBrowseDownloadLocationSelected);

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = CallBackEvent;
  config.CallbackObject = this;
  config.Title = "Select a folder";
  config.AddFilter("Zero Project Folder", "*.none");
  config.StartingDirectory = mDownloadLocation->GetText();
  config.Flags |= FileDialogFlags::Folder;
  Z::gEngine->has(OsShell)->SaveFile(config);
}

//******************************************************************************
void SettingsMenu::OnBrowseDownloadLocationSelected(OsFileSelection* e)
{
  if(e->Files.Size() > 0)
  {
    String path = FilePath::GetDirectoryPath(e->Files[0]);
    mDownloadLocation->SetText(path);

    OnDownloadLocationTextSubmit(nullptr);
  }
}

//******************************************************************************
void SettingsMenu::OnChangeDownloadLocation(ModalConfirmEvent* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  if(e->mConfirmed == true)
  {
    // Grab the old download path and update to the new one
    String oldDownloadPath = config->mDownloadPath;
    config->mDownloadPath = mDownloadLocation->GetText();
    SaveConfig();

    ZPrint("Settings: Changing downloads from '%s' to '%s'\n", oldDownloadPath.c_str(), config->mDownloadPath.c_str());

    // Move the old folder to the new location
    MoveFolderContents(config->mDownloadPath, oldDownloadPath);
    // Currently Move doesn't delete the old directory, so delete it...
    if(oldDownloadPath != config->mDownloadPath)
      DeleteDirectory(oldDownloadPath);

    Event toSend;
    mLauncher->mConfigCog->DispatchEvent(Events::LauncherConfigChanged, &toSend);
  }
  else
  {
    // They hit cancel, set the download location text back to the config's settings
    mDownloadLocation->SetText(config->mDownloadPath);
  }
}

//******************************************************************************
void SettingsMenu::OnAutoRunModeSelected(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  config->mAutoRunMode = mAutoRunMode->GetSelectedItem();
  SaveConfig();
}

//******************************************************************************
void SettingsMenu::OnMaxRecentProjectsModified(Event* e)
{
  RecentProjects* recentProjects = mLauncher->mConfigCog->has(RecentProjects);

  uint maxRecentProjects;
  ToValue(mMaxNumberOfRecentProjects->GetText(), maxRecentProjects);
  // For now, ToValue doesn't return if the parsing failed, it just will return
  // the default value of zero. In that case just return revert back to the
  // config's settings (even if they typed zero) and ignore their changes.
  if(maxRecentProjects == 0)
    maxRecentProjects = recentProjects->mMaxRecentProjects;

  // We still need some logical extremes for the max projects...
  maxRecentProjects = Math::Clamp(maxRecentProjects, 1u, 50u);

  ZPrint("Settings: Changing max recent projects from %d to %d\n", recentProjects->mMaxRecentProjects, maxRecentProjects);

  // If we clamped the value then the text also has to be updated
  mMaxNumberOfRecentProjects->SetText(ToString(maxRecentProjects));
  // Make sure we remove projects if we now have too many
  recentProjects->UpdateMaxNumberOfProjects(maxRecentProjects, true);
  // Since we may have changed what projects were in recent make sure to save
  SaveConfig();
}

//******************************************************************************
void SettingsMenu::OnAutoCheckForMajorUpdatesModified(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  config->mAutoCheckForMajorUpdates = mAutoCheckForMajorUpdatesCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();
}

//******************************************************************************
void SettingsMenu::OnShowDevelopModified(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  config->mShowDevelopmentBuilds = mShowDevelopCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();

  // Tell everyone listening to the config that this changed (so the builds menu is updated)
  Event toSend;
  config->DispatchEvent(Events::ShowDevelopChanged, &toSend);
}

void SettingsMenu::OnShowExperimentalBranchesModified(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  config->mShowExperimentalBranches = mShowExperimentalBranchesCheckBox->GetChecked();

  // Save the config now with the new settings
  SaveConfig();

  // Tell everyone listening to the config that this changed (so the builds menu is updated)
  Event toSend;
  config->DispatchEvent(Events::ShowExperimentalBranchesChanged, &toSend);
}

//******************************************************************************
void SettingsMenu::OnCheckForUpdates(Event* e)
{
  ZPrint("Check for updates\n");
  Event toSend;
  mLauncher->DispatchEvent(Events::CheckForUpdates, &toSend);
}

//******************************************************************************
void SettingsMenu::OnCheckForLauncherUpdates(Event* e)
{
  ZPrint("Check for launcher updates\n");
  mLauncher->CheckForLauncherUpdates();
}

//******************************************************************************
void SettingsMenu::OnRevertToDefaults(Event* e)
{
  // Make a confirmation dialog to see if the user whats to move
  // the downloads folder and contents to the new location
  ModalConfirmAction* modal = new ModalConfirmAction(mParentModal, "Revert all settings to default?");
  ConnectThisTo(modal, Events::ModalConfirmResult, OnRevertToDefaultsConfirmation);
  mConfirmModal = modal;
}

//******************************************************************************
void SettingsMenu::OnRevertToDefaultsConfirmation(ModalConfirmEvent* e)
{
  if(e->mConfirmed == true)
    RevertConfigToDefaults();
}

//******************************************************************************
void SettingsMenu::LoadFromConfig()
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  RecentProjects* recentProjects = mLauncher->mConfigCog->has(RecentProjects);

  mDefaultProjectLocation->SetText(config->mDefaultProjectSaveLocation);
  mDownloadLocation->SetText(config->mDownloadPath);
  mAutoRunMode->SetSelectedItem(config->mAutoRunMode, false);

  mMaxNumberOfRecentProjects->SetText(ToString(recentProjects->mMaxRecentProjects));
  mShowDevelopCheckBox->SetCheckedDirect(config->mShowDevelopmentBuilds);
  mShowExperimentalBranchesCheckBox->SetCheckedDirect(config->mShowExperimentalBranches);
}

//******************************************************************************
void SettingsMenu::SaveConfig()
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  SaveLauncherConfig(config->GetOwner());
}

//******************************************************************************
void SettingsMenu::RevertConfigToDefaults()
{
  ZPrint("Settings: Revert to default\n");
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  // Grab the old location of the downloads
  String oldDownloadPath = config->mDownloadPath;

  // Revert the config to the default serialization values
  DefaultSerializer defaultStream;
  config->Serialize(defaultStream);
  // We can't default serialize the recent projects because then we'd lose all of
  // the recent projects, so for now the max recent projects default is hardcoded here as well.
  RecentProjects* recentProjects = mLauncher->mConfigCog->has(RecentProjects);
  recentProjects->mMaxRecentProjects = 20;

  // Save the new config
  SaveConfig();
  // Update all of the ui to display the current config values
  LoadFromConfig();

  // Move the old location of the downloads to the new one
  MoveFolderContents(config->mDownloadPath, oldDownloadPath);
  if(oldDownloadPath != config->mDownloadPath)
    DeleteDirectory(oldDownloadPath);
  
  Event toSend;
  mLauncher->mConfigCog->DispatchEvent(Events::LauncherConfigChanged, &toSend);
}

//******************************************************************************
void SettingsMenu::OnSettingsPressed(Event*)
{
  mParentModal->Close();
}

//******************************************************************************
void SettingsMenu::OnModalClosed(Event*)
{
  AnimateOut();
}

//******************************************************************************
void SettingsMenu::AnimateIn()
{
  ActionGroup* group = new ActionGroup(this);

  // Animate the background in
  group->Add(MoveWidgetAction(mBackground, Vec3::cZero, SettingsUi::SlideInTime));

  float delayPadding = 0.01f;

  // Animate each widget
  for(uint i = 0; i < mWidgetsToAnimate.Size(); ++i)
  {
    Widget* widget = mWidgetsToAnimate[i];

    float delay = delayPadding * float(i);
    ProxyAndAnimateIn(widget, Pixels(-500, 0, 0), 0.22f, 0.1f, delay);
  }
}

//******************************************************************************
void SettingsMenu::AnimateOut()
{
  ActionGroup* group = new ActionGroup(this);

  Vec3 destination = Pixels(-500, 0, 0);

  // Animate the background in
  group->Add(MoveWidgetAction(mBackground, destination, SettingsUi::SlideInTime));

  // Animate each widget
  forRange(Widget* widget, mWidgetsToAnimate.All())
    group->Add(MoveWidgetAction(widget, destination, SettingsUi::SlideInTime));
}

}//namespace Zero
