///////////////////////////////////////////////////////////////////////////////
///
/// \file EditorViewportMenu.hpp
/// Implementation of the EditorViewportMenu class.
/// 
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ViewportMenuUi
{
  const cstr cLocation = "EditorUi/Viewport/ViewportMenu";
  Tweakable(float, MouseOffTransparency, 0.8f,          cLocation);
  Tweakable(float, MouseOffSpeed,        0.4f,          cLocation);
  Tweakable(Vec4,  BackgroundColor,      Vec4(1,1,1,1), cLocation);
}

//--------------------------------------------------------- Viewport Menu Button
//******************************************************************************
ViewportMenuButton::ViewportMenuButton(Composite* parent)
  : ButtonBase(parent, "TextButton")
{
  mIcon = NULL;
  mText = NULL;
  mExpandIcon = CreateAttached<Element>("ViewportButtonExpand");
  mBackgroundColor = ToByteColor(IconButtonUi::DefaultColor);
}

//******************************************************************************
void ViewportMenuButton::UpdateTransform()
{
  mExpandIcon->SetTranslation(Vec3(mSize.x - Pixels(12), mSize.y * 0.5f, 0));

  Widget* activeWidget = mIcon;
  if(mIcon == NULL)
    activeWidget = mText;
  
  if(activeWidget)
  {
    float expandLeft = mExpandIcon->mTranslation.x;

    Vec2 minSize = activeWidget->GetMinSize();
    Rect rect = Rect::PointAndSize(mSize * 0.5f - minSize * 0.5f, minSize);
    if(rect.Right() > expandLeft)
      rect.X -= (rect.Right() - expandLeft + Pixels(3));
    activeWidget->SetTranslation(ToVector3(rect.TopLeft()));
  }

  ButtonBase::UpdateTransform();
}

//******************************************************************************
void ViewportMenuButton::SetText(StringParam text)
{
  mText = new Text(this, cText);
  mText->SetText(text);
  mText->SizeToContents();
}

//******************************************************************************
void ViewportMenuButton::SetIcon(StringParam icon)
{
  mIcon = CreateAttached<Element>(icon);
}

//--------------------------------------------------------- Editor Viewport Menu
//******************************************************************************
EditorViewportMenu::EditorViewportMenu(EditorViewport* viewport)
  : Composite(viewport), mViewport(viewport)
{
  SetName("EditorViewportMenu");
  
  // Background
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetNotInLayout(true);

  // Default to mouse off
  SetColor(Vec4(1,1,1, ViewportMenuUi::MouseOffTransparency));

  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(8, 0), Thickness(Pixels(4, 2, 0, 2))));

  // Camera mode
  mCameraMode = new ToggleIconButton(this);
  mCameraMode->SetEnabledIcon("Mode3D");
  mCameraMode->SetDisabledIcon("Mode2D");
  mCameraMode->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(49));
  mCameraMode->SetToolTip("Camera Mode");
  ConnectThisTo(mCameraMode, Events::ButtonPressed, OnCameraModePressed);

  // Perspective mode
  mPerspectiveMode = new ToggleIconButton(this);
  mPerspectiveMode->SetEnabledIcon("Perspective");
  mPerspectiveMode->SetDisabledIcon("Orthographic");
  mPerspectiveMode->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(19));
  mPerspectiveMode->SetToolTip("Perspective Mode");
  ConnectThisTo(mPerspectiveMode, Events::ButtonPressed, OnPerspectiveModePressed);

  // Camera options
  mCameraButton = new ViewportMenuButton(this);
  mCameraButton->SetIcon("CameraOptions");
  mCameraButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(49));
  mCameraButton->SetToolTip("Camera Options");
  ConnectThisTo(mCameraButton, Events::ButtonPressed, OnCameraButtonPressed);

  mGridButton = new ToggleIconButton(this);
  mGridButton->SetEnabledIcon("ViewportGridIconOn");
  mGridButton->SetDisabledIcon("ViewportGridIconOff");
  mGridButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(19));
  mGridButton->SetToolTip("Toggle Grids");
  ConnectThisTo(mGridButton, Events::ButtonPressed, OnGridButtonPressed);

  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

//******************************************************************************
void EditorViewportMenu::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(ViewportMenuUi::BackgroundColor);

  Composite::UpdateTransform();
}

//******************************************************************************
void EditorViewportMenu::InitializeFromSpace(Space* space)
{
  bool gridsActive = false;
  forRange(Cog& object, space->AllObjects())
  {
    if(GridDraw* grid = object.has(GridDraw))
    {
      if(grid->mActive)
      {
        gridsActive = true;
        break;
      }
    }
  }
  mGridButton->SetEnabled(gridsActive);

  // have the viewport ribbon take the current levels editor camera settings
  EditorMode::Enum mode = Z::gEditor->GetEditMode();

  if (mode == EditorMode::Mode2D)
    mCameraMode->SetEnabled(false);
  else
    mCameraMode->SetEnabled(true);

  if (Cog* camCog = mViewport->mEditorCamera)
  {
    if (Camera* cam = camCog->has(Camera))
    {
      if (cam->GetPerspectiveMode() == PerspectiveMode::Perspective)
        mPerspectiveMode->SetEnabled(true);
      else
        mPerspectiveMode->SetEnabled(false);
    }
  }
}

//******************************************************************************
void EditorViewportMenu::OnMouseEnter(Event* e)
{
  FadeIn();
}

//******************************************************************************
void EditorViewportMenu::OnMouseExit(Event* e)
{
  // Don't fade out if a menu is open
  if(mActivePopup.IsNotNull())
    FadeOut();
}

//******************************************************************************
void EditorViewportMenu::FadeIn()
{
  Vec4 color(1,1,1,1);
  float t = ViewportMenuUi::MouseOffSpeed;

  ActionSequence* seq = new ActionSequence(this, ActionExecuteMode::FrameUpdate);
  seq->Add(AnimatePropertyGetSet(Widget, Color, Ease::Quad::InOut, this, t, color));
}

//******************************************************************************
void EditorViewportMenu::FadeOut()
{
  Vec4 color(1,1,1, ViewportMenuUi::MouseOffTransparency);
  float t = ViewportMenuUi::MouseOffSpeed;

  ActionSequence* seq = new ActionSequence(this, ActionExecuteMode::FrameUpdate);
  seq->Add(AnimatePropertyGetSet(Widget, Color, Ease::Quad::InOut, this, t, color));
}

//******************************************************************************
void EditorViewportMenu::OnCameraModePressed(Event* e)
{
  EditorMode::Enum mode = Z::gEditor->GetEditMode();

  if (mode == EditorMode::Mode2D)
  {
    Z::gEditor->SetEditMode(EditorMode::Mode3D);
    mPerspectiveMode->SetEnabled(true);
  }
  else
  {
    Z::gEditor->SetEditMode(EditorMode::Mode2D);
    mPerspectiveMode->SetEnabled(false);
  }
}

//******************************************************************************
void EditorViewportMenu::OnPerspectiveModePressed(Event* e)
{
  if(Cog* camCog = mViewport->mEditorCamera)
  {
    if(Camera* cam = camCog->has(Camera))
    {
      if(mPerspectiveMode->GetEnabled())
        cam->SetPerspectiveMode(PerspectiveMode::Perspective);
      else
        cam->SetPerspectiveMode(PerspectiveMode::Orthographic);
    }
  }
}


//******************************************************************************
void EditorViewportMenu::OnCameraButtonPressed(Event* e)
{
  ContextMenu* contextMenu = new ContextMenu(mCameraButton);
  contextMenu->SetTranslation(mCameraButton->GetScreenPosition() + Pixels(0, mCameraButton->mSize.y, 0));
  contextMenu->AddCommandByName("ResetCamera");
  contextMenu->AddDivider();
  contextMenu->AddCommandByName("AlignSelectedCameraToCamera");
  contextMenu->AddCommandByName("AlignCameraToSelectedCamera");
  contextMenu->SizeToContents();

  mActivePopup = contextMenu;
  ConnectThisTo(contextMenu, Events::PopUpClosed, OnPopUpClosed);
}

//******************************************************************************
void EditorViewportMenu::OnGridButtonPressed(Event* e)
{
  Space* space = mViewport->mEditSpace;
  if(space == NULL)
    return;

  bool gridFound = false;
  bool active = mGridButton->GetEnabled();
  forRange(Cog& object, space->AllObjects())
  {
    if(GridDraw* grid = object.has(GridDraw))
    {
      grid->mActive = active;
      gridFound = true;
    }
  }

  // If there was no grid found, add a grid for them
  if(!gridFound && active)
  {
    if(Cog* camCog = mViewport->mEditorCamera)
    {
      // Add the grid draw to the level settings object
      Cog* levelSettings = space->FindObjectByName(SpecialCogNames::LevelSettings);
      levelSettings->AddComponentByType(ZilchTypeId(GridDraw));
      GridDraw* grid = levelSettings->has(GridDraw);

      // Set the grid axis based on the edit mode
      if(Z::gEditor->GetEditMode() == EditorMode::Mode3D)
        grid->SetAxis(AxisDirection::Y); 
      else
        grid->SetAxis(AxisDirection::Z);
    }
   
  }
}

//******************************************************************************
void EditorViewportMenu::OnPopUpClosed(Event*)
{
  // Fade out unless the mouse is still over the menu
  if(!IsMouseOver())
    FadeOut();
}

}// namespace Zero
