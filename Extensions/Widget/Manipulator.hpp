///////////////////////////////////////////////////////////////////////////////
///
/// \file Manipulator.hpp
/// Declaration of the MouseManipulation and Gripper.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------- Mouse Manipulation

// Helper class for implementing Drag, and other mouse manipulations.
// Provides relative supports functions and a basic interface.
class MouseManipulation : public Widget
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MouseManipulation(Mouse* mouse, Composite* parent);
  ~MouseManipulation();

  /// Support Functions.
  void ReleaseCapture();

  void CloseAndReturnFocus();

  /// MouseManipulation Interface.
  virtual void OnFocusReset(FocusEvent* event);

  /// Generic Mouse Events.
  virtual void OnMouseUp(MouseEvent* event);
  virtual void OnMouseDown(MouseEvent* event){}
  virtual void OnMouseMove(MouseEvent* event){}

  /// Named MouseUp Events.
  virtual void OnLeftMouseUp(MouseEvent* event) {};
  virtual void OnRightMouseUp(MouseEvent* event){};
  virtual void OnMiddleMouseUp(MouseEvent* event){};

  /// Named MouseDownEvents
  virtual void OnLeftMouseDown(MouseEvent* event) {};
  virtual void OnRightMouseDown(MouseEvent* event) {};
  virtual void OnMiddleMouseDown(MouseEvent* event) {};

  virtual void OnMouseUpdate(MouseEvent* event){};
  virtual void OnMouseScroll(MouseEvent* event) {};
  
  /// Keyboard Events.
  virtual void OnKeyDown(KeyboardEvent* event){};
  virtual void OnKeyUp(KeyboardEvent* event){};

  virtual void OnTargetDestroy(MouseEvent* event){};
  virtual void OnUpdate(UpdateEvent* event){};

protected: 
  /// For relative positions and focus will be
  /// returned to this widget.
  HandleOf<Composite> mRelative;
  Mouse* mMouse;
  Vec2 mMouseStartPosition;
  MouseButtons::Enum mButton;
};

//---------------------------------------------------------- Sizing Manipulation
class SizingManipulation : public MouseManipulation
{
public:
  SizingManipulation(Mouse* mouse, Widget* toSize, DockMode::Enum mode);
  DockMode::Enum mSizerMode;
  HandleOf<Widget> mBeingSized;
  Vec3 mTargetStartPosition;
  Vec2 mTargetStartSize;

  // MouseManipulation Interface
  void OnMouseMove(MouseEvent* event) override;
  void OnMouseUp(MouseEvent* event) override;
};

//---------------------------------------------------------------------- Gripper
class Gripper : public Widget
{
public:
  typedef Gripper ZilchSelf;
  Gripper(Composite* parent, Widget* sizeTarget, DockMode::Enum gripDirection);
  ~Gripper();

  Vec2 Measure(LayoutArea& data) override { return mSize; }

  Widget* mTarget;
  DockMode::Enum mGripDirection;
  bool mMouseOver;

  // Events
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseDown(MouseEvent* event);
  void OnMouseDrag(MouseEvent* event);

private:
  Cursor::Enum GetMouseCursor();
};

class GripZones : public Composite
{
public:
  GripZones(Composite* owner, Widget* sizeTarget);
  Gripper* mGripper[4];
  Gripper* mCornerGripper[4];
  void UpdateTransform() override;
};

}//namespace Zero
