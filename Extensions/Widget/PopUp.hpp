///////////////////////////////////////////////////////////////////////////////
///
/// \file PopUp.hpp
/// Declaration of the PopUp.
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

extern const String PopUpNormal;
extern const String PopUpLight;

namespace Events
{
  DeclareEvent(PopUpClosed);
}

/// Composite with a background and drop shadow
class FloatingComposite : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef FloatingComposite ZilchSelf;
  FloatingComposite(Composite* parent, StringParam className = PopUpNormal);

  void UpdateTransform() override;
  //Fade in the PopUp
  void FadeIn();
  //Fade out the PopUp
  void FadeOut();
  void Slide(Vec3Param offset, float time);

  //Internals
  Element* mBorder;
  Element* mBackground;
  Element* mDropShadow;
};

DeclareEnum2(PopUpCloseMode, MouseOutTarget, MouseDistance);

///A Floating Pop Up Mix in class.
class PopUp : public FloatingComposite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// Popup will attach to the root widget of the target and disappear if
  /// the mouse moves outside the target, or the mouse moves away from the popup
  PopUp(Widget* target, PopUpCloseMode::Enum popCloseMode, StringParam className = PopUpNormal);

  //Position the PopUp below the mouse.
  void SetBelowMouse(Mouse* mouse, Vec2 offset);
  //Shift to be visible on screen
  void ShiftOntoScreen(Vec3 offset);

  //Events
  void OnMouseMove(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  void OnFocusOut(FocusEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnAnyGained(FocusEvent* event);
  void OnTargetMouseExit(MouseEvent* event);

  //Internals
  bool mMoved;
  PopUpCloseMode::Enum mCloseMode;
  HandleOf<Widget> mTarget;
};

}//namespace Zero
