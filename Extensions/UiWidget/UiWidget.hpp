////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class Transform;
class Area;
struct LayoutArea;
class UiRootWidget;
class KeyboardEvent;
class UiWidget;

// Typedefs
typedef Array<UiWidget*> UiWidgetArray;

//-------------------------------------------------------------------------------------------- Focus
DeclareEnum2(UiFocusDirection, Forward, Backwards);
void FindNextFocus(UiWidget* widget, UiFocusDirection::Enum direction);

//------------------------------------------------------------------------------------------- Events
namespace Events
{

DeclareEvent(PreTransformUpdate);
DeclareEvent(PostTransformUpdate);
//DeclareEvent(UiWidgetGetMinSize);

}//namespace Events

// Sent with the transform update events.
class UiTransformUpdateEvent : public Event
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  UiTransformUpdateEvent() : mRootWidget(nullptr) {}

  UiRootWidget* GetRootWidget();

  UiRootWidget* mRootWidget;
};

DeclareEnum2(Axis, X, Y);

//------------------------------------------------------------------------------------------- Sizing
// Used for determining how this widget should be sized when in a layout.
DeclareEnum3(UiSizePolicy,
             Auto,  // Uses default widget size
             Fixed, // Fixed widget size
             Flex); // Shares space with other widgets

// Alignments used to shift widgets when in a layout.
DeclareEnum3(UiVerticalAlignment, Bottom, Center, Top);
DeclareEnum3(UiHorizontalAlignment, Left, Center, Right);

//---------------------------------------------------------------------------------------- Dock Mode
// We can't use our bitfield declaration macros because we need to set
// custom values for the different entries.
namespace UiDockMode
{
  typedef uint Type;
  static const cstr EnumName = "UiDockMode";
  enum Enum
  {
    Left   = (1 << 1),
    Top    = (1 << 2),
    Right  = (1 << 3),
    Bottom = (1 << 4),
  };
  enum { Size = 4 };
  static const cstr Names[] =
  {
    "Left", "Top", "Right", "Bottom", NULL
  };
  static const uint Values[] =
  {
    Left, Top, Right, Bottom
  };

}//namespace DockMode

//--------------------------------------------------------------------------- Transform Update State
DeclareEnum3(UiTransformUpdateState,
             Updated,       // The Widget is completely up to date
             ChildUpdate,   // A children of this Widget needs updating
             LocalUpdate);  // This Widget needs updating

//------------------------------------------------------------------------------------- Widget Flags
DeclareBitField10(UiWidgetFlags,
  // If inactive, it will not draw this Widget and all children. It will
  // also not be updated in layouts.
  Active,
  // Whether or not to draw just this widget and all children.
  Visible,
  // Whether or not this Widget should get mouse and keyboard events.
  Interactive,
  // If true, we will be ignored when our parent updates the layout. Disable
  // this if you want to manually place this widget.
  InLayout,
  // Whether or not we want our children to display outside of our size.
  ClipChildren,
  // Mouse is over object
  MouseOver,
  // Mouse is over object or child  has focus
  MouseOverHierarchy,
  // If true, when the widget is clicked on, it will gain focus and keyboard
  // events will be sent directly to this widget.
  CanTakeFocus,
  // Object has focus
  HasFocus,
  // Object has focus or child has focus
  HierarchyHasFocus);

// Macros to make it easy to define setters / getters for all the flags. The reason we need
#define DeclareWidgetFlagSetterGetter(flag)             \
        bool Get##flag()                                \
        {                                               \
          return mFlags.IsSet(UiWidgetFlags::flag);     \
        }                                               \
        void Set##flag(bool state)                      \
        {                                               \
          mFlags.SetState(UiWidgetFlags::flag, state);  \
        }

//--------------------------------------------------------------------- Ui Widget Cast Results Range
class UiWidgetCastResultsRange
{
public:
  /// Typedefs
  typedef UiWidget value_type;
  typedef UiWidget* return_type;

  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ValueType);
  
  /// Constructor.
  UiWidgetCastResultsRange(const UiWidgetArray& overlappingWidgets);

  /// Range Interface.
  bool Empty();
  UiWidget* Front();
  void PopFront();
  uint Size();

  UiWidgetArray mOverlappingWidgets;
  uint mIndex;
};

//------------------------------------------------------------------------------------------- Widget
class UiWidget : public ComponentHierarchy<UiWidget>
{
public:
  /// Typedefs.
  typedef ComponentHierarchy::ChildList ChildList;
  typedef ComponentHierarchy<UiWidget> BaseType;

  /// Meta Initialization.
  ZilchDeclareDerivedTypeExplicit(UiWidget, Component, TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnDestroy(uint flags = 0) override;

  /// When our parent changes, we need to mark our parent for an update.
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;

  /// A layout Component could be added, so we need to be updated.
  void ComponentAdded(BoundType* typeId, Component* component) override;
  void ComponentRemoved(BoundType* typeId, Component* component) override;

  /// The order of children changes how they're 
  void OnChildrenOrderChanged(Event* e);

  /// Returns the minimum size that this widget needs to be.
  virtual Vec2 Measure(Rectangle& data);

  Vec2 GetMinSize();

  /// Returns the parent widget. Will return null on the root widget.
  UiWidget* GetParentWidget();
  UiRootWidget* GetRootWidget();

  void SizeToContents();

  /// Finds the Widget at the given point. All Widgets bellow and including
  /// the 'ignore' widget will not be included. The ignore was added for
  /// trying to find the widget underneath a dragging window. The window is
  /// directly under the mouse, so we want to ignore it.
  UiWidget* CastPoint(Vec2Param worldPoint, UiWidget* ignore = nullptr,
                      bool interactiveOnly = false);
  UiWidgetCastResultsRange CastRect(UiRectParam worldRect, UiWidget* ignore = nullptr, bool interactiveOnly = false);

  /// Returns our rect relative to parent.
  Rectangle GetLocalRectangle();

  /// Returns our world rect.
  Rectangle GetWorldRectangle();

  /// Active getter / setter.
  bool GetActive();
  void SetActive(bool state);
  
  /// Local Translation relative to parent.
  Vec2 GetLocalTranslation();
  void SetLocalTranslation(Vec2Param localTranslation);

  /// Translation in world space.
  Vec2 GetWorldTranslation();
  void SetWorldTranslation(Vec2Param worldTranslation);

  /// Transforms a local point into world space.
  Vec2 TransformPoint(Vec2Param localPosition);

  /// Transforms a world point into this Widget's local space. Note, this is not the same space
  /// as LocalTranslation. LocalTranslation is in this Widget's parent space.
  Vec2 TransformPointInverse(Vec2Param worldPosition);

  /// Size getter / setter. This acts as a shortcut to the Area Component.
  Vec2 GetSize();
  void SetSize(Vec2Param size);

  /// Top left relative to parent.
  Vec2 GetLocalTopLeft();
  void SetLocalTopLeft(Vec2Param localTopLeft);
  Vec2 GetWorldTopLeft();
  void SetWorldTopLeft(Vec2Param worldTopLeft);
  Vec2 GetLocalTopRight();
  void SetLocalTopRight(Vec2Param localTopRight);
  Vec2 GetWorldTopRight();
  void SetWorldTopRight(Vec2Param worldTopRight);
  Vec2 GetLocalBottomLeft();
  void SetLocalBottomLeft(Vec2Param localBottomLeft);
  Vec2 GetWorldBottomLeft();
  void SetWorldBottomLeft(Vec2Param worldBottomLeft);
  Vec2 GetLocalBottomRight();
  void SetLocalBottomRight(Vec2Param localBottomRight);
  Vec2 GetWorldBottomRight();
  void SetWorldBottomRight(Vec2Param worldBottomRight);
  Vec2 GetLocalCenter();
  void SetLocalCenter(Vec2Param localCenter);
  Vec2 GetWorldCenter();
  void SetWorldCenter(Vec2Param worldCenter);

  float GetLocalTop();
  void  SetLocalTop(float localTop);
  float GetWorldTop();
  void  SetWorldTop(float worldTop);
  float GetLocalRight();
  void  SetLocalRight(float localRight);
  float GetWorldRight();
  void  SetWorldRight(float worldRight);
  float GetLocalBottom();
  void  SetLocalBottom(float localBottom);
  float GetWorldBottom();
  void  SetWorldBottom(float worldBottom);
  float GetLocalLeft();
  void  SetLocalLeft(float localLeft);
  float GetWorldLeft();
  void  SetWorldLeft(float worldLeft);

  /// When the area changes, we want to mark ourselves as needing to be updated.
  void OnAreaChanged(Event* e);

  /// Returns the snap size defined by the root widget.
  float GetSnapSize();

  /// Lets the Widget system know that this object has been modified and needs
  /// to be re-laid out. 
  void MarkAsNeedsUpdate();

  /// Used internally to hide the idea of a local transform update.
  void MarkAsNeedsUpdateInternal(bool localUpdate);

  /// Handles the updating of this Widget and the child Widgets. Once called,
  /// it will update the internal TransformUpdateState.
  virtual void UpdateTransform(UiTransformUpdateEvent* e);

  /// Until we get a render in hierarchy order, we need to move our
  /// child widgets in front of us.
  void MoveChildrenToFront(float& currDepth, float amount);

  /// Focus control.
  void TakeFocus();

  /// Gives focus back to the root widget.
  void LoseFocus();

  /// Changes focus to the next applicable widget in the direction based on the
  /// key pressed in the given keyboard event.
  bool TabJump(KeyboardEvent* e);

  /// Changes focus to the next applicable widget in the given direction.
  void TabJumpDirection(UiFocusDirection::Enum direction);

  //----------------------------------------------------------------- Properties
  /// Used for determining how this widget should be sized when in a layout.
  UiSizePolicy::Enum GetSizePolicy(Axis::Enum axis);
  UiSizePolicy::Enum GetSizePolicyX();
  UiSizePolicy::Enum GetSizePolicyY();
  void SetSizePolicy(Axis::Enum axis, UiSizePolicy::Enum policy);
  void SetSizePolicyX(UiSizePolicy::Enum policy);
  void SetSizePolicyY(UiSizePolicy::Enum policy);

  /// If true, we will be ignored when our parent updates the layout. Disable
  /// this if you want to manually place this widget.
  bool GetInLayout();
  void SetInLayout(bool state);

  /// Flex size used in conjunction with mSizePolicy (if FlexSize is set).
  Vec2 GetFlexSize();
  void SetFlexSize(Vec2Param flexSize);

  /// The minimum size this widget has to be when being laid out.
  Vec2 GetAbsoluteMinSize();
  void SetAbsoluteMinSize(Vec2Param minSize);

  /// Alignments used to shift widgets when in a layout.
  UiVerticalAlignment::Enum GetVerticalAlignment();
  void SetVerticalAlignment(UiVerticalAlignment::Enum alignment);

  /// Alignments used to shift widgets when in a layout.
  UiHorizontalAlignment::Enum GetHorizontalAlignment();
  void SetHorizontalAlignment(UiHorizontalAlignment::Enum alignment);

  /// Used in the dock layout.
  UiDockMode::Enum GetDockMode();
  void SetDockMode(UiDockMode::Enum dockMode);

  /// Adds room around the content of this object. For more
  /// information, see the CSS Box Model.
  const Thickness& GetMargins();
  float GetMarginLeft();
  float GetMarginTop();
  float GetMarginRight();
  float GetMarginBottom();
  void SetMarginLeft(float val);
  void SetMarginTop(float val);
  void SetMarginRight(float val);
  void SetMarginBottom(float val);

  void OnPropertyModified(PropertyEvent* e);

  /// Flag getters / setters.
  DeclareWidgetFlagSetterGetter(Visible);
  DeclareWidgetFlagSetterGetter(Interactive);
  DeclareWidgetFlagSetterGetter(ClipChildren);
  DeclareWidgetFlagSetterGetter(MouseOver);
  DeclareWidgetFlagSetterGetter(MouseOverHierarchy);
  DeclareWidgetFlagSetterGetter(CanTakeFocus);
  DeclareWidgetFlagSetterGetter(HasFocus);
  DeclareWidgetFlagSetterGetter(HierarchyHasFocus);

  /// Dependencies.
  Transform* mTransform;
  Area* mArea;

  /// Color that does not cascade to children. It is, however, affected
  /// by the parents color.
  Vec4 mLocalColor;

  /// Color that cascades down to children.
  Vec4 mHierarchyColor;

  BitField<UiWidgetFlags::Enum> mFlags;

private:
  friend class UiRootWidget;

  union
  {
    UiSizePolicy::Enum mSizePolicy[Axis::Size];
    struct
    {
      UiSizePolicy::Enum mSizePolicyX;
      UiSizePolicy::Enum mSizePolicyY;
    };
  };
  
  Vec2 mFlexSize;
  Vec2 mAbsoluteMinSize;
  UiVerticalAlignment::Enum mVerticalAlignment;
  UiHorizontalAlignment::Enum mHorizontalAlignment;
  UiDockMode::Enum mDockMode;
  Thickness mMargins;
  UiTransformUpdateState::Type mTransformUpdateState;
};

typedef ComponentHandle<UiWidget> UiWidgetHandle;

}//namespace Zero
