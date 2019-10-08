// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace ActiveProjectUi
{
const cstr cLocation = "LauncherUi/ActiveProject";
Tweakable(Vec4, TextColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, BackgroundColor, Vec4(1, 1, 1, 1), cLocation);
} // namespace ActiveProjectUi

Modal* ActiveProjectMenu::OpenProject(Composite* modalParent, CachedProject* cachedProject, LauncherWindow* launcher)
{
  ModalStrip* modal = new ModalStrip(modalParent, 0.3f);
  modal->SetStripHeight(ModalSizeMode::Percentage, 0.789f);
  modal->SetStripColor(ActiveProjectUi::BackgroundColor);
  modal->mStripArea->SetLayout(CreateFillLayout());

  ActiveProjectMenu* menu = new ActiveProjectMenu(modal, launcher);
  menu->SelectProject(cachedProject);

  return modal;
}

ActiveProjectMenu::ActiveProjectMenu(Composite* parent, LauncherWindow* launcher) :
    Composite(parent),
    mLauncher(launcher)
{
  mActiveSizeTask = nullptr;

  launcher->mSearch->SetVisible(false);

  Thickness padding(Pixels(32, 14, 32, 14));
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, padding));

  Composite* textArea = new Composite(this);
  textArea->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(0, 5), Thickness::cZero));
  textArea->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(429));
  {
    // Create an area to store the back button and project name
    Composite* topArea = new Composite(textArea);
    topArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(0, 0), Thickness::cZero));

    // Add the back button (to close the modal)
    mBackButton = new IconButton(topArea);
    mBackButton->SetIcon("BackIcon");
    mBackButton->mBackground->SetVisible(false);
    mBackButton->mBorder->SetVisible(false);
    mBackButton->mIconColor = ToByteColor(Vec4(0.9f, 0.9f, 0.9f, 1));
    mBackButton->mIconHoverColor = Color::White;
    mBackButton->mIconClickedColor = ToByteColor(Vec4(0.7f, 0.7f, 0.7f, 1));
    mBackButton->SizeToContents();
    ConnectThisTo(mBackButton, Events::ButtonPressed, OnBackButtonPressed);

    // Spacing between project name and back button
    new Spacer(topArea, SizePolicy::Fixed, Pixels(6, 0));

    Composite* projectNameArea = new Composite(topArea);
    projectNameArea->SetLayout(CreateStackLayout(LayoutDirection::RightToLeft, Pixels(0, 0), Thickness::cZero));
    projectNameArea->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);

    mEditProjectNameIconButton = new IconButton(projectNameArea);
    mEditProjectNameIconButton->SetIcon("EditScript");
    mEditProjectNameIconButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, 32);
    mEditProjectNameIconButton->mBackground->SetVisible(false);
    mEditProjectNameIconButton->mBorder->SetVisible(false);
    mEditProjectNameIconButton->mIconColor = ToByteColor(Vec4(0.8, 0.8, 0.8, 1));
    mEditProjectNameIconButton->mIconHoverColor = Color::White;
    mEditProjectNameIconButton->mIconClickedColor = ToByteColor(Vec4(0.6, 0.6, 0.6, 1));
    mEditProjectNameIconButton->SetToolTip("Rename Project");
    mEditProjectNameIconButton->SizeToContents();
    ConnectThisTo(mEditProjectNameIconButton, Events::ButtonPressed, OnEditProjectName);

    mProjectName = new TextBox(projectNameArea, "ActiveProjectName");
    mProjectName->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(300));
    // Text box doesn't properly take the font definition from the provided
    // style, forcibly override the font for now...
    mProjectName->mEditTextField->mFont = FontManager::GetInstance()->GetRenderFont(mLauncherRegularFont, 24, 0);
    mProjectName->SetTextClipping(true);
    mProjectName->HideBackground(true);
    mProjectName->SetColor(ActiveProjectUi::TextColor);
    ConnectThisTo(parent, Events::KeyDown, OnKeyDown);
    // Listen for the text changing on the project name so the user can rename
    // their project
    ConnectThisTo(mProjectName, Events::TextSubmit, OnProjectNameTextSubmit);
    ConnectThisTo(mProjectName, Events::FocusLostHierarchy, OnProjectNameFocusLost);

    Element* e = textArea->CreateAttached<Element>(cWhiteSquare);
    e->SetNotInLayout(false);
    e->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    e->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(1));
    e->SetColor(ActiveProjectUi::TextColor);

    mDate = new Text(textArea, mLauncherRegularFont, 12);
    mDate->SetColor(ActiveProjectUi::TextColor);

    mSize = new Text(textArea, mLauncherRegularFont, 12);
    mSize->SetColor(ActiveProjectUi::TextColor);

    mTags = new Text(textArea, mLauncherRegularFont, 12);
    mTags->SetColor(ActiveProjectUi::TextColor);

    Composite* folderRow = new Composite(textArea);
    folderRow->SetLayout(CreateRowLayout());
    {
      mLocation = new Text(folderRow, mLauncherRegularFont, 12);
      mLocation->SetColor(ActiveProjectUi::TextColor);
      mLocation->mVerticalAlignment = VerticalAlignment::Center;

      new Spacer(folderRow, SizePolicy::Fixed, Pixels(5, 0));

      IconButton* showFolder = new IconButton(folderRow);
      showFolder->SetIcon("OpenFolderIcon");
      showFolder->SetToolTip("Open Folder");

      // Icon colors
      showFolder->mIconColor = ToByteColor(Vec4(0.8, 0.8, 0.8, 1));
      showFolder->mIconHoverColor = Color::White;
      showFolder->mIconClickedColor = ToByteColor(Vec4(0.6, 0.6, 0.6, 1));

      // Background colors
      showFolder->mBackground->SetVisible(false);
      showFolder->mBorder->SetVisible(false);

      showFolder->MarkAsNeedsUpdate();
      ConnectThisTo(showFolder, Events::ButtonPressed, OnShowProjectFolder);
    }

    // Push the build selector to the bottom
    new Spacer(textArea, SizePolicy::Flex, Vec2(1));

    // Used to push the build selector to the right
    Composite* buildRow = new Composite(textArea);
    buildRow->SetLayout(CreateRowLayout());
    {
      mBackupProjectButton = new TextButton(buildRow);
      mBackupProjectButton->SetStyle(TextButtonStyle::Modern);
      mBackupProjectButton->SetText("Backup Project");
      mBackupProjectButton->mTextColor = Vec4(0.5f, 0.5f, 0.5f, 1);
      mBackupProjectButton->mTextHoverColor = Vec4(0.660000026f, 0.660000026f, 0.660000026f, 1);
      mBackupProjectButton->mTextClickedColor = Vec4(0.330000013f, 0.330000013f, 0.330000013f, 1);
      ConnectThisTo(mBackupProjectButton, Events::ButtonPressed, OnBackupProject);

      new Spacer(buildRow, SizePolicy::Flex, Vec2(1));

      // If we are to run with the debugger (zero is currently in visual studio
      // with the debugger attached) then add an extra button to communicate
      // with the open zero
      if (mLauncher->GetConfig()->mRunDebuggerMode)
      {
        mRunWithDebugger = new TextButton(buildRow);
        mRunWithDebugger->SetText("Run In Active Debugger");
        ConnectThisTo(mRunWithDebugger, Events::ButtonPressed, OnRunWithDebugger);
      }

      // Download button
      mInstallBuild = new IconButton(buildRow);
      mInstallBuild->SetIcon("DownloadIcon");
      mInstallBuild->mBackground->SetVisible(false);
      mInstallBuild->mBorder->SetVisible(false);
      mInstallBuild->SizeToContents();
      mInstallBuild->mIconColor = ToByteColor(Vec4(0.7f, 0.7f, 0.7f, 1));
      mInstallBuild->mIconHoverColor = ToByteColor(Vec4(0.9f, 0.9f, 0.9f, 1));
      mInstallBuild->mIconClickedColor = ToByteColor(Vec4(0.6f, 0.6f, 0.6f, 1));
      mInstallBuild->SetToolTip("Install Build");
      ConnectThisTo(mInstallBuild, Events::ButtonPressed, OnInstallPressed);

      // It will be activated based on the selected project
      mInstallBuild->SetVisible(false);

      // Spacing
      new Spacer(buildRow, SizePolicy::Fixed, Pixels(5, 0));

      // Build Selector
      mBuildSelector = new BuildSelector(buildRow, mLauncher->mVersionSelector);
      ConnectThisTo(mBuildSelector, Events::BuildSelected, OnBuildStateChanged);
    }
  }

  // Slide the text in
  ProxyAndAnimateIn(textArea, Pixels(-500, 0, 0), 0.25f, 0.1f, 0.045f);

  new Spacer(this, SizePolicy::Fixed, Pixels(58, 1));

  Composite* imageParent = new Composite(this);
  imageParent->mName = "ImageParent";
  // Keep the screenshot's ratio, and push it to the right to maintain a
  // constant distance between the screenshot and the edge of the window
  imageParent->SetLayout(CreateRatioLayout());
  imageParent->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
  imageParent->SetSizing(SizeAxis::Y, SizePolicy::Flex, 1);
  {
    // Default to no screenshot available
    mNoScreenshotImage = new NoScreenshotAvailable(imageParent);
    mNoScreenshotImage->SetSize(Pixels(164, 92));
    mNoScreenshotImage->mHorizontalAlignment = HorizontalAlignment::Right;

    mProjectImage = new TextureView(imageParent);
    mProjectImage->SetActive(false);
    mProjectImage->SetSize(Pixels(164, 92));
    mProjectImage->mHorizontalAlignment = HorizontalAlignment::Right;
  }

  // Slide the image in
  ProxyAndAnimateIn(imageParent, Pixels(-500, 0, 0), 0.25f, 0.1f, 0);

  // Other menu's will set this to invisible, and because it's fading
  // out through actions, we're already activated before it gets deactivated
  MainButton* button = mLauncher->mMainButton;
  button->SetVisible(true);
  button->SetText("LAUNCH");
  button->SetSubText("PROJECT");

  ConnectThisTo(button, Events::ButtonPressed, OnLaunchProject);
  // Listen for when the build selector changes the state of the current build
  // (it's listening to whichever build we care about)
  ConnectThisTo(mBuildSelector->mCurrentBuild, Events::BuildStateChanged, OnBuildStateChanged);
  ConnectThisTo(parent, Events::ModalClosed, OnModalClosed);
  ConnectThisTo(mLauncher->mVersionSelector, Events::VersionListLoaded, OnVersionListLoaded);
}

void ActiveProjectMenu::UpdateTransform()
{
  SetSizeXToRemainder(mProjectName);
  SetSizeXToRemainder(mLocation);
  Composite::UpdateTransform();
}

void ActiveProjectMenu::SetSizeXToRemainder(Widget* widgetToSize)
{
  Composite* parent = widgetToSize->GetParent();
  float remainingSize = parent->GetSize().x;
  AutoDeclare(range, parent->GetChildren());
  for (; !range.Empty(); range.PopFront())
  {
    Widget* widget = &range.Front();
    if (widget == widgetToSize)
      continue;

    remainingSize -= widget->GetSize().x;
  }

  widgetToSize->SetSizing(SizeAxis::X, SizePolicy::Fixed, remainingSize);
}

void ActiveProjectMenu::OnModalClosed(Event* e)
{
  // Ideally we should have already checked to see if the user was editing
  // the text field to determine whether or not to close but this requires
  // sending out an event before closing

  // Re-Enable the selected menu
  MenuData* data = mLauncher->mSelectedMenu;

  // When shutting down cancel the directory size task and stop listening to
  // events on it
  if (mActiveSizeTask != nullptr)
  {
    mActiveSizeTask->Cancel();
    DisconnectAll(mActiveSizeTask, this);
  }

  if (!data->mClientArea->mDestroyed)
  {
    Event eventToSend;
    data->mClientArea->DispatchEvent(Events::MenuDisplayed, &eventToSend);
  }

  // Destruction doesn't seem to disconnect events, so manually disconnect from
  // some
  DisconnectAll(mBuildSelector->mCurrentBuild, this);
  DisconnectAll(mLauncher->mVersionSelector, this);
  // This one is the most important as we can receive this event callback later
  // whenever the thread finishes loading a screenshot. This occasionally
  // happens in-between when we are destroyed and when we are deleted.
  DisconnectAll(mCachedProject, this);
}

void ActiveProjectMenu::UpdateScreenshot()
{
  Texture* texture = mCachedProject->mScreenshotTexture;
  // If there is a texture then de-activate the no-screenshot image and set the
  // texture as active
  if (texture != nullptr)
  {
    mProjectImage->SetTexture(mCachedProject->mScreenshotTexture);
    mProjectImage->SetActive(true);
    mNoScreenshotImage->SetActive(false);

    float aspect = mProjectImage->mSize.x / mProjectImage->mSize.y;
    mProjectImage->ClipUvToAspectRatio(aspect);
  }
  // Otherwise switch back to the no-screenshot image
  else
  {
    mProjectImage->SetActive(false);
    mNoScreenshotImage->SetActive(true);
  }
}

void ActiveProjectMenu::OnScreenshotUpdated(Event* e)
{
  UpdateScreenshot();
}

void ActiveProjectMenu::SelectProject(CachedProject* cachedProject)
{
  mCachedProject = cachedProject;
  // The project file could've changed out from underneath us (version control
  // for example)
  bool successfulReload = mLauncher->mProjectCache->ReloadProjectFile(mCachedProject, false);
  // The project was deleted out from under us so close the modal.
  if (!successfulReload)
  {
    Modal* modal = mLauncher->mActiveProjectModal;
    if (modal != nullptr)
      modal->Close();
    return;
  }

  String projectPath = mCachedProject->GetProjectPath();
  mProjectName->SetText(mCachedProject->GetProjectName());

  StringRange location = FilePath::GetDirectoryPath(projectPath);
  mLocation->SetText(location);

  CalendarDateTime modifiedTime;
  GetFileDateTime(projectPath, modifiedTime);
  mDate->SetText(String::Format("MODIFIED %d-%d-%d", modifiedTime.Month, modifiedTime.Day, modifiedTime.Year));

  // Update the tag text
  mTags->SetText(mCachedProject->GetTagsDisplayString());

  // If there was already a task to compute the project size then cancel it
  if (mActiveSizeTask != nullptr && mActiveSizeTask->IsCompleted() == false)
    mActiveSizeTask->Cancel();

  // Create a job to compute the size of the project folder
  DirectorySizeJob* job = new DirectorySizeJob(mCachedProject->GetProjectFolder());
  mActiveSizeTask = Z::gBackgroundTasks->CreateTask(job);
  mActiveSizeTask->mName = "ComputeSize";
  ConnectThisTo(mActiveSizeTask, Events::BackgroundTaskUpdated, OnSizeTaskUpdate);
  ConnectThisTo(mActiveSizeTask, Events::BackgroundTaskCompleted, OnSizeTaskUpdate);
  mActiveSizeTask->Execute();

  ZeroBuild* build = mLauncher->mVersionSelector->FindExactVersion(mCachedProject);

  mLauncher->mMainButton->SetEnabled(false);
  mInstallBuild->SetVisible(false);

  // The only time the build should be null is for someone with a build not on
  // the website
  if (build)
  {
    mBuildSelector->SetBuild(build);
    if (build->mInstallState == InstallState::Installed)
      mLauncher->mMainButton->SetEnabled(true);

    // Only show the install button if the build can be installed
    if (build->mInstallState == InstallState::NotInstalled && build->mOnServer)
      mInstallBuild->SetVisible(true);
  }
  else
    mBuildSelector->SetBuild(mCachedProject->GetBuildId());

  mProjectImage->SetActive(false);
  mNoScreenshotImage->SetActive(true);

  // Listen for whenever the screenshot changes for this cached project
  ConnectThisTo(mCachedProject, Events::ScreenshotUpdated, OnScreenshotUpdated);
  // Update the texture (if it is newer we'll get the ScreenshotUpdated event
  // called
  mCachedProject->UpdateTexture();
  // Update the current screenshot
  UpdateScreenshot();
}

void ActiveProjectMenu::OnBackButtonPressed(Event* e)
{
  Modal* modal = mLauncher->mActiveProjectModal;
  if (modal != nullptr)
    modal->Close();
}

void ActiveProjectMenu::OnKeyDown(KeyboardEvent* e)
{
  // If F2 is hit then have the project take focus
  // (so they can rename without leaving the keyboard)
  if (!e->Handled && e->Key == Keys::F2)
    mProjectName->TakeFocus();
}

void ActiveProjectMenu::OnEditProjectName(Event* e)
{
  mProjectName->SetEditable(true);
  // Text clipping must be turned off when the text is selected otherwise the
  // textbox displays the clipped text with "..." and a bunch of white space.
  mProjectName->SetTextClipping(false);
  mProjectName->TakeFocus();
  mProjectName->mEditTextField->SelectAll();
}

void ActiveProjectMenu::OnProjectNameTextSubmit(Event* e)
{
  String newProjectName = mProjectName->GetText();
  String oldProjectName = mCachedProject->GetProjectName();
  // If the name hasn't changed then the user probably
  // just clicked off the name text so don't do anything
  if (newProjectName == oldProjectName)
    return;

  String projectFolder = mCachedProject->GetProjectFolder();
  String oldProjectFolderName = FilePath::GetDirectoryName(mCachedProject->GetProjectPath());
  String projectParentPath = FilePath::GetDirectoryPath(projectFolder);
  String newProjectFolder = FilePath::Combine(projectParentPath, newProjectName);
  String newProjectFolderName = newProjectName;

  // If this project already exists then let the user know we can't rename to
  // this name
  if (DirectoryExists(newProjectFolder))
  {
    String msg = "Project already exists";
    String extraMsg = "Please choose a different name";
    ModalButtonsAction* modal = new ModalButtonsAction(this, msg.ToUpper(), "CLOSE", extraMsg.ToUpper());
    mLauncher->mActiveModal = modal;
    SelectProject(mCachedProject);
    return;
  }

  // @JoshD: This should ideally check to see if the project folder is in use
  // before trying to rename, but if it is then no major problem happens as the
  // next run of the engine will fail. Properly checking for use is fairly
  // complicated because only certain files/folders will randomly be marked as
  // readonly/archive/etc...

  // Open a confirmation modal for them to rename and move the project.
  // For now include the project name and folder so they know how it will be
  // renamed.
  Array<String> buttonNames;
  buttonNames.PushBack("CONFIRM");
  buttonNames.PushBack("CANCEL");
  String confirmMsg = String::Format("Rename '%s/%s.zeroproj' to '%s/%s.zeroproj'?",
                                     oldProjectFolderName.c_str(),
                                     oldProjectName.c_str(),
                                     newProjectFolderName.c_str(),
                                     newProjectName.c_str());
  ModalButtonsAction* modal = new ModalButtonsAction(this, "RENAME PROJECT?", buttonNames, confirmMsg.ToUpper());
  mLauncher->mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalClosed, OnProjectRenameCanceled);
  ConnectThisTo(modal, Events::ModalButtonPressed, OnProjectRenameConfirmed);
}

void ActiveProjectMenu::OnProjectNameFocusLost(Event* e)
{
  // See OnEditProjectName for why this is turned on dynamically
  mProjectName->SetTextClipping(true);
  mProjectName->SetEditable(false);
}

void ActiveProjectMenu::OnProjectRenameCanceled(Event* e)
{
  SelectProject(mCachedProject);
}

void ActiveProjectMenu::OnProjectRenameConfirmed(ModalButtonEvent* e)
{
  // If the user didn't hit confirm, then just reload the project file
  if (e->mButtonName != "CONFIRM")
  {
    SelectProject(mCachedProject);
    return;
  }

  // Otherwise, we need to remove the project from the recent projects list,
  // rename and move the file, reselect the project so all the ui is up-to-date,
  // and add the new project path to the recent projects list
  mLauncher->RemoveFromRecentProjects(mCachedProject);

  String newProjectName = mProjectName->GetText();
  Status status;
  mCachedProject->RenameAndMoveProject(status, newProjectName);

  // Report failure via a modal
  if (status.Failed())
  {
    // Ideally this should use the status' error message, but there's word-wrap
    // issues with modals right now.
    String msg = String::Format("Cannot rename project to '%s'", newProjectName.c_str());
    ModalButtonsAction* modal = new ModalButtonsAction(mLauncher, "Invalid Rename", "Close", msg);
    mLauncher->mActiveModal = modal;
  }

  SelectProject(mCachedProject);

  mLauncher->AddToRecentProjects(mCachedProject);
}

void ActiveProjectMenu::OnBackupProject(Event* e)
{
  String projectDirectory = mCachedProject->GetProjectFolder();
  String projectName = mCachedProject->GetProjectName();

  // Build the backup's file path based upon the project name and the current
  // time date stamp.
  String backupDirectory = FilePath::Combine(GetUserDocumentsApplicationDirectory(), "Backups");
  String timeStamp = GetTimeAndDateStamp();
  String backupName = BuildString(projectName, timeStamp);
  String backupFilePath = FilePath::CombineWithExtension(backupDirectory, backupName, ".zip");

  // Create the task to backup the project (this can take
  // a while in large projects, mainly with dll plugins)
  BackupProjectJob* job = new BackupProjectJob(projectDirectory, backupFilePath);
  job->mOpenDirectoryOnCompletion = true;
  BackgroundTask* task = Z::gBackgroundTasks->CreateTask(job);
  task->mName = "Backup Project";

  // Create a modal to notify the user of progress
  ModalBackgroundTaskProgessBar* modal = new ModalBackgroundTaskProgessBar(GetRootWidget(), "Archiving", task);
  modal->mCloseOnBackgroundClicked = false;
  mLauncher->mActiveModal = modal;

  ZPrint("Backing up project '%s' to location '%s'.", projectDirectory.c_str(), backupFilePath.c_str());

  task->Execute();
}

void ActiveProjectMenu::OnSizeTaskUpdate(BackgroundTaskEvent* e)
{
  mSize->SetText(e->ProgressText);
}

void ActiveProjectMenu::OnBuildStateChanged(Event* e)
{
  mInstallBuild->SetVisible(false);

  ZeroBuild* activeBuild = mBuildSelector->GetBuild();
  if (activeBuild != nullptr)
  {
    String primaryMsg;
    String secondMsg;
    // Check the warning for changing versions (for now don't warn for upgrades,
    // except to broken builds)
    WarningLevel::Enum result =
        mLauncher->mVersionSelector->CheckVersionForProject(activeBuild, mCachedProject, primaryMsg, false);
    // If this is a sever warning add a second message to double check
    if (result == WarningLevel::Severe)
      secondMsg = "Are you absolutely sure?";

    // If we had any message for any reason then create a modal confirmation
    // before changing the version number
    if (!primaryMsg.Empty())
    {
      ModalConfirmAction* modal = new ModalConfirmAction(this, primaryMsg.ToUpper());
      // Save the new build version on the modal's user data (so we can change
      // if they confirm)
      modal->mUserData = activeBuild;
      modal->mStringUserData = secondMsg;
      mLauncher->mActiveModal = modal;
      ConnectThisTo(modal, Events::ModalConfirmResult, OnConfirmBuildVersionChange);
      return;
    }

    String oldBuildString = mCachedProject->GetDebugIdString();
    String newBuildString = activeBuild->GetDebugIdString();
    BuildId oldBuildId = mCachedProject->GetBuildId();
    ZPrint("Changing project '%s' from Build %s to %s\n",
           mCachedProject->GetProjectPath().c_str(),
           oldBuildString.c_str(),
           newBuildString.c_str());
    // Otherwise no warning message was required, change the project's version
    // number
    mCachedProject->SetBuildId(activeBuild->GetBuildId());

    bool installed = false;
    if (activeBuild != nullptr)
      installed = (activeBuild->mInstallState == InstallState::Installed);
    mLauncher->mMainButton->SetEnabled(installed);

    // Only show the install button if the build can be installed
    if (activeBuild != nullptr && activeBuild->mInstallState == InstallState::NotInstalled && activeBuild->mOnServer)
      mInstallBuild->SetVisible(true);
  }
}

void ActiveProjectMenu::OnConfirmBuildVersionChange(ModalConfirmEvent* e)
{
  mInstallBuild->SetVisible(false);

  if (e->mConfirmed)
  {
    // If the upgrade has a second message (likely just "Are you absolutely
    // sure?") then open a second modal to double check their response.
    if (!e->mStringUserData.Empty())
    {
      ModalConfirmAction* modal = new ModalConfirmAction(this, e->mStringUserData.ToUpper());
      modal->mUserData = e->mUserData;
      mLauncher->mActiveModal = modal;
      ConnectThisTo(modal, Events::ModalConfirmResult, OnConfirmBuildVersionChange);
      return;
    }

    // The user has now confirmed they will change builds
    ZeroBuild* activeBuild = mBuildSelector->GetBuild();

    // Grab the new version number that was cached on the modal's user data
    if (activeBuild != nullptr)
    {
      ZeroBuild* newBuild = (ZeroBuild*)e->mUserData;
      String newBuildIdString = newBuild->GetDisplayString(true);
      String oldBuildIdString = mCachedProject->GetDisplayString(true);
      ZPrint("Changing project '%s' from Build %s to %s\n",
             mCachedProject->GetProjectPath().c_str(),
             oldBuildIdString.c_str(),
             newBuildIdString.c_str());
      mCachedProject->SetBuildId(newBuild->GetBuildId());

      bool installed = (activeBuild->mInstallState == InstallState::Installed);
      mLauncher->mMainButton->SetEnabled(installed);
    }
  }
  // If the user canceled the version change then just set the build selector
  // back to the previous version
  else
  {
    ZeroBuild* build = mLauncher->mVersionSelector->FindExactVersion(mCachedProject);
    mBuildSelector->SetBuild(build);

    // Only show the install button if the build can be installed
    if (build != nullptr && build->mInstallState == InstallState::NotInstalled && build->mOnServer)
      mInstallBuild->SetVisible(true);
  }
}

void ActiveProjectMenu::OnLaunchProject(Event* e)
{
  VersionSelector* versionSelector = mLauncher->mVersionSelector;

  // The project could've changed after we cached it (it could be currently
  // running in zero) so reload it but make sure to preserve the current version
  // id we have loaded in the cache.
  mLauncher->mProjectCache->ReloadProjectFile(mCachedProject, true);
  ZeroBuild* version = versionSelector->FindExactVersion(mCachedProject);
  if (version != nullptr)
  {
    mCachedProject->Save(false);

    if (version->mInstallState == InstallState::Installed)
      versionSelector->RunProject(version, mCachedProject);
    else if (version->mInstallState == InstallState::NotInstalled)
    {
      // The current version isn't installed. Install it then run the project
      BackgroundTask* task = versionSelector->InstallVersion(version);
      ConnectThisTo(task, Events::BackgroundTaskCompleted, OnDownloadCompleted);
    }
    else if (version->mInstallState == InstallState::Installing)
    {
      // It's currently installing, wait for the install to finish then
      // auto-run?
    }
  }
}

void ActiveProjectMenu::OnVersionListLoaded(Event* e)
{
  /// New versions have been loaded, make sure to update the project's page
  /// (since it may have been associated with a previously un-available build).
  SelectProject(mCachedProject);
}

void ActiveProjectMenu::OnShowProjectFolder(Event* e)
{
  Os::SystemOpenFile(mCachedProject->GetProjectFolder().c_str());
}

void ActiveProjectMenu::OnInstallPressed(Event* e)
{
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(Fade(mInstallBuild, Vec4(1, 1, 1, 0), 0.25f));

  ZeroBuild* build = mBuildSelector->GetBuild();
  mLauncher->mVersionSelector->InstallVersion(build);
}

void ActiveProjectMenu::OnRunWithDebugger(Event* e)
{
  // This will currently leak, but it's tricky to deal with the tcp socket
  // destruction (this shouldn't happen too often so whatever for now)
  LauncherDebuggerCommunication* debugCommunication = new LauncherDebuggerCommunication();
  mLauncher->mDummyCommunicators.PushBack(debugCommunication);
  debugCommunication->SendOpenProject(mCachedProject->GetProjectPath());
}

void ActiveProjectMenu::OnDownloadCompleted(Event* e)
{
  VersionSelector* versionSelector = mLauncher->mVersionSelector;

  ZeroBuild* version = versionSelector->FindExactVersion(mCachedProject);
  versionSelector->RunProject(version, mCachedProject);
}

} // namespace Zero
