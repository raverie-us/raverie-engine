///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(LauncherConfigChanged);
}//namespace Events

/// The various configuration settings for the launcher.
class SettingsMenu : public Composite
{
public:
  /// Typedefs.
  typedef SettingsMenu ZilchSelf;

  /// Constructor.
  SettingsMenu(Modal* parent, LauncherWindow* launcher);

  /// Composite Interface.
  void UpdateTransform() override;

  void OnActivated(Event* e);
  void OnDeactivated(Event* e);
  void OnProjectLocationTextSubmit(Event* e);
  void OnBrowseProjectLocation(Event* e);
  void OnBrowseProjectLocationSelected(OsFileSelection* e);
  void OnDownloadLocationTextChanged(Event* e);
  void OnDownloadLocationTextSubmit(Event* e);
  void OnBrowseDownloadLocation(Event* e);
  void OnBrowseDownloadLocationSelected(OsFileSelection* e);
  void OnChangeDownloadLocation(ModalConfirmEvent* e);
  void OnAutoRunModeSelected(Event* e);
  void OnMaxRecentProjectsModified(Event* e);
  void OnAutoCheckForMajorUpdatesModified(Event* e);
  void OnShowDevelopModified(Event* e);
  void OnShowExperimentalBranchesModified(Event* e);

  void OnCheckForUpdates(Event* e);
  void OnCheckForLauncherUpdates(Event* e);
  void OnRevertToDefaults(Event* e);
  void OnRevertToDefaultsConfirmation(ModalConfirmEvent* e);

  /// Update all of the ui to display the current config values
  void LoadFromConfig();
  void SaveConfig();
  void RevertConfigToDefaults();

  void OnSettingsPressed(Event*);
  void OnModalClosed(Event*);

  void AnimateIn();
  void AnimateOut();

  /// All widgets, in top down order that need to be animated in.
  Array<Widget*> mWidgetsToAnimate;

  Element* mModalBackground;
  Element* mBackground;
  ComboBox* mAutoRunMode;
  TextBoxButton* mDefaultProjectLocation;
  TextBoxButton* mDownloadLocation;
  LauncherWindow* mLauncher;
  TextBox* mMaxNumberOfRecentProjects;
  CheckBox* mAutoCheckForMajorUpdatesCheckBox;
  CheckBox* mShowDevelopCheckBox;
  CheckBox* mShowExperimentalBranchesCheckBox;
  /// Handle to the modal so that we can remove it if the page switches.
  HandleOf<ModalConfirmAction> mConfirmModal;
  HandleOf<ToolTip> mToolTip;
  Modal* mParentModal;
};

}//namespace Zero
