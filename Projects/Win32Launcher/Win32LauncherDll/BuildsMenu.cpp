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
namespace BuildsUi
{
  const cstr cLocation = "LauncherUi/BuildsWindow";
  Tweakable(Vec4, BuildNotesColor,       Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ReleaseNotes,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildVersion,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ReleaseDate,           Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, Tags,                  Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildHover,            Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildSelected,         Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildSelectedHover,    Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildBadSelected,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildBadSelectedHover, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildBackground0,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, BuildBackground1,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DownloadForeground,    Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, DownloadBackground,    Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------------------ Builds Item
//******************************************************************************
BuildItem::BuildItem(Composite* parent, ZeroBuild* version,
                     BuildsMenu* buildsMenu) :
  Composite(parent),
  mVersion(version),
  mBuildsMenu(buildsMenu)
{
  mVersionSelector = buildsMenu->mLauncher->mVersionSelector;

  mSelected = false;
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(Vec4::cZero);

  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero,
                              Thickness(Pixels(0,9,10,0))));//3
  SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(46));//34

  Composite* leftPadding = new Composite(this);
  leftPadding->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(12));

  Composite* leftHalf = new Composite(this);
  leftHalf->SetLayout(CreateStackLayout());
  leftHalf->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(144));
  {
    mBuildVersion = new Text(leftHalf, mLauncherRegularFont, 11);
    mBuildVersion->SetText(version->GetDisplayString());
    mBuildVersion->SetColor(BuildsUi::BuildVersion);
    mBuildVersion->mAlign = TextAlign::Left;

    mInstallState = new Text(leftHalf, mLauncherBoldFont, 10);
    mInstallState->mAlign = TextAlign::Left;

    UpdateInstallState();
  }

  Composite* spacer = new Composite(this);
  spacer->SetSizing(SizePolicy::Flex, Vec2(1));

  Composite* rightHalf = new Composite(this);
  rightHalf->SetLayout(CreateStackLayout());
  rightHalf->SetSizing(SizeAxis::X, SizePolicy::Flex, 1.0f);
  {
    mTags = new Text(rightHalf, mLauncherRegularFont, 11);

    String tagText = version->GetTagsString().ToUpper();
    tagText = tagText.Replace(",", " / ");

    mTags->SetText(tagText);
    mTags->mAlign = TextAlign::Right;
    mTags->SetColor(BuildsUi::Tags);

    String year, month, day;
    version->GetReleaseDate(year, month, day);
    
    mReleaseDate = new Text(rightHalf, mLauncherBoldFont, 10);
    mReleaseDate->SetText(String::Format("%s-%s-%s", month.c_str(), day.c_str(), year.c_str()));
    mReleaseDate->mAlign = TextAlign::Right;
    mReleaseDate->SetColor(BuildsUi::ReleaseDate);
  }

  mDownloadProgress = nullptr;

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::RightClick, OnRightClick);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  ConnectThisTo(version, Events::InstallStarted, OnInstallStarted);
  ConnectThisTo(version, Events::BackgroundTaskUpdated, OnDownloadUpdate);
  ConnectThisTo(version, Events::InstallCompleted, OnDownloadCompleted);
  ConnectThisTo(version, Events::UninstallCompleted, OnUninstallCompleted);
  ConnectThisTo(version, Events::BackgroundTaskFailed, OnDownloadFailed);
}

//******************************************************************************
void BuildItem::OnDestroy()
{
  DisconnectAll(this, this);
  DisconnectAll(mVersion, this);

  Composite::OnDestroy();
}

//******************************************************************************
void BuildItem::UpdateTransform()
{
  mBackground->SetSize(mSize);

  // Update the size of the progress bar
  if(mDownloadProgress)
  {
    float textWidth = mBuildVersion->GetMinSize().x;
    // Center the progress bar to be 24 pixels smaller than the build item
    mDownloadProgress->SetTranslation(Vec3(Pixels(12), Pixels(30), 0));
    mDownloadProgress->SetSize(Vec2(mSize.x - Pixels(24), Pixels(4)));
  }

  // When we're selected, we gotta change the color of some text to
  // make it stand out against the selected background color
  if(mSelected)
  {
    mBuildVersion->SetColor(BuildsUi::ReleaseDate);
    mInstallState->SetColor(Vec4(0, 0, 0, 0.5f));
    mTags->SetColor(BuildsUi::ReleaseDate);
    mReleaseDate->SetColor(BuildsUi::Tags);
  }
  else
  {
    UpdateInstallState();
    
    mBuildVersion->SetColor(BuildsUi::BuildVersion);
    mTags->SetColor(BuildsUi::Tags);
    mReleaseDate->SetColor(BuildsUi::ReleaseDate);
  }

  if(IsMouseOver())
  {
    if(mSelected)
    {
      if(mVersion->IsBad())
        mBackground->SetColor(BuildsUi::BuildBadSelectedHover);
      else
        mBackground->SetColor(BuildsUi::BuildSelectedHover);
    }
    else
    {
      mBackground->SetColor(BuildsUi::BuildHover);
    }
  }
  else
  {
    if(mSelected)
    {
      if(mVersion->IsBad())
        mBackground->SetColor(BuildsUi::BuildBadSelected);
      else
        mBackground->SetColor(BuildsUi::BuildSelected);
    }
    else
    {
      mBackground->SetColor(mBackgroundColor);
    }
  }
  
  Composite::UpdateTransform();
}

//******************************************************************************
void BuildItem::Install()
{
  if(mVersion->mInstallState != InstallState::NotInstalled)
    return;

  // Start the install
  BackgroundTask* task = mVersionSelector->InstallVersion(mVersion);
}

//******************************************************************************
void BuildItem::OnInstallStarted(Event* e)
{
  mBuildsMenu->UpdateInstallButton();

  // Display the progress bar
  mDownloadProgress = new ProgressBar(this);
  mDownloadProgress->SetNotInLayout(true);

  // Center the progress bar to be 24 pixels smaller than the build item
  mDownloadProgress->SetTranslation(Vec3(Pixels(12), Pixels(18), 0));
  mDownloadProgress->SetSize(Vec2(mSize.x - Pixels(24), Pixels(4)));

  // Remove the padding
  mDownloadProgress->mPadding = Thickness::cZero;
  mDownloadProgress->SetPrimaryColor(BuildsUi::DownloadForeground);
  mDownloadProgress->SetBackgroundColor(BuildsUi::DownloadBackground);

  // Don't want to display the percentage text, just the bar is enough
  mDownloadProgress->SetTextVisible(false);

  // Hide the text that the progress bar will be on top of
  mInstallState->SetVisible(false);
  mReleaseDate->SetVisible(false);
}

//******************************************************************************
void BuildItem::Uninstall()
{
  if(mVersion->mInstallState == InstallState::NotInstalled)
    return;

  String msg = "UNINSTALL BUILD";
  if(mVersion->mOnServer == false)
    msg = "UNINSTALL LOCAL BUILD";

  Modal* modal = new ModalConfirmAction(mBuildsMenu, msg);
  mBuildsMenu->mUninstallModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnUninstallModalResult);
  modal->TakeFocus();
  mBuildsMenu->mLauncher->mActiveModal = modal;
}

//******************************************************************************
void BuildItem::UninstallBuildInternal()
{
  mVersionSelector->DeleteVersion(mVersion);
  UpdateInstallState();
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildItem::OnUninstallModalResult(ModalConfirmEvent* e)
{
  if(e->mConfirmed)
  {
    // If there are copies of the build currently running then we can't close and 
    // we have to prompt the user on what action they want to take
    if(mVersionSelector->CheckForRunningBuild(mVersion) == true)
    {
      CreateBuildsRunningModal();
      return;
    }

    // Otherwise we can actually uninstall the build
    UninstallBuildInternal();
  }
}

//******************************************************************************
void BuildItem::CreateBuildsRunningModal()
{
  // Warn the user that the build is currently running
  String msg = String::Format("Build %s is currently running", mVersion->GetDisplayString().c_str());
  // Create the buttons that we will display and listen for responses from
  Array<String> buttons;
  buttons.PushBack("RETRY");
  buttons.PushBack("FORCE CLOSE");
  buttons.PushBack("CANCEL");
  // Create the modal and connect to whenever a button is pressed
  ModalButtonsAction* modal = new ModalButtonsAction(mBuildsMenu, msg.ToUpper(), buttons);
  ConnectThisTo(modal, Events::ModalButtonPressed, OnBuildRunningModalResult);
  modal->TakeFocus();
  // Make sure to set what the active modal is on the launcher window (so escape can cancel)
  mBuildsMenu->mLauncher->mActiveModal = modal;
}

//******************************************************************************
void BuildItem::OnBuildRunningModalResult(ModalButtonEvent* e)
{
  // The user wants to retry (they are manually closing their copies of zero)
  if(e->mButtonName == "RETRY")
  {
    // If there are any copies of the build still running then create the modal again
    if(mVersionSelector->CheckForRunningBuild(mVersion) == true)
    {
      CreateBuildsRunningModal();
      return;
    }

    // Otherwise we can now safely uninstall
    UninstallBuildInternal();
  }
  // The user wants us to close all copies of zero that are open with that build
  else if(e->mButtonName == "FORCE CLOSE")
  {
    mVersionSelector->ForceCloseRunningBuilds(mVersion);
    UninstallBuildInternal();
  }
  // The user chanced their mind and doesn't want to uninstall
  else if(e->mButtonName == "CANCEL")
  {
    return;
  }
}

//******************************************************************************
void BuildItem::OnDownloadUpdate(BackgroundTaskEvent* e)
{
  // Update the download progress on the progress bar
  if(mDownloadProgress)
    mDownloadProgress->SetPercentage(e->PercentComplete * 0.95f);
}

//******************************************************************************
void BuildItem::OnDownloadCompleted(BackgroundTaskEvent* e)
{
  // Destroy the progress bar
  if(mDownloadProgress != nullptr)
    mDownloadProgress->Destroy();
  mDownloadProgress = nullptr;

  // Make the hidden text visible again
  mInstallState->SetVisible(true);
  mReleaseDate->SetVisible(true);

  // We're now installed, so update the state
  UpdateInstallState();
  // Also update the button text, active state, etc...
  mBuildsMenu->UpdateInstallButton();
}

//******************************************************************************
void BuildItem::OnUninstallCompleted(BackgroundTaskEvent* e)
{
  OnDownloadCompleted(e);
  if(mVersion->mOnServer == false)
    mBuildsMenu->CreateBuildItems();
}

//******************************************************************************
void BuildItem::OnDownloadFailed(BackgroundTaskEvent* e)
{
  ZPrint("Failed to download build %s\n", mVersion->GetDebugIdString().c_str());

  // Destroy the progress bar
  mDownloadProgress->Destroy();
  mDownloadProgress = nullptr;

  // Make the hidden text visible again
  mInstallState->SetVisible(true);
  mTags->SetVisible(true);

  // Should notify that it failed to download
  // ...
}

//******************************************************************************
void BuildItem::OnMouseEnter(MouseEvent* e)
{
  MarkAsNeedsUpdate();

  // Create a tooltip if the build was deprecated and we don't already have one
  if(mToolTip == nullptr && mVersion->GetDeprecatedInfo(false) != nullptr)
  {
    ToolTip* toolTip = new ToolTip(this);

    // Position the tool tip centered on the right of the build item
    Vec3 translation = GetScreenPosition();
    Vec2 size = GetSize();
    translation.x += size.x;
    translation.y += size.y / 2;
    
    toolTip->SetArrowTipTranslation(translation);
    toolTip->SetText(mVersion->GetDeprecatedString());
    mToolTip = toolTip;
  }
}

//******************************************************************************
void BuildItem::OnLeftClick(MouseEvent* e)
{
  ObjectEvent eventToSend(this);
  GetDispatcher()->Dispatch(Events::ButtonPressed, &eventToSend);
  SetSelected(true);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildItem::OnRightClick(MouseEvent* e)
{

}

//******************************************************************************
void BuildItem::OnMouseExit(MouseEvent* e)
{
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildItem::SetSelected(bool state)
{
  mSelected = state;
  MarkAsNeedsUpdate();
}

//******************************************************************************
void BuildItem::UpdateInstallState()
{
  if(mVersion->mInstallState == InstallState::Installed)
  {
    mInstallState->SetText("INSTALLED");
    mInstallState->SetColor(BuildColors::Installed);
  }
  else if(mVersion->IsBad())
  {
    mInstallState->SetText("DEPRECATED");
    mInstallState->SetColor(BuildColors::Deprecated);
  }
  else
  {
    mInstallState->SetText("NOT INSTALLED");
    mInstallState->SetColor(BuildColors::NotInstalled);
  }
}

//---------------------------------------------------------------- Release Notes
//******************************************************************************
ReleaseNotes::ReleaseNotes(Composite* parent) : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(5,5), Thickness::cZero));

  Composite* headerText = new Composite(this);
  headerText->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(5,0), Thickness(Pixels(0, 0, 8, 0))));
  {
    Composite* topRow = new Composite(headerText);
    topRow->SetLayout(CreateRowLayout());
    {
      new Spacer(topRow, SizePolicy::Flex, Vec2(1));

      mDownloadButton = new IconButton(topRow);
      mDownloadButton->SetIcon("DownloadIcon");
      mDownloadButton->mBackground->SetVisible(false);
      mDownloadButton->mBorder->SetVisible(false);
      mDownloadButton->SizeToContents();
      mDownloadButton->mIconColor = ToByteColor(Vec4(0.9f, 0.9f, 0.9f, 1));
      mDownloadButton->mIconHoverColor = Color::White;
      mDownloadButton->mIconClickedColor = ToByteColor(Vec4(0.7f, 0.7f, 0.7f, 1));
      mDownloadButton->SetToolTip("Install Build");

      mUninstallButton = new IconButton(topRow);
      mUninstallButton->SetIcon("XIcon");
      mUninstallButton->mBackground->SetVisible(false);
      mUninstallButton->mBorder->SetVisible(false);
      mUninstallButton->SizeToContents();
      mUninstallButton->mIconColor = ToByteColor(Vec4(0.9f, 0.9f, 0.9f, 1));
      mUninstallButton->mIconHoverColor = Color::White;
      mUninstallButton->mIconClickedColor = ToByteColor(Vec4(0.7f, 0.7f, 0.7f, 1));
      mUninstallButton->SetToolTip("Uninstall Build");

      new Spacer(topRow, SizePolicy::Fixed, Pixels(10, 0));

      mBuildVersion = new Text(topRow, mLauncherRegularFont, 36);
      mBuildVersion->SetText("BUILD 9001");
      mBuildVersion->mAlign = TextAlign::Right;
      mBuildVersion->SetColor(BuildsUi::BuildNotesColor);
    }

    new Spacer(headerText, SizePolicy::Fixed, Pixels(0, -3));

    mNote = new Text(headerText, mLauncherRegularFont, 18);
    mNote->SetText("MINOR REVISION / BUG FIXES");
    mNote->mAlign = TextAlign::Right;
    mNote->SetColor(BuildsUi::ReleaseNotes);

    new Spacer(headerText, SizePolicy::Fixed, Pixels(0, 5));

    mTitle = new Text(headerText, mLauncherRegularFont, 12);
    mTitle->SetText("RELEASE NOTES");
    mTitle->mAlign = TextAlign::Right;
    mTitle->SetColor(BuildsUi::ReleaseNotes);
  }
  headerText->SizeToContents();

  // Text editor for the release notes
  mReleaseNotes = new TextEditor(this);
  mReleaseNotes->SetReadOnly(true);
  mReleaseNotes->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  mReleaseNotes->mHotspots.PushBack(new HyperLinkHotspot());

  // We want the background of the text editor to be completely transparent
  ColorScheme localScheme;
  localScheme.Default = FloatColorRGBA(100,100,100,255);
  localScheme.Background = FloatColorRGBA(100,100,100,0);
  localScheme.Gutter = FloatColorRGBA(100,100,100,0);
  mReleaseNotes->SetColorScheme(localScheme);

  mSelectedBuild = nullptr;
}

//******************************************************************************
void ReleaseNotes::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
void ReleaseNotes::DisplayReleaseNotes(ZeroBuild* build)
{
  if(mSelectedBuild)
    mSelectedBuild->GetDispatcher()->Disconnect(this);

  ConnectThisTo(build, Events::InstallStarted, OnSelectedBuildChanged);
  ConnectThisTo(build, Events::InstallCompleted, OnSelectedBuildChanged);
  ConnectThisTo(build, Events::UninstallStarted, OnSelectedBuildChanged);
  ConnectThisTo(build, Events::UninstallCompleted, OnSelectedBuildChanged);
  mSelectedBuild = build;

  // Set the build text
  mBuildVersion->SetText(build->GetDisplayString());

  // Tags
  mNote->SetText(build->GetTagsString());

  // Release Notes
  mReleaseNotes->SetReadOnly(false);
  String releaseNotes = build->GetReleaseNotes();
  mReleaseNotes->SetAllText(releaseNotes);
  TextEditorHotspot::MarkHotspots(mReleaseNotes);
  mReleaseNotes->SetReadOnly(true);
  // At the moment, scroll bars only grow, never shrink. To prevent showing a scrollbar
  // when there's no text (which looks odd) for now just toggle visibility on the release
  // notes. Need to figure out what's wrong with scintilla later...
  mReleaseNotes->SetVisible(!releaseNotes.Empty());

  UpdateBuildButtons();
}

//******************************************************************************
void ReleaseNotes::OnSelectedBuildChanged(Event*)
{
  UpdateBuildButtons();
}

//******************************************************************************
void ReleaseNotes::UpdateBuildButtons()
{
  mUninstallButton->SetActive(false);
  mDownloadButton->SetActive(false);

  if(mSelectedBuild->mInstallState == InstallState::NotInstalled)
    mDownloadButton->SetActive(true);

  if(mSelectedBuild->mInstallState == InstallState::Installed)
    mUninstallButton->SetActive(true);
}

//------------------------------------------------------------------ Builds Menu
//******************************************************************************
BuildsMenu::BuildsMenu(Composite* parent, LauncherWindow* launcher)
  : Composite(parent), mLauncher(launcher)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(10, 0), Thickness(Pixels(0,5,24,0))));
  SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);

  mBuildList = new ScrollArea(this);
  mBuildList->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(400));
  mBuildList->GetClientWidget()->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 0), Thickness(Pixels(32,0,0,0))));
  mBuildList->mScrollSpeedScalar = 0.5f;

  Splitter* splitter = new Splitter(this);
  splitter->mAxis = SizeAxis::X;

  mReleaseNotes = new ReleaseNotes(this);
  mReleaseNotes->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

  ConnectThisTo(mReleaseNotes->mDownloadButton, Events::ButtonPressed, OnInstallPressed);
  ConnectThisTo(mReleaseNotes->mUninstallButton, Events::ButtonPressed, OnUninstallPressed);

  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  ConnectThisTo(versionSelector, Events::VersionListLoaded, OnVersionListLoaded);
  ConnectThisTo(versionSelector, Events::BuildListUpdated, OnBuildListUpdated);
  ConnectThisTo(this, Events::MenuDisplayed, OnMenuDisplayed);
  ConnectThisTo(this, Events::MenuHidden, OnMenuHidden);
  
  // Listen for when the user tags change on the config (most likely from the settings menu)
  ConnectThisTo(mLauncher->mConfigCog, Events::ShowDevelopChanged, OnShowDevelopChanged);
  ConnectThisTo(mLauncher->mConfigCog, Events::ShowExperimentalBranchesChanged, OnShowDevelopChanged);

  // Create the initial build list (at this point it should be the locally downloaded ones)
  CreateBuildItems();
}

//******************************************************************************
void BuildsMenu::UpdateTransform()
{
  // Update the client size of the scroll area
  Widget* clientWidget = mBuildList->GetClientWidget();
  clientWidget->mSize.x = mBuildList->mSize.x - mBuildList->GetScrollBarSize() - Pixels(10);

  Composite::UpdateTransform();
}

//******************************************************************************
void BuildsMenu::CreateBuildItems()
{
  // Delete all old build items
  for(size_t i = 0; i < mBuildItems.Size(); ++i)
    mBuildItems[i]->Destroy();
  mBuildItems.Clear();

  LauncherConfig* launcherConfig = mLauncher->mConfigCog->has(LauncherConfig);

  // Create UI to represent each build on the build server
  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  Array<ZeroBuild*> results;
  TagList tagResults, rejections;
  if(launcherConfig->mShowDevelopmentBuilds == false)
    rejections.Insert("Develop");

  // Check to see if we display legacy builds
  LauncherLegacySettings* legacySettings = launcherConfig->mOwner->has(LauncherLegacySettings);
  if(legacySettings == nullptr || !legacySettings->mDisplayLegacyBuilds)
    rejections.Insert("Legacy");

  // Get all builds with the current search filters
  String searchText = mLauncher->mSearch->GetText();

  ZeroBuildTagPolicy policy(launcherConfig);
  HashSet<String> legacyTags;
  FilterDataSetWithTags(legacyTags, rejections, searchText, versionSelector->mVersions, results, tagResults, policy);

  uint index = 0;
  String currentPlatform = BuildId::GetCurrentLauncherId().mPlatform;
  bool displayOnlyPreferredPlatform = mLauncher->mVersionSelector->mConfig->mDisplayOnlyPreferredPlatform;
  forRange(ZeroBuild* version, results.All())
  {
    BuildItem* item = new BuildItem(mBuildList, version, this);
    ConnectThisTo(item, Events::ButtonPressed, OnBuildSelected);
    mBuildItems.PushBack(item);

    if(index % 2)
      item->mBackgroundColor = BuildsUi::BuildBackground0;
    else
      item->mBackgroundColor = BuildsUi::BuildBackground1;
    ++index;
  }

  // Select the latest build by default
  if(!mBuildItems.Empty())
  {
    BuildItem* latestBuild = mBuildItems.Front();
    SelectBuild(latestBuild);
  }

  // Update the size of the scroll area client widget
  Vec2 clientSize = mBuildList->GetClientWidget()->GetMinSize();
  mBuildList->SetClientSize(clientSize);
}

//******************************************************************************
void BuildsMenu::OnVersionListLoaded(Event* e)
{
  ZPrint("Loading builds list\n");
  CreateBuildItems();
}

//******************************************************************************
void BuildsMenu::OnBuildListUpdated(Event* e)
{
  CreateBuildItems();
}

//******************************************************************************
void BuildsMenu::OnShowDevelopChanged(Event* e)
{
  CreateBuildItems();
}

//******************************************************************************
void BuildsMenu::OnBuildSelected(ObjectEvent* e)
{
  BuildItem* toSelect = (BuildItem*)e->Source;
  SelectBuild(toSelect);
}

//******************************************************************************
void BuildsMenu::SelectBuild(BuildItem* toSelect)
{
  // Display the release notes for this build
  mReleaseNotes->DisplayReleaseNotes(toSelect->mVersion);

  // De-select the previously selected build
  if(BuildItem* selected = mSelectedBuild)
    selected->SetSelected(false);

  // Select the new build
  toSelect->SetSelected(true);

  mSelectedBuild = toSelect;

  // Update the button to reflect the currently selected build
  UpdateInstallButton();
}

//******************************************************************************
void BuildsMenu::OnMenuDisplayed(Event* e)
{
  ZPrint("Builds menu displayed\n");

  // Other menu's will set this to invisible, and because it's fading
  // out through actions, we're already activated before it gets deactivated
  mLauncher->mMainButton->SetVisible(true);

  mLauncher->mSearch->SetVisible(true);

  // We're using the main button as an install button
  MainButton* button = mLauncher->mMainButton;
  button->SetText("INSTALL");

  // Update the button to reflect the currently selected build
  UpdateInstallButton();

  // When it's pressed, we want to install
  ConnectThisTo(button, Events::ButtonPressed, OnInstallPressed);
  ConnectThisTo(mLauncher->mSearch, Events::TextChanged, OnSearchChanged);
}

//******************************************************************************
void BuildsMenu::OnMenuHidden(Event* e)
{
  // No longer want to listen to button presses on the main button
  mLauncher->mMainButton->GetDispatcher()->Disconnect(this);
  mLauncher->mSearch->GetDispatcher()->Disconnect(this);

  mUninstallModal.SafeDestroy();
}

//******************************************************************************
void BuildsMenu::OnInstallPressed(Event* e)
{
  // Install the currently selected build
  if(BuildItem* buildItem = mSelectedBuild)
  {
    buildItem->Install();

    // Now that we're installing it, we no longer want the install button
    // to be click able
    UpdateInstallButton();
  }
}

//******************************************************************************
void BuildsMenu::OnUninstallPressed(Event* e)
{
  // Install the currently selected build
  if(BuildItem* buildItem = mSelectedBuild)
  {
    buildItem->Uninstall();

    // Now that we're installing it, we no longer want the install button
    // to be click able
    UpdateInstallButton();
  }
}

//******************************************************************************
void BuildsMenu::OnSearchChanged(Event* e)
{
  CreateBuildItems();
}

//******************************************************************************
void BuildsMenu::UpdateInstallButton()
{
  // If this page isn't currently active then don't update the install button
  if(mActive == false)
    return;

  // If the build can be installed, enable the install button
  MainButton* button = mLauncher->mMainButton;
  BuildItem* selectedBuild = mSelectedBuild;
  if(selectedBuild == nullptr)
  {
    button->SetEnabled(false);
    return;
  }

  ZeroBuild* version = selectedBuild->mVersion;
  button->SetVersionAndProject(version, nullptr);
}

}//namespace Zero
