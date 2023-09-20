// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

extern const String cPopUpNormal;
extern const String cPopUpLight;

namespace Events
{
DeclareEvent(PopUpClosed);
}

/// Composite with a background and drop shadow
class FloatingComposite : public Composite
{
public:
  RaverieDeclareType(FloatingComposite, TypeCopyMode::ReferenceType);
  FloatingComposite(Composite* parent, StringParam className = cPopUpNormal);

  void UpdateTransform() override;
  // Fade in the PopUp
  void FadeIn(float time = 0.1f);
  // Fade out the PopUp
  void FadeOut(float time = 0.1f);
  void Slide(Vec3Param offset, float time);

  // Internals
  Element* mBorder;
  Element* mBackground;
  Element* mDropShadow;
};

DeclareEnum3(PopUpCloseMode, MouseOutTarget, MouseDistance, DisableClose);

/// A Floating Pop Up Mix in class.
class PopUp : public FloatingComposite
{
public:
  RaverieDeclareType(PopUp, TypeCopyMode::ReferenceType);
  /// Popup will attach to the root widget of the target and disappear if
  /// the mouse moves outside the target, or the mouse moves away from the popup
  PopUp(Widget* target, PopUpCloseMode::Enum popCloseMode, StringParam className = cPopUpNormal);

  // Position the PopUp below the mouse.
  void SetBelowMouse(Mouse* mouse, Vec2 offset);
  // Shift to be visible on screen
  virtual void ShiftOntoScreen(Vec3 offset);

  // Events
  void OnMouseMove(MouseEvent* event);
  void OnKeyDown(KeyboardEvent* event);
  virtual void OnFocusOut(FocusEvent* event);
  virtual void OnMouseDown(MouseEvent* event);
  virtual void OnAnyGained(FocusEvent* event);
  void OnTargetMouseExit(MouseEvent* event);

  // Internals
  bool mMoved;
  PopUpCloseMode::Enum mCloseMode;
  HandleOf<Widget> mTarget;
};

} // namespace Raverie
