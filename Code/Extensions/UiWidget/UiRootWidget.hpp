// MIT Licensed (see LICENSE.md).
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
DeclareEnum4(StencilDrawMode, None, Add, Remove, Test);

// Root Widget
class UiRootWidget : public UiWidget
{
public:
  /// Meta Initialization.
  ZilchDeclareType(UiRootWidget, TypeCopyMode::ReferenceType);

  UiRootWidget();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Updates all widgets and layouts that need to be updated. This should be
  /// called right before rendering.
  void Update();

  /// RootWidget overrides this to account for OnTop widgets.
  UiWidget* CastPoint(Vec2Param worldPoint, UiWidget* ignore = nullptr, bool interactiveOnly = false) override;

  // Keyboard Events
  void PerformKeyDown(Keys::Enum key);
  void PerformKeyUp(Keys::Enum key);
  void PerformKeyboardEvent(KeyboardEvent* e);

  // Mouse Events
  /// This must be called appropriately before mouse clicks.
  void PerformMouseMove(Vec2Param newRootPoint);

  /// For ctrl+click (and other similar operations), 'PerformKeyDown' and
  /// 'PerformKeyUp' need to be called appropriately.
  void PerformMouseDown(MouseButtons::Enum button, Vec2Param rootPoint);
  void PerformMouseUp(MouseButtons::Enum button, Vec2Param rootPoint);
  void PerformMouseScroll(Vec2Param rootPoint, Vec2Param scroll);

  /// Used to update mouse hover and double click timing.
  void UpdateMouseTimers(float dt, ViewportMouseEvent* e);

  void BuildMouseEvent(ViewportMouseEvent* e, Vec2Param rootPoint, MouseButtons::Enum button = MouseButtons::None);

  ///
  void PerformMouseEvent(ViewportMouseEvent* e);
  void PerformMouseButton(ViewportMouseEvent* e);
  void MouseMove(ViewportMouseEvent* e);
  void MouseOver(ViewportMouseEvent* e, UiWidget* newMouseOver);

  /// Finds the Widget at the given location and dispatches an event on the
  /// Widget.
  void DispatchAt(DispatchAtParams& dispatchParams);

  /// Reactive event response
  void OnMouseEvent(ViewportMouseEvent* e);
  void OnMouseButton(ViewportMouseEvent* e);
  void OnMouseUpdate(ViewportMouseEvent* e);
  void OnKeyboardEvent(KeyboardEvent* e);

  /// When we forward events through the RootWidget, they will bubble back up
  /// and we'll get them again. This is used to ignore duplicate events.
  bool mIgnoreEvents;

  // Rendering
  /// Renders the Ui to the given color render target. The depth render target
  /// must have stencil.
  void Render(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth, MaterialBlock* renderPass);

  //-------------- Internals
  typedef Pair<UiWidget*, Vec4> CachedFloatingWidget;
  void RenderWidgets(RenderTasksEvent* e,
                     RenderTarget* color,
                     RenderTarget* depth,
                     MaterialBlock* renderPass,
                     UiWidget* widget,
                     Vec4Param colorTransform,
                     Array<CachedFloatingWidget>* floatingWidgets);

  void AddGraphical(RenderTasksEvent* e,
                    RenderTarget* color,
                    RenderTarget* depth,
                    MaterialBlock* renderPass,
                    Cog* widgetCog,
                    StencilDrawMode::Enum stencilMode,
                    uint stencilIncrement);

  /// Clears the GraphicalRangeInterface and
  void FlushGraphicals(RenderTasksEvent* e, RenderTarget* color, RenderTarget* depth, MaterialBlock* renderPass);

  /// All objects to be rendered will be added to this list.
  GraphicalRangeInterface mGraphicals;

  uint mStencilCount;
  StencilDrawMode::Enum mStencilDrawMode;

  GraphicsRenderSettings mStencilAddSettings;
  GraphicsRenderSettings mStencilRemoveSettings;
  GraphicsRenderSettings mStencilTestSettings;

  // Other
  /// The widget currently in focus.
  void SetFocusWidget(UiWidget* newFocus);
  UiWidget* GetFocusWidget();

  /// The widget that the mouse is currently over.
  UiWidget* GetMouseOverWidget();

  /// The widget that the mouse was pressed down on.
  UiWidget* GetMouseDownWidget();

  /// Used for focus tab logic. This is for keyboard events that have
  /// bubbled through the widget system.
  void OnWidgetKeyDown(KeyboardEvent* e);

  void SetDebugSelected(Cog* selected);
  Cog* GetDebugSelected();

  float mSnapSize;

  /// Used for sending out MouseHover events on logic update.
  Vec2 mLastMousePosition;
  uint mMouseButtonDownCount;

  /// Whether or not to print out debug information to the console about what
  /// the mouse is currently doing.
  bool mDebugMouseInteraction;

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

  /// All widgets marked on top so we can walk these first for picking.
  Array<UiWidget*> mOnTopWidgets;
};

} // namespace Zero
