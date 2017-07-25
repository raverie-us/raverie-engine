///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015-216, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations.
class UiWidget;
class UpdateEvent;
class ViewportMouseEvent;
class KeyboardEvent;
class DispatchAtParams;

/// Add Pixel scalar to everything

//------------------------------------------------------------------ Root Widget
class UiRootWidget : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UiRootWidget();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Updates all widgets and layouts that need to be updated. This should be
  /// called right before rendering.
  void Update();

  /// Used to update mouse hover and double click timing.
  void UpdateMouseTimers(float dt, ViewportMouseEvent* e);

  //------------------------------------------------------------ Keyboard Events
  void PerformKeyDown(Keys::Enum key);
  void PerformKeyUp(Keys::Enum key);
  void PerformKeyboardEvent(KeyboardEvent* e);

  //--------------------------------------------------------------- Mouse Events
  /// This must be called appropriately before mouse clicks.
  void PerformMouseMove(Vec2Param newRootPoint);

  /// For ctrl+click (and other similar operations), 'PerformKeyDown' and 
  /// 'PerformKeyUp' need to be called appropriately.
  void PerformMouseDown(MouseButtons::Enum button, Vec2Param rootPoint);
  void PerformMouseUp(MouseButtons::Enum button, Vec2Param rootPoint);
  void PerformMouseScroll(Vec2Param rootPoint, Vec2Param scroll);

  void BuildMouseEvent(ViewportMouseEvent* e, Vec2Param rootPoint,
                       MouseButtons::Enum button = MouseButtons::None);

  /// 
  void PerformMouseEvent(ViewportMouseEvent* e);
  void PerformMouseButton(ViewportMouseEvent* e);
  void MouseMove(ViewportMouseEvent* e);
  void MouseOver(ViewportMouseEvent* e, UiWidget* newMouseOver);

  /// Focus.
  void RootChangeFocus(UiWidget* newFocus);

  /// Finds the Widget at the given location and dispatches an event on the Widget.
  void DispatchAt(DispatchAtParams& dispatchParams);

  void SetDebugSelected(Cog* selected);
  Cog* GetDebugSelected();

  /// We must have a Widget Component.
  UiWidget* mWidget;

  float mSnapSize;

  /// Used for sending out MouseHover events on logic update.
  Vec2 mLastMousePosition;
  uint mMouseButtonDownCount;

  /// Whether or not to print out debug information to the console about what
  /// the mouse is currently doing.
  bool mDebugMouseInteraction;

  bool mAlwaysUpdate;

  /// Only send the MouseHover event when the mouse has been over
  /// a single widget for this amount of time.
  float mMouseHoverTime;
  /// The amount of time we've been hovering over the 'MouseOver' widget.
  float mCurrHoverTime;

  /// Only send the 'MouseHold' event when the mouse has been holding on
  /// a single widget for this amount of time.
  float mMouseHoldTime;
  /// The amount of time we've been holding over the 'MouseOver' widget.
  float mCurrHoldTime;

  /// The amount of time between clicks to send the 'DoubleClick' event.
  float mDoubleClickTime;
  /// The amount of time since the last click happened. This is used
  /// for determining when to send out the DoubleClick event.
  float mTimeSinceLastClick;
  MouseButtons::Enum mLastClickedButton;
  Vec2 mLastClickPosition;

  /// Used for debugging.
  float mDepthSeparation;

  /// Used to better debug the selected widget.
  UiWidgetHandle mDebugSelectedWidget;

  /// The widget the mouse is currently over
  UiWidgetHandle mMouseOverWidget;

  /// The widget that the mouse was pressed down on.
  UiWidgetHandle mMouseDownWidget;

  /// The widget that currently has focus.
  UiWidgetHandle mFocusWidget;

  /// Used to retain focus when the OS window loses focus
  /// ?? Look more into this
  UiWidgetHandle mFocusWaitingWidget;

  /// The widget that currently has input captured.
  UiWidgetHandle mCapturedWidget;
};

}//namespace Zero
