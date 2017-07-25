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
namespace RecentProjectUi
{
  const cstr cLocation = "LauncherUi/RecentProjects"; 
  Tweakable(Vec4, TextColor,             Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ProjectColor,          Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, ProjectHoverColor,     Vec4(1,1,1,1), cLocation);
  Tweakable(Vec2, ProjectSize,           Vec2(400, 86), cLocation);
  Tweakable(Vec4, NoScreenshotBg,        Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, Favorite,              Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FavoriteHover,         Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FavoriteSelected,      Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, FavoriteSelectedHover, Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, Remove,                Vec4(1,1,1,1), cLocation);
  Tweakable(Vec4, RemoveHover,           Vec4(1,1,1,1), cLocation);
}

//------------------------------------------------------ No Screenshot Available
//******************************************************************************
NoScreenshotAvailable::NoScreenshotAvailable(Composite* parent) : 
  Composite(parent)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(RecentProjectUi::NoScreenshotBg);

  mText = new Text(this, mLauncherRegularFont, 12);
  mText->SetText("NO SCREENSHOT AVAILABLE");
  mText->SetMultiLine(true);
  mText->SetSize(Pixels(90, 40));
  mText->SetColor(RecentProjectUi::TextColor);
}

//******************************************************************************
void NoScreenshotAvailable::UpdateTransform()
{
  // Move the text to the lower left corner
  Vec2 textSize = mText->GetSize();
  Vec3 lowerLeft(Pixels(10), mSize.y - textSize.y - Pixels(13), 0);
  mText->SetTranslation(lowerLeft);

  // Fill the background
  mBackground->SetSize(mSize);

  Composite::UpdateTransform();
}

//--------------------------------------------------------------- Recent Project
//******************************************************************************
RecentProjectItem::RecentProjectItem(Composite* parent,
  RecentProjectsMenu* projectsMenu, CachedProject* cachedProject) :
  Composite(parent),
  mRecentProjectsMenu(projectsMenu),
  mCachedProject(cachedProject)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(RecentProjectUi::ProjectColor);

  VersionSelector* versionSelector = mRecentProjectsMenu->mLauncher->mVersionSelector;
  SetLayout(CreateDockLayout(Thickness::All(Pixels(2))));

  // Create a composite to store the screenshot and the no-screenshot available
  // images so they can be swapped in the same place.
  Composite* imageParent = new Composite(this);
  imageParent->SetDockMode(DockMode::DockLeft);
  imageParent->SetLayout(CreateRatioLayout());
  imageParent->SetSizing(SizePolicy::Fixed, Pixels(164, 92));
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

  // Update the screenshot for the project
  UpdateScreenshot();
  // Try to get a newer screenshot if available
  ConnectThisTo(mCachedProject, Events::ScreenshotUpdated, OnScreenshotUpdated);
  mCachedProject->UpdateTexture();

  Composite* buttonArea = new Composite(this);
  buttonArea->SetDockMode(DockMode::DockRight);
  buttonArea->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(Pixels(0, 11, 5, 9))));
  {
    bool showFavoriteIcon = false;
    if(showFavoriteIcon)
    {
      mFavoriteIcon = new ToggleIconButton(buttonArea);
      mFavoriteIcon->SetEnabledIcon("FavoriteIcon");
      mFavoriteIcon->SetDisabledIcon("FavoriteIcon");
      mFavoriteIcon->mBackground->SetVisible(false);
      mFavoriteIcon->mBorder->SetVisible(false);
      mFavoriteIcon->mIconColor = ToByteColor(RecentProjectUi::FavoriteSelected);
      mFavoriteIcon->mIconHoverColor = ToByteColor(RecentProjectUi::FavoriteSelectedHover);
      mFavoriteIcon->mIconDisabledColor = ToByteColor(RecentProjectUi::Favorite);
      mFavoriteIcon->mIconDisabledHoverColor = ToByteColor(RecentProjectUi::FavoriteHover);
      mFavoriteIcon->SetEnabled(false);
      mFavoriteIcon->SizeToContents();
    }

    new Spacer(buttonArea, SizePolicy::Flex, Vec2(1));

    mRemoveButton = new IconButton(buttonArea);
    mRemoveButton->SetIcon("RemoveProject");
    mRemoveButton->SetToolTip("Remove from this list");
    mRemoveButton->mBackground->SetVisible(false);
    mRemoveButton->mBorder->SetVisible(false);
    mRemoveButton->mIconColor = ToByteColor(RecentProjectUi::Remove);
    mRemoveButton->mIconHoverColor = ToByteColor(RecentProjectUi::RemoveHover);
    mRemoveButton->UpdateIconColor();
    mRemoveButton->SetVisible(false);
    mRemoveButton->SizeToContents();
  }
  
  // If we aren't displaying the build this project is running in,
  // we're going to layout slightly different
  bool showBuild = projectsMenu->mLauncher->GetConfig()->mDisplayBuildOnProjects;
  bool showTags = true;

  Spacer* padding = new Spacer(this, SizePolicy::Fixed, Vec2(12, 1));
  padding->SetDockMode(DockMode::DockLeft);

  Composite* rightSide = new Composite(this);
  rightSide->SetDockMode(DockMode::DockFill);
  rightSide->SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(Pixels(0, 9, 0, 9))));
  {
    mProjectName = new Text(rightSide, "ProjectNameText");
    mProjectName->SetColor(RecentProjectUi::TextColor);
    mProjectName->SetText(mCachedProject->GetProjectName());
    mProjectName->mAlign = TextAlign::Right;
  
    mDate = new Text(rightSide, "ProjectDate");
    mDate->SetColor(RecentProjectUi::TextColor);
    CalendarDateTime projectDateTime;
    GetFileDateTime(mCachedProject->GetProjectPath(), projectDateTime);
    mDate->SetText(String::Format("%d-%d-%d", projectDateTime.Month, projectDateTime.Day, projectDateTime.Year));
    mDate->mAlign = TextAlign::Right;
  
    mBuild = nullptr;
    if(showBuild)
    {
      new Spacer(rightSide, SizePolicy::Fixed, Pixels(1, 9));
  
      Composite* buildRow = new Composite(rightSide);
      buildRow->SetLayout(CreateRowLayout());
      buildRow->SetSizing(SizePolicy::Flex, 1);
      {
        // Used to push the build selector to the right
        new Spacer(buildRow);
  
        mBuild = new BuildStatusView(buildRow);
        mBuild->SetVersion(mCachedProject->GetBuildId());
      }
    }
  
    if(showTags)
    {
      Text* mTags = new Text(rightSide, mLauncherRegularFont, 12);
      mTags->SetColor(RecentProjectUi::TextColor);
      mTags->mAlign = TextAlign::Right;
      mTags->SetText(mCachedProject->GetTagsDisplayString());
    }
  }

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(mRemoveButton, Events::ButtonPressed, OnRemovePressed);

  mCurrentVersion = versionSelector->FindExactVersion(mCachedProject);

  UpdateConnections();
}

//******************************************************************************
void RecentProjectItem::UpdateScreenshot()
{
  Texture* texture = mCachedProject->mScreenshotTexture;
  // If there is a texture then de-activate the no-screenshot image and set the texture as active
  if(texture != nullptr)
  {
    mProjectImage->SetTexture(mCachedProject->mScreenshotTexture);
    mProjectImage->SetActive(true);
    mNoScreenshotImage->SetActive(false);

    float aspect = mProjectImage->mParent->mSize.x / mProjectImage->mParent->mSize.y;
    mProjectImage->ClipUvToAspectRatio(aspect);
  }
  // Otherwise switch back to the no-screenshot image
  else
  {
    mProjectImage->SetActive(false);
    mNoScreenshotImage->SetActive(true);
  }
}

//******************************************************************************
void RecentProjectItem::UpdateTransform()
{
  mBackground->SetSize(mSize);
  Composite::UpdateTransform();
}

//******************************************************************************
void RecentProjectItem::OnScreenshotUpdated(Event* e)
{
  UpdateScreenshot();
}

//******************************************************************************
void RecentProjectItem::OnMouseEnter(MouseEvent* e)
{
  mBackground->SetColor(RecentProjectUi::ProjectHoverColor);
  mRemoveButton->SetVisible(true);
}

//******************************************************************************
void RecentProjectItem::OnMouseExit(MouseEvent* e)
{
  mBackground->SetColor(RecentProjectUi::ProjectColor);
  mRemoveButton->SetVisible(false);
}

//******************************************************************************
void RecentProjectItem::OnLeftClick(MouseEvent* e)
{
  if(e->Handled)
    return;

  mRecentProjectsMenu->mLauncher->SelectActiveProject(mCachedProject, false);
}

//******************************************************************************
void RecentProjectItem::UpdateConnections()
{
  // Disconnect events from the old version
  if(mCurrentVersion != nullptr)
  {
    DisconnectAll(mCurrentVersion, this);
  }

  VersionSelector* versionSelector = mRecentProjectsMenu->mLauncher->mVersionSelector;
  mCurrentVersion = versionSelector->FindExactVersion(mCachedProject);

  // Connect to the new version
  if(mBuild && mCurrentVersion != nullptr)
    mBuild->SetVersion(mCurrentVersion);
}

//******************************************************************************
void RecentProjectItem::OnRemovePressed(Event*)
{
  Modal* modal = new ModalConfirmAction(mRecentProjectsMenu, "REMOVE FROM RECENT PROJECTS");
  mRecentProjectsMenu->mRemoveModal = modal;
  mRecentProjectsMenu->mLauncher->mActiveModal = modal;
  ConnectThisTo(modal, Events::ModalConfirmResult, OnRemoveModalResult);

  modal->TakeFocus();
}

//******************************************************************************
void RecentProjectItem::OnRemoveModalResult(ModalConfirmEvent* e)
{
  if(e->mConfirmed)
  {
    String projectPath = mCachedProject->GetProjectPath();
    ZPrint("Removing project '%s' from recent projects\n", projectPath.c_str());

    // Remove build from recent
    Cog* configCog = mRecentProjectsMenu->mLauncher->mConfigCog;
    RecentProjects* recentProjects = configCog->has(RecentProjects);
    recentProjects->RemoveRecentProject(projectPath, true);
    SaveLauncherConfig(configCog);
  }
}

//--------------------------------------------------------- Recent Projects Menu
//******************************************************************************
RecentProjectsMenu::RecentProjectsMenu(Composite* parent, LauncherWindow* launcher)
  : Composite(parent), mLauncher(launcher)
{
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2::cZero, Thickness(Pixels(0,0,10,0))));
  mScrollArea = new ScrollArea(this);
  mScrollArea->SetSizing(SizePolicy::Flex, 1);

  UpdateRecentProjects();

  VersionSelector* versionSelector = mLauncher->mVersionSelector;
  ConnectThisTo(versionSelector, Events::VersionListLoaded, OnVersionListLoaded);

  ConnectThisTo(mLauncher->mConfigCog, Events::RecentProjectsUpdated, OnRecentProjectsUpdated);
  ConnectThisTo(this, Events::MenuDisplayed, OnMenuDisplayed);
  ConnectThisTo(this, Events::MenuHidden, OnMenuHidden);
}

//******************************************************************************
void RecentProjectsMenu::UpdateTransform()
{
  Vec3 startPos = Pixels(32, 12, 0);
  Vec3 spacing = Pixels(32, 20, 0);

  Vec2 projectSize = RecentProjectUi::ProjectSize;

  uint xCount = uint((mSize.x - startPos.x) / (projectSize.x + spacing.x));
  // If this transform update happens from the launcher being opened and showing the
  // project page right away, the size will be small (since we're animating in) and the
  // xCount we get will be 0. This will cause a zero division (from a mod) below
  // so just bail and wait for another frame.
  if(xCount == 0)
  {
    Composite::UpdateTransform();
    return;
  }

  float totalHeight = 0.0f;

  for(uint i = 0; i < mProjects.Size(); ++i)
  {
    uint x = i % xCount;
    uint y = uint(i / xCount);

    Vec3 currPos = startPos;
    currPos.x += float(x * (projectSize.x + spacing.x));
    currPos.y += float(y * (projectSize.y + spacing.y));

    totalHeight = currPos.y + projectSize.y;

    mProjects[i]->SetTranslation(currPos);
  }

  // Spacing at the bottom
  totalHeight += Pixels(12);

  Vec2 clientSize;
  clientSize.x = mScrollArea->GetSize().x - mScrollArea->GetScrollBarSize();
  clientSize.y = totalHeight;

  mScrollArea->DisableScrollBar(0);

  mScrollArea->SetClientSize(clientSize);

  Composite::UpdateTransform();
}

//******************************************************************************
void RecentProjectsMenu::AddProject(CachedProject* cachedProject)
{
  RecentProjectItem* projectItem = new RecentProjectItem(mScrollArea, this, cachedProject);
  projectItem->SetSize(RecentProjectUi::ProjectSize);
  mProjects.PushBack(projectItem);
}

//******************************************************************************
void RecentProjectsMenu::UpdateRecentProjects()
{
  // Remove all old projects
  for(size_t i = 0; i < mProjects.Size(); ++i)
    mProjects[i]->Destroy();
  mProjects.Clear();

  Cog* configCog = mLauncher->mConfigCog;
  LauncherConfig* launcherConfig = configCog->has(LauncherConfig);
  RecentProjects* recentProjects = configCog->has(RecentProjects);
  if(recentProjects != nullptr)
  {
    Array<String> projects;
    recentProjects->GetProjectsByDate(projects);

    // To make life easier, we need to convert to a format that can be used for the FilterDataSetWithTags
    // function, however this function has a few minor restrictions, most notably that it needs a HashSet
    // to return by reference. Since projects have tags split up into two sets we need to re-merge them here to make life easier.
    Array<ProjectInformation> projectList;
    for(uint i = 0; i < projects.Size(); ++i)
    {
      ProjectInformation info;

      String filePath = projects[i];
      info.mCachedProject = mLoadedProjects.FindValue(filePath, nullptr);
      if(info.mCachedProject == nullptr)
      {
        info.mCachedProject = mLauncher->mProjectCache->LoadProjectFile(projects[i]);
        if(info.mCachedProject == nullptr)
        {
          // Failed to open the project for some reason, just skip it for now
          continue;
        }

        mLoadedProjects.Insert(filePath, info.mCachedProject);
      }

      info.mCachedProject->GetTags(info.mTags);
      projectList.PushBack(info);
    }

    // Filter all of the projects
    TagList rejectionTags, resultTags;
    Array<ProjectInformation> results;
    ProjectPolicy policy;
    String searchText = mLauncher->mSearch->GetText();
    HashSet<String> legacyTags;
    FilterDataSetWithTags(legacyTags, rejectionTags, searchText, projectList, results, resultTags, policy);

    for(uint i = 0; i < results.Size(); ++i)
    {
      CachedProject* cachedProject = results[i].mCachedProject;
      AddProject(cachedProject);
    }
  }
}

//******************************************************************************
void RecentProjectsMenu::OnVersionListLoaded(Event* e)
{
  for(size_t i = 0; i < mProjects.Size(); ++i)
    mProjects[i]->UpdateConnections();
}

//******************************************************************************
void RecentProjectsMenu::OnRecentProjectsUpdated(Event* e)
{
  UpdateRecentProjects();
}

//******************************************************************************
void RecentProjectsMenu::OnUserTagsModified(Event* e)
{
  UpdateRecentProjects();
}

//******************************************************************************
void RecentProjectsMenu::OnBrowsePressed(ObjectEvent* e)
{

}

//******************************************************************************
void RecentProjectsMenu::OnSearchChanged(Event* e)
{
  UpdateRecentProjects();
}

//******************************************************************************
void RecentProjectsMenu::OnMenuDisplayed(Event* e)
{
  mLauncher->mSearch->SetVisible(true);
  mLauncher->mMainButton->SetVisible(false);
  UpdateRecentProjects();

  ConnectThisTo(mLauncher->mSearch, Events::TextChanged, OnSearchChanged);
}

//******************************************************************************
void RecentProjectsMenu::OnMenuHidden(Event* e)
{
  mLauncher->mSearch->GetDispatcher()->Disconnect(this);
  mRemoveModal.SafeDestroy();
}

}//namespace Zero
