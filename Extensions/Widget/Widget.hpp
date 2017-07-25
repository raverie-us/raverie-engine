///////////////////////////////////////////////////////////////////////////////
///
/// \file Widget.hpp
/// Declaration of the base widget class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef Widget Element;

//------------------------------------------------------------------ Draw Params
struct DrawParams
{
  uint FrameId;
  GraphicsEngine* System;
};

//-------------------------------------------------------------- Color Transform
struct ColorTransform
{
  Vec4 ColorMultiply;
};

DeclareEnum2(FocusDirection, Forward, Backwards);

DeclareEnum2(DisplayOrigin, TopLeft, Center);

DeclareBitField4(DisplayFlags, 
  // Mouse is over object
  MouseOver, 
  // Mouse is over object or child  has focus
  MouseOverHierarchy, 
  // Object has focus
  Focus,
  // Object has focus or child has focus
  FocusHierarchy);

//Focus Modes
// Soft focus is focus that can be taken away by soft focus
// or hard focus. Usually activated when the mouse moves into some ui and
// it wants to listen to keyboard key presses.
// Hard Focus is restrictive for modes like typing in a text box when
// other ui would not want to steal focus just by the mouse moving in them.
DeclareEnum2(FocusMode, Soft, Hard);

//----------------------------------------------------------------------- Docker
///Docker class is used to dock widgets.
class Docker
{
public:
  virtual void Dock(Widget* widget, DockArea::Enum area) = 0;
  virtual DockArea::Enum GetDockArea() = 0;
  virtual void Zoom(Widget* widget){};
  virtual void Show(Widget* widget){};
  virtual void WidgetDestroyed(Widget* widget){};
  virtual bool StartManipulation(Widget* widget, DockMode::Enum direction) = 0;
  virtual ~Docker() {}
};

//------------------------------------------------------------------------------
namespace TransformUpdateState
{
  enum Enum
  {
    Updated,
    ChildUpdate,
    LocalUpdate
  };
}//namespace TransformUpdateState

//------------------------------------------------------------------ Size Policy
DeclareEnum2(SizeAxis, X, Y);

// Fixed - fixed widget size
// Flex - shares space with other widgets
// Auto - uses default widget size
DeclareEnum3(SizePolicy, Auto, Fixed, Flex);

class SizePolicies
{
public:
  ZilchDeclareType(TypeCopyMode::ValueType);

  SizePolicies()
  {
    XPolicy = SizePolicy::Flex;
    YPolicy = SizePolicy::Flex;
    Size = Vec2::cZero;
  }

  SizePolicies(SizePolicy::Enum xpolicy, SizePolicy::Enum ypolicy)
    :XPolicy(xpolicy), YPolicy(ypolicy), Size(Vec2::cZero)
  {
    XPolicy = xpolicy;
    YPolicy = ypolicy;
  }

  SizePolicies(SizePolicy::Enum xyPolicy)
    : XPolicy(xyPolicy), YPolicy(xyPolicy), Size(Vec2::cZero)
  {
  }

  union
  {
    struct
    {
      SizePolicy::Enum XPolicy;
      SizePolicy::Enum YPolicy;
    };
    SizePolicy::Enum Policy[2];
  };

  Vec2 Size;
};

DeclareEnum2(AttachType,
  // Normal attachment may get redirected to client widget (ScrollArea etc)
  Normal,
  // Attach as direct child
  Direct);

//----------------------------------------------------------------------- Widget
class WidgetHandleManager : public HandleManager
{
public:
  WidgetHandleManager(ExecutableState* state) : HandleManager(state) {}

  void ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize) override;
  byte* HandleToObject(const Handle& handle) override;
  void Delete(const Handle& handle) override;
  bool CanDelete(const Handle& handle) override;
};

/// Widget UI base class. Widget is the base class for all user interface system
/// objects. It is an atomic piece of UI functionality most of which are visible
/// like tool bars or buttons. Two major types of object inherit from widget:
/// Composite classes which can contain other widgets, and the control classes 
/// which implements different UI functionality like button, listbox, etc. 
class Widget : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Widget(Composite* parent, AttachType::Enum attachType = AttachType::Normal);
  virtual ~Widget();
  virtual void OnDestroy();

  Actions* GetActions() override;

  /// Destroy this widget removing it from its parent
  /// Widget will be marked and cleaned up at next update
  void Destroy();

  /// Name of the widget for paths, searching, and debugging
  void SetName(StringParam name);
  String GetName();

  /// Get name to display when debugging widget trees
  virtual String GetDebugName();

  /// Widgets that are not active do not receive use input, are not
  /// drawn and not placed in layouts. Used to disable or hide widgets.
  void SetActive(bool active);
  bool GetActive();
  /// Walks its parents to see if any of them are active.
  bool GetGlobalActive();

  /// Translation of the widget relative to it's parent
  Vec3 GetTranslation(){return mTranslation;}
  void SetTranslation(Vec3 newTranslation);

  /// Size of the widget
  Vec2 GetSize(){return mSize;}
  void SetSize(Vec2 newSize);
  
  /// Sizing
  void SetSizing(SizeAxis::Enum axis, SizePolicy::Enum policy, float size);
  void SetSizing(SizePolicy::Enum policy, Vec2Param size);
  void SetSizing(SizePolicy::Enum policy, float size);

  SizePolicies GetSizePolicy() { return mSizePolicy; }
  void SetSizePolicy(SizePolicies newPolicy) { mSizePolicy = newPolicy; }
  virtual void SizeToContents();

  // Get the minimum size needs for this widget to be useful
  virtual Vec2 GetMinSize();

  /// Function to set translation and size together
  void SetTranslationAndSize(Vec3 newTranslation, Vec2 newSize);

  /// Color tint applied to this widget and it's children
  Vec4 GetColor(){return mColor;}
  void SetColor(Vec4Param color);

  /// Dispatch an event to this widget and all ancestors
  void DispatchBubble(StringParam eventId, Event* event);
  virtual void DispatchDown(StringParam eventId, Event* event) {};

  // Focus
  void TakeFocus();
  bool TryTakeFocus();
  void SoftTakeFocus();
  void HardTakeFocus();
  void SetTakeFocusMode(FocusMode::Type mode);

  // This widget or one of its children should take focus

  // Does this widget or one of its children have focus?
  bool HasFocus();
  void LoseFocus();

  // With clipping enabled the drawing of this widget and its
  // children will be clipped to the size of the widget
  void SetClipping(bool clipping);
  bool GetClipping();

  template<typename type>
  type* CreateAttached(StringParam name){return (type*)CreateAttachedGeneric(name);}
  Element* CreateAttachedGeneric(StringParam name);

  //Tree
  RootWidget* GetRootWidget();

  void UpdateTransformExternal();
  virtual void UpdateTransform();
  virtual Widget* HitTest(Vec2 screenPoint, Widget* skip);
  virtual void ChangeRoot(RootWidget* newRoot){ mRootWidget = newRoot; }
  Composite* GetParent() { return mParent; }
  virtual Composite* GetSelfAsComposite() { return NULL; }
  bool IsAncestorOf(Widget* child);
  void MoveToFront();
  void MoveToBack();

  //Coordinates
  Rect GetRectInParent();
  Rect GetLocalRect();
  Rect GetScreenRect();
  Vec2 ToLocal(Vec2Param screenPoint);
  Vec3 ToLocal(Vec3Param screenPoint);
  Vec2 ToScreen(Vec2Param localPoint);
  Vec3 GetScreenPosition();
  bool Contains(Vec2 screenPoint);

  //Layout functions
  virtual Thickness GetBorderThickness();
  virtual Vec2 Measure(LayoutArea& data);
  bool GetNotInLayout(){ return mNotInLayout; }
  void SetNotInLayout(bool value) { mNotInLayout = value; }

  //Style functions
  DefinitionSet* GetDefinitionSet(){return mDefSet;}
  virtual void ChangeDefinition(BaseDefinition* set);

  void CaptureMouse();
  void ReleaseMouseCapture();

  void SetRotation(float rotation);
  float GetRotation();

  /// Captures the back buffer in the region of this widget.
  virtual void ScreenCaptureBackBuffer(Image& image);
  void ScreenCaptureBackBuffer(Image& image, Rect& subViewport);

  //Docking
  DockMode::Enum GetDockMode() { return mCurDockMode; }
  virtual void SetDockMode(DockMode::Enum dock);
  void SetDockArea(DockArea::Enum dockArea);
  Docker* GetDocker() { return mDocker; }
  void SetDocker(Docker* docker);

  bool IsMouseOver();
  void MarkAsNeedsUpdate(bool local = true);
  void NeedsRedraw();
  TransformUpdateState::Enum GetTransformUpdateState(){return mTransformUpdateState;}

  void SetHideOnClose(bool value) { mHideOnClose = value; }
  void SetInteractive(bool value);
  bool GetVisible() { return mVisible; }
  void SetVisible(bool visible) { mVisible = visible; }

  virtual void Draw(DisplayRender* render, Mat4Param parentTx, ColorTransform& colorTx, DrawParams& params){};
  virtual void DispatchAt(DispatchAtParams& params);

  virtual void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect);
  ViewNode& AddRenderNodes(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Texture* texture);
  void CreateRenderData(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& vertices, PrimitiveType::Enum primitiveType);

//Internals
  IntrusiveLink(Widget, mWidgetLink);

  //Used for sorting layout
  u64 mId;
  BitField<DisplayFlags::Enum> mFlags;
  SizePolicies mSizePolicy;
  bool mNotInLayout;
  bool mHideOnClose;
  bool mDestroyed;
  bool mNeedsRedraw;
  bool mVisible;
  bool mClipping;
  bool mActive;
  bool mInteractive;
  float mAngle;
  Mat4 mWorldTx;
  TransformUpdateState::Enum mTransformUpdateState;
  FocusMode::Type mTakeFocusMode;
  RootWidget* mRootWidget;
  DockMode::Enum mCurDockMode;
  DefinitionSet* mDefSet;
  Vec3 mTranslation;
  Vec2 mSize;
  Vec4 mColor;
  String mName;
  Composite* mParent;
  Docker* mDocker;
  Actions* mActions;
  MultiManager* mManager;
  DisplayOrigin::Type mOrigin;
  float mDragDistance;

  HorizontalAlignment::Enum mHorizontalAlignment;
  VerticalAlignment::Enum mVerticalAlignment;

  friend class Composite;
  friend class MultiDock;
  friend class Layout;
  friend class RootWidget;

  void ClearValues();
  void InternalDestroy();
  void BuildLocalMatrix(Mat4& output);
  bool InputBlocked();
  bool CheckClipping(Vec2Param screenPoint);

private:
  virtual bool TakeFocusOverride();
};


}//namespace Zero
