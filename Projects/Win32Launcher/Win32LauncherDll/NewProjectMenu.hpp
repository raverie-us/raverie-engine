///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------- Template Project Item
class TemplateProjectItem : public Composite
{
public:
  /// Typedefs.
  typedef TemplateProjectItem ZilchSelf;

  /// Constructor.
  TemplateProjectItem(Composite* parent, NewProjectMenu* newProjectMenu,
                      TemplateProject* project);

  /// Composite Interface.
  void UpdateTransform();

  /// Event Response.
  void OnMouseEnter(Event* e);
  void OnMouseExit(Event* e);
  void OnLeftClick(Event* e);
  void OnRightClick(Event* e);
  void OnTemplateProjectPreviewUpdated(Event* e);
  void UpdatePreviewImage();

  Element* mBackground;
  Element* mBorder;
  MultiLineText* mTitle;
  TextureView* mImage;

  TemplateProject* mTemplateProject;
  NewProjectMenu* mNewProjectMenu;
  bool mSelected;
};

//------------------------------------------------------------- New Project Menu
class NewProjectMenu : public Composite
{
public:
  /// Typedefs.
  typedef NewProjectMenu ZilchSelf;

  /// Constructor.
  NewProjectMenu(Composite* parent, LauncherWindow* launcher);

  /// Returns the selected build's id. If no valid build is selected
  /// then the current launcher's id is returned instead.
  BuildId GetBuildId() const;
  void UpdateTemplates();

  void ValidateProjectCreation();
  void SetInvalidProject(StringParam errorMessage);
  /// When this menu is activated / deactivated, we want to change the
  /// launch buttons functionality.
  void OnMenuDisplayed(Event* e);
  void OnMenuHidden(Event* e);
  void OnTemplateListLoaded(Event* e);
  void OnFirstInstallStarted(Event* e);
  void OnVersionChange(Event* e);
  void OnBuildStateChanged(Event* e);
  void OnFilePathTextChanged(Event* e);
  void OnSearchDataModified(Event* e);
  void OnLauncherConfigChanged(Event* e);
  void OnTemplateSelected(ObjectEvent* e);
  void SelectTemplate(TemplateProjectItem* selectedItem);
  void OnCreateProject(Event* e);
  void OnTemplateInstallFinished(BackgroundTaskEvent* e);
  void RunNewlyCreatedProject(CachedProject* project);

  /// Open the windows folder dialog when the 
  void OnSelectLocationPressed(Event* e);
  /// Special case handling of the user trying to run the new project with the active debugger but none exists.
  void OnNewProjectDebuggerCommunicationFailed(LauncherCommunicationEvent* e);
  void OnLocationSelected(OsFileSelection* e);

  /// All templates will be a child of this
  Composite* mTemplateArea;

  TextBox* mNameBox;
  TextBoxButton* mLocationBox;
  TextBox* mTags;
  Text* mProjectCreationErrorMessage;
  TextCheckBox* mLauncherWithDebugger;
  BuildSelector* mBuildSelector;

  LauncherWindow* mLauncher;
  Array<TemplateProjectItem*> mTemplates;

  HandleOf<TemplateProjectItem> mSelectedTemplate;
};

}//namespace Zero
