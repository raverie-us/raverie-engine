///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorViewportMenu.hpp
/// Declaration of the EditorViewportMenu class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations
class EditorViewport;
class ToggleIconButton;

//--------------------------------------------------------- Viewport Menu Button
class ViewportMenuButton : public ButtonBase
{
public:
  ViewportMenuButton(Composite* parent);

  /// Widget Interface.
  void UpdateTransform() override;

  void SetText(StringParam text);
  void SetIcon(StringParam icon);

  Element* mIcon;
  Text* mText;
  Element* mExpandIcon;
};

//--------------------------------------------------------- Editor Viewport Menu
class EditorViewportMenu : public Composite
{
public:
  typedef EditorViewportMenu ZilchSelf;

  EditorViewportMenu(EditorViewport* viewport);

  /// Widget Interface.
  void UpdateTransform() override;

  // Initializes buttons from the given space.
  void InitializeFromSpace(Space* space);

  /// Event Response.
  void OnMouseEnter(Event* e);
  void OnMouseExit(Event* e);

  void FadeIn();
  void FadeOut();

  void OnCameraModePressed(Event* e);
  void OnPerspectiveModePressed(Event* e);

  /// Camera stuff
  void OnCameraButtonPressed(Event* e);

  void OnGridButtonPressed(Event* e);

  /// Once the pop up closes, fade out the menu.
  void OnPopUpClosed(Event*);
  
  /// We don't want to fade out when the mouse leaves if there's an
  /// active menu still showing.
  HandleOf<Widget> mActivePopup;

  /// Our parent viewport.
  EditorViewport* mViewport;

  /// 3D / 2D mode.
  ToggleIconButton* mCameraMode;
  ToggleIconButton* mPerspectiveMode;

  /// Camera options.
  ViewportMenuButton* mCameraButton;

  /// Grid toggle.
  ToggleIconButton* mGridButton;

  Element* mBackground;
};

}//namespace Zero
