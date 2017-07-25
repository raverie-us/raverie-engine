///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace NewProjectsUi
{
  const cstr cLocation = "LauncherUi/NewProjects";
  Tweakable(Vec4, TemplateColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, TemplateHoverColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, TemplateSelectedColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, TemplateSelectedHoverColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(Vec4, LabelTextColor, Vec4(1, 1, 1, 1), cLocation);
  Tweakable(float, TemplateWidth, 128.0f, cLocation);
  Tweakable(Vec4, IconButtonBackground, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, InvalidColor, Vec4(1,1,1,1), cLocation);
}

//---------------------------------------------------------------- TemplateProjectItem
//******************************************************************************
TemplateProjectItem::TemplateProjectItem(Composite* parent, NewProjectMenu* newProjectMenu, TemplateProject* project)
  : Composite(parent), mNewProjectMenu(newProjectMenu), mTemplateProject(project)
{
  mSelected = false;
  SetClipping(true);

  // Some values copied for now, change later
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(NewProjectsUi::TemplateColor);
  SetSizing(SizeAxis::X, SizePolicy::Fixed, NewProjectsUi::TemplateWidth);
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Pixels(13, 0), Thickness(Pixels(4, 0, 0, 5))));

  mImage = new TextureView(this);
  mImage->SetNotInLayout(true);

  new Spacer(this, SizePolicy::Flex, Vec2(1));

  String projectName = mTemplateProject->GetDisplayName();
  forRange(String curr, projectName.Split("\\n"))
  {
    Text* name = new Text(this, mLauncherBoldFont, 16);
    name->SetText(curr);
    name->SizeToContents();
  }

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::RightClick, OnRightClick);

  // Listen for the template project being updated (so if it ever downloads a new preview we update)
  ConnectThisTo(mTemplateProject, Events::TemplateProjectPreviewUpdated, OnTemplateProjectPreviewUpdated);
  // Grab the current preview image (if it has one)
  UpdatePreviewImage();
}

//******************************************************************************
void TemplateProjectItem::UpdateTransform()
{
  mBackground->SetSize(mSize);

  Rect rect = GetLocalRect();
  PlaceCenterToRect(rect, mImage);

  Composite::UpdateTransform();
}

//******************************************************************************
void TemplateProjectItem::OnMouseEnter(Event* e)
{
  if(mSelected)
    mBackground->SetColor(NewProjectsUi::TemplateSelectedHoverColor);
  else
    mBackground->SetColor(NewProjectsUi::TemplateHoverColor);
}

//******************************************************************************
void TemplateProjectItem::OnMouseExit(Event* e)
{
  if(mSelected)
    mBackground->SetColor(NewProjectsUi::TemplateSelectedColor);
  else
    mBackground->SetColor(NewProjectsUi::TemplateColor);
}

//******************************************************************************
void TemplateProjectItem::OnLeftClick(Event* e)
{
  ObjectEvent toSend(this);
  DispatchEvent(Events::ButtonPressed, &toSend);
}

//******************************************************************************
void TemplateProjectItem::OnRightClick(Event* e)
{
  LauncherWindow* launcher = mNewProjectMenu->mLauncher;
  DeveloperConfig* devConfig = launcher->mConfigCog->has(DeveloperConfig);
  if(devConfig != nullptr)
    launcher->mVersionSelector->DownloadTemplateProject(mTemplateProject);
}

//******************************************************************************
void TemplateProjectItem::OnTemplateProjectPreviewUpdated(Event* e)
{
  UpdatePreviewImage();
}

//******************************************************************************
void TemplateProjectItem::UpdatePreviewImage()
{
  // Make sure there is a preview texture
  Texture* previewTexture = mTemplateProject->mIconTexture;
  if(previewTexture == nullptr)
    return;

  mImage->SetTexture(previewTexture);
  mImage->SizeToContents();

  Rect rect = GetLocalRect();
  PlaceCenterToRect(rect, mImage);
}

//---------------------------------------------------------------- NewProjectMenu
//******************************************************************************
NewProjectMenu::NewProjectMenu(Composite* parent, LauncherWindow* launcher)
  : Composite(parent), mLauncher(launcher)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(35, 23, 35, 21)));
  SetClipping(true);

  mTemplateArea = new Composite(this);
  mTemplateArea->SetSizing(SizePolicy::Flex, Vec2(1));
  mTemplateArea->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(21, 0), Thickness(0, 0, 0, 11)));

  Composite* topRow = new Composite(this);
  topRow->SetLayout(CreateRowLayout());
  {
    Composite* nameGroup = new Composite(topRow);
    nameGroup->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(500));
    nameGroup->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(0, 5, 10, 0)));
    {
      mNameBox = new TextBox(nameGroup);
      mNameBox->SetReadOnly(false);
      mNameBox->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(18));
      mNameBox->SetStyle(TextBoxStyle::Modern);
      mNameBox->SetHintText("Project Name");
      ConnectThisTo(mNameBox, Events::TextChanged, OnFilePathTextChanged);

      Composite* bottomRow = new Composite(nameGroup);
      bottomRow->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2::cZero, Thickness::cZero));
      {
        Text* label = new Text(bottomRow, mLauncherRegularFont, 11);
        label->SetText("NAME");
        label->SetColor(NewProjectsUi::LabelTextColor);
        
        new Spacer(bottomRow,SizePolicy::Flex, Vec2(1,1));

        mProjectCreationErrorMessage = new Text(bottomRow, mLauncherRegularFont, 11);
        mProjectCreationErrorMessage->SetText("Error message unset");
        mProjectCreationErrorMessage->SetColor(NewProjectsUi::InvalidColor);
        mProjectCreationErrorMessage->SetVisible(false);
      }
    }

    Composite* tagsGroup = new Composite(topRow);
    tagsGroup->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    tagsGroup->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(0, 5, 5, 0)));
    {
      mTags = new TextBox(tagsGroup);
      mTags->SetReadOnly(false);
      mTags->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(18));
      mTags->SetStyle(TextBoxStyle::Modern);

      Text* label = new Text(tagsGroup, mLauncherRegularFont, 11);
      label->SetText("TAGS");
      label->SetColor(NewProjectsUi::LabelTextColor);
    }

    VersionSelector* selector = mLauncher->mVersionSelector;
    ZeroBuild* mostRecent = nullptr;
    if(!selector->mVersions.Empty())
      mostRecent = selector->mVersions.Front();
    mBuildSelector = new BuildSelector(topRow, selector, mostRecent);
  }

  new Spacer(this, SizePolicy::Fixed, Pixels(0, 5));

  Composite* bottomRow = new Composite(this);
  bottomRow->SetLayout(CreateStackLayout());
  {
    Composite* nameGroup = new Composite(bottomRow);
    nameGroup->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
    nameGroup->SetLayout(CreateFillLayout());
    {
      mLocationBox = new TextBoxButton(nameGroup, "OpenFolderIcon");
      mLocationBox->SetReadOnly(false);
      mLocationBox->SetSizing(SizeAxis::X, SizePolicy::Flex, 1);
      mLocationBox->SetSizing(SizeAxis::Y, SizePolicy::Fixed, Pixels(18));
      mLocationBox->SetStyle(TextBoxStyle::Modern);
      ConnectThisTo(mLocationBox, Events::TextChanged, OnFilePathTextChanged);
      ConnectThisTo(mLocationBox->mButton, Events::ButtonPressed, OnSelectLocationPressed);
    }

    Text* label = new Text(bottomRow, mLauncherRegularFont, 11);
    label->SetText("LOCATION");
    label->SetColor(NewProjectsUi::LabelTextColor);

    mLauncherWithDebugger = new TextCheckBox(bottomRow);
    mLauncherWithDebugger->SetText("Run With Debugger");

    bool isDebuggerMode = mLauncher->GetConfig()->mRunDebuggerMode;
    mLauncherWithDebugger->SetActive(isDebuggerMode);
    mLauncherWithDebugger->SetChecked(isDebuggerMode);
  }

  ConnectThisTo(this, Events::MenuDisplayed, OnMenuDisplayed);
  ConnectThisTo(this, Events::MenuHidden, OnMenuHidden);

  mLocationBox->SetText(config->mDefaultProjectSaveLocation);

  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  ConnectThisTo(versionSelector, Events::TemplateListLoaded, OnTemplateListLoaded);
  ConnectThisTo(versionSelector, Events::TemplateListUpdated, OnTemplateListLoaded);
  ConnectThisTo(mBuildSelector, Events::BuildSelected, OnVersionChange);

  // Listen for when the user tags change on the config (most likely from the settings menu)
  ConnectThisTo(mLauncher->mConfigCog, Events::LauncherConfigChanged, OnLauncherConfigChanged);
}

//******************************************************************************
BuildId NewProjectMenu::GetBuildId() const
{
  ZeroBuild* currentBuild = mBuildSelector->GetBuild();
  // If there is no build (likely cause no builds are installed)
  // then just return the version the launcher was made in
  if(currentBuild == nullptr)
    return BuildId::GetCurrentLauncherId();

  return currentBuild->GetBuildId();
}

//******************************************************************************
void NewProjectMenu::UpdateTemplates()
{
  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  TagChainTextBox* search = mLauncher->mSearch;
  
  // Find the name of the previously selected project
  String previousTemplateId;
  TemplateProjectItem* selectedTemplate = mSelectedTemplate;
  if(selectedTemplate != nullptr)
    previousTemplateId = selectedTemplate->mTemplateProject->GetIdString();

  // Destroy all old template ui
  for(size_t i = 0; i < mTemplates.Size(); ++i)
    mTemplates[i]->Destroy();
  mTemplates.Clear();
  
  BuildId buildId = GetBuildId();
  // Create ui for all the templates of the current version number
  Array<TemplateProject*> templates;
  TagSet rejectionTags;

  versionSelector->FindTemplateWithTags(buildId, search->mSearch.ActiveTags,
                                    rejectionTags, search->mSearch.SearchString,
                                    templates, search->mSearch.AvailableTags);
  if(search->mSearch.AvailableTags.Size() == 1)
  {
    SearchViewResult result;
    result.Name = search->mSearch.AvailableTags.All().Front();
    search->mSearch.Results.PushBack(result);
  }
  
  for(size_t i = 0; i < templates.Size(); ++i)
  {
    TemplateProject* project = templates[i];
    TemplateProjectItem* item = new TemplateProjectItem(mTemplateArea, this, project);
    ConnectThisTo(item, Events::ButtonPressed, OnTemplateSelected);
    mTemplates.PushBack(item);
  }

  // Now re-select whatever template was selected before
  for(size_t i = 0; i < mTemplates.Size(); ++i)
  {
    if(mTemplates[i]->mTemplateProject->GetIdString() == previousTemplateId)
    {
      SelectTemplate(mTemplates[i]);
      break;
    }
  }
  
  // If no template is currently selected then select the first on
  if(mSelectedTemplate.IsNull() && mTemplates.Size() != 0)
    SelectTemplate(mTemplates[0]);
}

//******************************************************************************
void NewProjectMenu::ValidateProjectCreation()
{
  String projectName = mNameBox->GetText();
  String projectLocation = mLocationBox->GetText();

  // Save the project to a folder under the project's name
  String projectFolder = FilePath::Combine(projectLocation, projectName);
  String filePath = FilePath::CombineWithExtension(projectFolder, projectName, ".zeroproj");

  bool projectExists = FileExists(filePath);
  
  ZeroBuild* selectedBuild = mBuildSelector->GetBuild();
  bool validBuild = (selectedBuild != nullptr && selectedBuild->mInstallState == InstallState::Installed);

  mProjectCreationErrorMessage->SetVisible(false);
  mNameBox->mBackgroundColor = ToByteColor(ModernTextBoxUi::BackgroundColor);
  mNameBox->mBorderColor = ToByteColor(ModernTextBoxUi::BackgroundColor);
  mNameBox->mFocusBorderColor = ToByteColor(ModernTextBoxUi::FocusBorderColor);

  bool highlightBuildBorder = false;

  // The project name field was empty, don't let the user create that project
  if(projectName.Empty())
  {
    SetInvalidProject("Project name required");
  }
  else if(projectExists || !validBuild)
  {
    if(projectExists)
    {
      SetInvalidProject("Project already exists at given location");
    }
    else if(selectedBuild  == nullptr || selectedBuild->mInstallState != InstallState::Installing)
    {
      highlightBuildBorder = true;
      SetInvalidProject("Please select a valid build version");
    }
  }
  else
  {
    // make sure the project isn't named the same as any of our default libraries
    ContentLibrary* library = Z::gContentSystem->Libraries.FindValue(projectName, NULL);
    if (library)
    {
      // we found a library with the same name as the project, disallow the name
      SetInvalidProject("Project name conflicts with existing libraries");
    }
    else
      mLauncher->mMainButton->SetEnabled(true);
  }

  mBuildSelector->SetHighlight(highlightBuildBorder);
}

//******************************************************************************
void NewProjectMenu::SetInvalidProject(StringParam errorMessage)
{
  mNameBox->mBackgroundColor = ToByteColor(NewProjectsUi::InvalidColor);
  mNameBox->mBorderColor = ToByteColor(NewProjectsUi::InvalidColor);
  mNameBox->mFocusBorderColor = ToByteColor(BuildColors::Deprecated);

  mProjectCreationErrorMessage->SetText(errorMessage);
  mProjectCreationErrorMessage->SetColor(NewProjectsUi::InvalidColor);
  mProjectCreationErrorMessage->SetVisible(true);
  mLauncher->mMainButton->SetEnabled(false);
}

//******************************************************************************
void NewProjectMenu::OnMenuDisplayed(Event* e)
{
  // Setup the launcher's main button for creating the project
  MainButton* button = mLauncher->mMainButton;
  button->SetText("CREATE");
  button->SetSubText("PROJECT");
  button->SetEnabled(true);
  button->SetVisible(true);

  // If the text was empty set it to the text 'Project Name'. Also always select the text.
  if(mNameBox->GetText().Empty())
    mNameBox->SetText("Project Name");
  mNameBox->TakeFocus();

  bool isDebuggerMode = mLauncher->GetConfig()->mRunDebuggerMode;
  if(mLauncherWithDebugger->GetActive() == false && isDebuggerMode == true)
    mLauncherWithDebugger->SetChecked(true);
  mLauncherWithDebugger->SetActive(isDebuggerMode);

  ZeroBuild* latestInstalledBuild = nullptr;

  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  forRange(ZeroBuild* build, versionSelector->mVersions.All())
  {
    if(build->mInstallState == InstallState::Installed)
    {
      latestInstalledBuild = build;
      break;
    }
  }

  if(latestInstalledBuild)
  {
    mBuildSelector->SetBuild(latestInstalledBuild);
  }
  else
  {
    mBuildSelector->mCurrentBuild->mBuildText->SetText("Select Build");
    mBuildSelector->mCurrentBuild->mInstallState->SetText("NONE");
    mBuildSelector->mCurrentBuild->mInstallState->SetColor(BuildColors::Deprecated);

    ConnectThisTo(versionSelector, Events::InstallStarted, OnFirstInstallStarted);
  }

  // When it's pressed, we want to install
  ConnectThisTo(button, Events::ButtonPressed, OnCreateProject);

  // If no template is currently selected then select the first on
  if(mSelectedTemplate.IsNull() && mTemplates.Size() != 0)
    SelectTemplate(mTemplates[0]);

  TagChainTextBox* search = mLauncher->mSearch;
  search->SetVisible(true);

  ConnectThisTo(search, Events::SearchDataModified, OnSearchDataModified);

  UpdateTemplates();
  ValidateProjectCreation();

  MarkAsNeedsUpdate();
}

//******************************************************************************
void NewProjectMenu::OnMenuHidden(Event* e)
{
  // No longer want to listen to button presses on the main button
  mLauncher->mMainButton->GetDispatcher()->Disconnect(this);
  mLauncher->mSearch->GetDispatcher()->Disconnect(this);
}

//******************************************************************************
void NewProjectMenu::OnTemplateListLoaded(Event* e)
{
  UpdateTemplates();
}

//******************************************************************************
void NewProjectMenu::OnFirstInstallStarted(Event* e)
{
  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  GetDispatcher()->DisconnectEvent(Events::InstallCompleted, this);

  ZeroBuild* latestInstalledBuild = nullptr;
  forRange(ZeroBuild* build, versionSelector->mVersions.All())
  {
    if(build->mInstallState == InstallState::Installed || build->mInstallState == InstallState::Installing)
    {
      latestInstalledBuild = build;
      break;
    }
  }

  mBuildSelector->SetBuild(latestInstalledBuild);
}

//******************************************************************************
void NewProjectMenu::OnVersionChange(Event* e)
{
  UpdateTemplates();
  ValidateProjectCreation();

  ZeroBuild* selectedBuild = mBuildSelector->GetBuild();
  if(selectedBuild)
  {
    ConnectThisTo(selectedBuild, Events::InstallStarted, OnBuildStateChanged);
    ConnectThisTo(selectedBuild, Events::InstallCompleted, OnBuildStateChanged);
    ConnectThisTo(selectedBuild, Events::UninstallStarted, OnBuildStateChanged);
    ConnectThisTo(selectedBuild, Events::UninstallCompleted, OnBuildStateChanged);
  }
}

//******************************************************************************
void NewProjectMenu::OnBuildStateChanged(Event* e)
{
  ValidateProjectCreation();
}

//******************************************************************************
void NewProjectMenu::OnFilePathTextChanged(Event* e)
{
  ValidateProjectCreation();
}

//******************************************************************************
void NewProjectMenu::OnSearchDataModified(Event* e)
{
  UpdateTemplates();
}

//******************************************************************************
void NewProjectMenu::OnLauncherConfigChanged(Event* e)
{
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  mLocationBox->SetText(config->mDefaultProjectSaveLocation);
}

//******************************************************************************
void NewProjectMenu::OnTemplateSelected(ObjectEvent* e)
{
  TemplateProjectItem* selectedItem = (TemplateProjectItem*)e->Source;
  SelectTemplate(selectedItem);
}

//******************************************************************************
void NewProjectMenu::SelectTemplate(TemplateProjectItem* selectedItem)
{
  for(size_t i = 0; i < mTemplates.Size(); ++i)
  {
    TemplateProjectItem* item = mTemplates[i];
    // Set the color and selected state on the selected item
    if(item == selectedItem)
    {
      item->mSelected = true;
      item->mBackground->SetColor(NewProjectsUi::TemplateSelectedColor);
    }
    // But set every other item to not be selected
    else
    {
      item->mSelected = false;
      item->mBackground->SetColor(NewProjectsUi::TemplateColor);
    }
  }

  mSelectedTemplate = selectedItem;
}

//******************************************************************************
void NewProjectMenu::OnCreateProject(Event* e)
{
  // No build is selected (likely none installed) so don't do anything
  if(mBuildSelector->GetBuild() == nullptr)
    return;

  String projectName = mNameBox->GetText();
  String projectLocation = mLocationBox->GetText();
  BuildId projectBuildId = GetBuildId();

  // Save the project to a folder under the project's name
  String projectFolder = FilePath::Combine(projectLocation, projectName);

  TemplateProjectItem* selectedItem = mSelectedTemplate;
  if(selectedItem == nullptr)
    return;

  TemplateProject* selectedProject = selectedItem->mTemplateProject;

  // Use the config's settings for where to install template projects
  LauncherConfig* config = mLauncher->mConfigCog->has(LauncherConfig);
  String templateInstallLocation = config->GetTemplateInstallPath();
  
  // Make a task to create the project from the template (download the template, unzip it and rename it)
  HashSet<String> projectTags;
  // At the moment the tags box is nothing more than a text box, for now just split based
  // upon commas until we have an actual tag chain box or something
  StringSplitRange splitRange = mTags->GetText().Split(",");
  for(; !splitRange.Empty(); splitRange.PopFront())
  {
    // Add all non-empty tags
    String tag = splitRange.Front();
    if(!tag.Empty())
      projectTags.Insert(tag);
  }

  ZPrint("Creating template:\n");
  ZPrint("  Project: %s\n", selectedProject->GetIdString().c_str());
  ZPrint("  Folder: %s\n", projectFolder.c_str());
  ZPrint("  ProjectName: %s\n", projectName.c_str());
  ZPrint("  Build: %s\n", projectBuildId.GetFullId(true, true, true).c_str());
  ZPrint("  Project Tags: %s\n", mTags->GetText().c_str());

  // If the project is already downloaded then just create a project from that, otherwise download it
  if(selectedProject->mIsDownloaded)
  {
    CachedProject* cachedProject = mLauncher->mProjectCache->CreateProjectFromTemplate(projectName, projectFolder, selectedProject->GetInstalledTemplatePath(), projectBuildId, projectTags);
    RunNewlyCreatedProject(cachedProject);
  }
  else
  {
    BackgroundTask* task = mLauncher->mVersionSelector->CreateProjectFromTemplate(selectedProject, templateInstallLocation, projectFolder, projectBuildId, projectTags);
    ConnectThisTo(task, Events::BackgroundTaskCompleted, OnTemplateInstallFinished);
    ConnectThisTo(task, Events::BackgroundTaskFailed, OnTemplateInstallFinished);
  }

  // Set the project name and tags back to empty
  mTags->SetText(String());
  mNameBox->SetText(String());
  mLocationBox->SetText(config->mDefaultProjectSaveLocation);
  mLauncher->mMainButton->SetEnabled(false);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void NewProjectMenu::OnTemplateInstallFinished(BackgroundTaskEvent* e)
{
  if(e->mTask->IsCompleted())
  {
    DownloadAndCreateTemplateTaskJob* job = (DownloadAndCreateTemplateTaskJob*)e->mTask->GetFinishedJob();
    CachedProject* project = job->GetOrCreateCachedProject(mLauncher->mProjectCache);
    
    RunNewlyCreatedProject(project);
  }
}

//******************************************************************************
void NewProjectMenu::RunNewlyCreatedProject(CachedProject* project)
{
  if(project == nullptr)
    return;

  // Add the project to the recent projects list (which also saves the config)
  mLauncher->AddToRecentProjects(project);

  LauncherConfig* config = mLauncher->GetConfig();
  if(mLauncherWithDebugger->GetChecked())
  {
    // This will currently leak, but it's tricky to deal with the tcp socket
    // destruction (this shouldn't happen too often so whatever for now)
    LauncherDebuggerCommunication* debugCommunication = new LauncherDebuggerCommunication();
    mLauncher->mDummyCommunicators.PushBack(debugCommunication);
    debugCommunication->SendOpenProject(project->GetProjectPath());
    ConnectThisTo(debugCommunication, Events::LauncherDebuggerCommunicationFailed, OnNewProjectDebuggerCommunicationFailed);
    return;
  }

  // Run the project that we just created
  ZeroBuild* version = mLauncher->mVersionSelector->FindExactVersion(project);
  mLauncher->mVersionSelector->RunProject(version, project);
}

//******************************************************************************
void NewProjectMenu::OnNewProjectDebuggerCommunicationFailed(LauncherCommunicationEvent* e)
{
  CachedProject* project = mLauncher->mProjectCache->LoadProjectFile(e->mProjectFile);
  // Run the project that we just created
  ZeroBuild* version = mLauncher->mVersionSelector->FindExactVersion(project);
  mLauncher->mVersionSelector->RunProject(version, project);
}

//******************************************************************************
void NewProjectMenu::OnSelectLocationPressed(Event* e)
{
  //Set up the callback for when project file is selected
  const String CallBackEvent = "FolderCallback"; 
  if(!GetDispatcher()->IsConnected(CallBackEvent, this))
    ConnectThisTo(this, CallBackEvent, OnLocationSelected);

  //Open the open file dialog
  FileDialogConfig config;
  config.EventName = CallBackEvent;
  config.CallbackObject = this;
  config.Title = "Select a folder";
  config.AddFilter("Zero Project Folder", "*.none");
  config.DefaultFileName = mLocationBox->GetText();
  config.Flags |= FileDialogFlags::Folder;
  Z::gEngine->has(OsShell)->SaveFile(config);
}

//******************************************************************************
void NewProjectMenu::OnLocationSelected(OsFileSelection* e)
{
  if(e->Files.Size() > 0)
  {
    String path = FilePath::GetDirectoryPath(e->Files[0]);
    mLocationBox->SetText(path);
    ValidateProjectCreation();
  }
}

}//namespace Zero
