///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------ No Screenshot Available
class NoScreenshotAvailable : public Composite
{
public:
  /// Constructor.
  NoScreenshotAvailable(Composite* parent);

  /// Composite Interface.
  void UpdateTransform() override;

  Element* mBackground;
  Text* mText;
};

//--------------------------------------------------------------- Recent Project
class RecentProjectItem : public Composite
{
public:
  /// Typedefs.
  typedef RecentProjectItem ZilchSelf;

  /// Constructor.
  RecentProjectItem(Composite* parent, RecentProjectsMenu* projectsMenu,
                    CachedProject* cachedProject);
  
  /// Composite Interface.
  void UpdateTransform() override;

  /// Since a version may not exist at the time of this item's creation (as the
  /// project list is loading) update the connections for which version we are
  /// listening to being installed / uninstalled.
  void UpdateConnections();
  void UpdateScreenshot();
  
  /// Event Response.
  void OnScreenshotUpdated(Event* e);
  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);
  void OnRemovePressed(Event*);
  void OnRemoveModalResult(ModalConfirmEvent* e);

  /// Background used for showing mouse hover.
  Element* mBackground;

  /// The image on the left side
  Widget* mNoScreenshotImage;
  TextureView* mProjectImage;

  Text* mProjectName;
  Text* mDate;
  BuildStatusView* mBuild;

  ToggleIconButton* mFavoriteIcon;
  IconButton* mRemoveButton;

  RecentProjectsMenu* mRecentProjectsMenu;
  CachedProject* mCachedProject;
  ZeroBuild* mCurrentVersion;
};

//--------------------------------------------------------- Recent Projects Menu
class RecentProjectsMenu : public Composite
{
public:
  /// Typedefs.
  typedef RecentProjectsMenu ZilchSelf;

  /// Constructor.
  RecentProjectsMenu(Composite* parent, LauncherWindow* launcher);

  /// Composite Interface.
  void UpdateTransform() override;

  void AddProject(CachedProject* cachedProject);
  void UpdateRecentProjects();

  void OnVersionListLoaded(Event* e);
  void OnRecentProjectsUpdated(Event* e);
  void OnUserTagsModified(Event* e);

  /// Button Event Response.
  void OnNewProjectPressed(ObjectEvent* e);
  void OnRecentProjectsPressed(ObjectEvent* e);
  void OnBrowsePressed(ObjectEvent* e);
  void OnSearchChanged(Event* e);

  /// When this menu is activated / deactivated, we want to change the
  /// launch buttons functionality.
  void OnMenuDisplayed(Event* e);
  void OnMenuHidden(Event* e);

  Array<RecentProjectItem*> mProjects;
  LauncherWindow* mLauncher;
  ScrollArea* mScrollArea;

  /// Handle to the modal so that we can remove it if the page switches.
  HandleOf<Modal> mRemoveModal;

  /// All projects that have already been loaded (key value is the
  /// full path to the zeroproj).
  HashMap<String, CachedProject*> mLoadedProjects;
};

}//namespace Zero
