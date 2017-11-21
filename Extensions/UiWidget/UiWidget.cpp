////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const float cDefaultSnapSize = 1.0f;

//------------------------------------------------------------------------------------------- Events
namespace Events
{

DefineEvent(PreTransformUpdate);
DefineEvent(PostTransformUpdate);

}//namespace Events

 //**************************************************************************************************
ZilchDefineType(UiTransformUpdateEvent, builder, type)
{
  ZilchBindGetterProperty(RootWidget);
}

//**************************************************************************************************
UiRootWidget* UiTransformUpdateEvent::GetRootWidget()
{
  return mRootWidget;
}

//--------------------------------------------------------------------- Ui Widget Cast Results Range
//**************************************************************************************************
ZilchDefineType(UiWidgetCastResultsRange, builder, type)
{
  // METAREFACTOR - range type?
  //BindAsRangeType();
  ZilchBindMethod(Empty);
  ZilchBindMethod(Front);
  ZilchBindMethod(PopFront);
  ZilchBindMethod(Size);
}

//**************************************************************************************************
UiWidgetCastResultsRange::UiWidgetCastResultsRange(const UiWidgetArray& overlappingWidgets) :
  mOverlappingWidgets(overlappingWidgets),
  mIndex(0)
{
  
}

//**************************************************************************************************
bool UiWidgetCastResultsRange::Empty()
{
  return mIndex >= mOverlappingWidgets.Size();
}

//**************************************************************************************************
UiWidget* UiWidgetCastResultsRange::Front()
{
  if(Empty())
  {
    DoNotifyException("Range Empty", "Cannot get front on an empty range");
    return nullptr;
  }

  return mOverlappingWidgets[mIndex];
}

//**************************************************************************************************
void UiWidgetCastResultsRange::PopFront()
{
  if(Empty())
  {
    DoNotifyException("Range Empty", "Cannot pop front on an empty range");
    return;
  }

  ++mIndex;
}

//**************************************************************************************************
uint UiWidgetCastResultsRange::Size()
{
  return mOverlappingWidgets.Size();
}

//------------------------------------------------------------------------------------------- Widget
//**************************************************************************************************
ZilchDefineType(UiWidget, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDependency(Transform);
  ZeroBindDependency(Area);

  // Events
  ZeroBindEvent(Events::PreTransformUpdate, UiTransformUpdateEvent);
  ZeroBindEvent(Events::PostTransformUpdate, UiTransformUpdateEvent);

  ZilchBindGetterSetterProperty(Active);
  ZilchBindGetterSetterProperty(Visible);
  ZilchBindGetterSetterProperty(Interactive);
  ZilchBindGetterSetterProperty(LocalTranslation)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(WorldTranslation)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(Size)->AddAttribute(PropertyAttributes::cLocalModificationOverride);

  ZilchBindGetterProperty(LocalRectangle);
  ZilchBindGetterProperty(WorldRectangle);

  ZilchBindGetterSetter(LocalTopLeft);
  ZilchBindGetterSetter(WorldTopLeft);
  ZilchBindGetterSetter(LocalTopRight);
  ZilchBindGetterSetter(WorldTopRight);
  ZilchBindGetterSetter(LocalBottomLeft);
  ZilchBindGetterSetter(WorldBottomLeft);
  ZilchBindGetterSetter(LocalBottomRight);
  ZilchBindGetterSetter(WorldBottomRight);
  ZilchBindGetterSetter(LocalCenter);
  ZilchBindGetterSetter(WorldCenter);

  ZilchBindGetterSetter(LocalTop);
  ZilchBindGetterSetter(WorldTop);
  ZilchBindGetterSetter(LocalRight);
  ZilchBindGetterSetter(WorldRight);
  ZilchBindGetterSetter(LocalBottom);
  ZilchBindGetterSetter(WorldBottom);
  ZilchBindGetterSetter(LocalLeft);
  ZilchBindGetterSetter(WorldLeft);

  ZilchBindFieldProperty(mAbsoluteMinSize);
  ZilchBindFieldProperty(mLocalColor);
  ZilchBindFieldProperty(mHierarchyColor);
  ZilchBindGetterSetterProperty(ClipChildren);
  ZilchBindGetterSetterProperty(InLayout)->ZeroLocalModificationOverride();

  static const String sLayoutGroup = "Layout";

  ZilchBindGetterSetterProperty(SizePolicyX)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(SizePolicyY)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindFieldProperty(mFlexSize)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);

  ZilchBindGetterSetterProperty(VerticalAlignment)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(HorizontalAlignment)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);

  ZilchBindGetterSetterProperty(MarginLeft)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(MarginTop)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(MarginRight)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(MarginBottom)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);

  ZilchBindGetterSetterProperty(DockMode)->ZeroLocalModificationOverride()->ZeroSetPropertyGroup(sLayoutGroup);
  ZilchBindGetterSetterProperty(CanTakeFocus);
  ZilchBindGetter(MouseOver);
  ZilchBindGetter(MouseOverHierarchy);
  ZilchBindGetter(HasFocus);
  ZilchBindGetter(HierarchyHasFocus);

  ZilchBindGetter(ParentWidget);
  ZilchBindGetter(RootWidget);

  ZilchBindMethodProperty(SizeToContents);
  ZilchBindMethod(TakeFocus);
  ZilchBindMethod(LoseFocus);
  ZilchBindMethod(TabJump);
  ZilchBindMethod(TabJumpDirection);
  ZilchBindMethod(TransformPoint);
  ZilchBindMethod(TransformPointInverse);
  ZilchBindMethod(CastPoint);
  ZilchBindMethod(CastRect);
  ZilchBindMethod(UpdateTransform);
}

//**************************************************************************************************
void UiWidget::Serialize(Serializer& stream)
{
  // Serialize flags
  u32 flagMask = UiWidgetFlags::MouseOver | UiWidgetFlags::MouseOverHierarchy |
                 UiWidgetFlags::HasFocus | UiWidgetFlags::HierarchyHasFocus;
  u32 flagDefaults = UiWidgetFlags::Active | UiWidgetFlags::Visible |
                     UiWidgetFlags::Interactive | UiWidgetFlags::InLayout;
  SerializeBits(stream, mFlags, UiWidgetFlags::Names, flagMask, flagDefaults);

  SerializeNameDefault(mLocalColor, Vec4(1));
  SerializeNameDefault(mHierarchyColor, Vec4(1));
  SerializeEnumNameDefault(UiSizePolicy, mSizePolicyX, UiSizePolicy::Auto);
  SerializeEnumNameDefault(UiSizePolicy, mSizePolicyY, UiSizePolicy::Auto);
  SerializeNameDefault(mFlexSize, Vec2(1));
  SerializeNameDefault(mAbsoluteMinSize, Vec2(1));
  SerializeEnumNameDefault(UiVerticalAlignment, mVerticalAlignment, UiVerticalAlignment::Top);
  SerializeEnumNameDefault(UiHorizontalAlignment, mHorizontalAlignment, UiHorizontalAlignment::Left);
  stream.SerializeFieldDefault("MarginLeft",   mMargins.Left,   0.0f);
  stream.SerializeFieldDefault("MarginTop",    mMargins.Top,    0.0f);
  stream.SerializeFieldDefault("MarginRight",  mMargins.Right,  0.0f);
  stream.SerializeFieldDefault("MarginBottom", mMargins.Bottom, 0.0f);
  SerializeEnumNameDefault(UiDockMode, mDockMode, UiDockMode::Left);
}

//**************************************************************************************************
void UiWidget::Initialize(CogInitializer& initializer)
{
  ComponentHierarchy::Initialize(initializer);
  mTransform = GetOwner()->has(Transform);
  mArea = GetOwner()->has(Area);

  ConnectThisTo(GetOwner(), Events::AreaChanged, OnAreaChanged);
  ConnectThisTo(GetOwner(), Events::ChildrenOrderChanged, OnChildrenOrderChanged);
  ConnectThisTo(GetOwner(), Events::PropertyModified, OnPropertyModified);
  ConnectThisTo(GetOwner(), Events::PropertyModifiedIntermediate, OnPropertyModified);

  // If we're dynamically added, we need to let our parent know that it needs
  // to update. Unfortunately, this means that when the Ui is created, the entire
  // tree is updated once. We could change this so this is only called if
  // the component was added dynamically once that information is available.
  MarkAsNeedsUpdate();

  // Currently, BottomLeft is the only thing supported by the widget system
  mArea->SetOrigin(Location::BottomLeft);
}

//**************************************************************************************************
void UiWidget::OnDestroy(uint flags)
{
  // We need to tell our parent that it needs to be updated
  if(mParent)
    mParent->MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::AttachTo(AttachmentInfo& info)
{
  ComponentHierarchy::AttachTo(info);

  if(mParent)
    mParent->MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::Detached(AttachmentInfo& info)
{
  // Notify our old parent that it needs to be updated before
  if(mParent)
    mParent->MarkAsNeedsUpdate();

  ComponentHierarchy::Detached(info);
}

//**************************************************************************************************
void UiWidget::ComponentAdded(BoundType* typeId, Component* component)
{
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::ComponentRemoved(BoundType* typeId, Component* component)
{
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::OnChildrenOrderChanged(Event* e)
{
  if(mParent)
    mParent->MarkAsNeedsUpdate();
}

//**************************************************************************************************
Vec2 UiWidget::Measure(Rectangle& data)
{
  bool xFixed = (GetSizePolicyX() == UiSizePolicy::Fixed);
  bool yFixed = (GetSizePolicyY() == UiSizePolicy::Fixed);

  // If we're fixed on both axes, just return our current size
  if(xFixed && yFixed)
    return GetSize();

  Vec2 measuredSize = mAbsoluteMinSize;

  // Query our layout if we have one
  if(UiLayout* layout = GetOwner()->has(UiLayout))
  {
    measuredSize = layout->Measure(data);

    // Clamp to our min sizes
    if(measuredSize.x < mAbsoluteMinSize.x)
      measuredSize.x = mAbsoluteMinSize.x;
    if(measuredSize.y < mAbsoluteMinSize.y)
      measuredSize.y = mAbsoluteMinSize.y;
  }

  // Account for fixed sizes
  if(xFixed)
    measuredSize.x = GetSize().x;
  if(yFixed)
    measuredSize.y = GetSize().y;

  return measuredSize;
}

//**************************************************************************************************
Vec2 UiWidget::GetMinSize()
{
  if(UiLayout* layout = GetOwner()->has(UiLayout))
  {
    // We don't want to set the size because we want the layout to return
    // the minimum size it needs
    Rectangle defaultRect;
    return Math::Max(layout->Measure(defaultRect), mAbsoluteMinSize);
  }

  Vec2 minSize = mAbsoluteMinSize;

  if(Sprite* sprite = GetOwner()->has(Sprite))
  {
    Vec2 spriteSize = sprite->GetSpriteSource()->GetSize();
    minSize = Math::Max(spriteSize, minSize);
  }
  if(SpriteText* spriteText = GetOwner()->has(SpriteText))
  {
    Vec2 spriteTextSize = spriteText->MeasureText();
    minSize = Math::Max(spriteTextSize, minSize);
  }

  return minSize;
}

//**************************************************************************************************
UiWidget* UiWidget::GetParentWidget()
{
  return mParent;
}

//**************************************************************************************************
UiRootWidget* UiWidget::GetRootWidget()
{
  Cog* root = GetRoot()->GetOwner();
  return root->has(UiRootWidget);
}

//**************************************************************************************************
void UiWidget::SizeToContents()
{
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "Size", GetSize());
  SetSize(GetMinSize());
}

//**************************************************************************************************
UiWidget* UiWidget::CastPoint(Vec2Param worldPoint, UiWidget* ignore, bool interactiveOnly)
{
  // Check to see if it's ignoring this widget or any children
  if(ignore == this)
    return nullptr;

  // All reasons a Widget should not be picked
  if(!GetActive() || !GetVisible() || GetOwner()->GetMarkedForDestruction())
    return nullptr;

  // Check whether or not the position is within our rect
  Rectangle worldRect = GetWorldRectangle();
  bool within = worldRect.Contains(worldPoint);

  // If clipping is enabled on this Widget, we want to ignore any points
  // outside the clip region
  if(GetClipChildren() && !within)
    return nullptr;

  // The children further down in the hierarchy are displayed in front, so we
  // want to walk the children in reverse to find the front most first
  forRange(UiWidget& child, GetChildrenReverse())
  {
    UiWidget* hit = child.CastPoint(worldPoint, ignore, interactiveOnly);
    if(hit)
      return hit;
  }
  
  // We were hit if we didn't hit a child but we were within our boundaries
  // Check to see if we only want interactive objects
  bool failedInteractive = (interactiveOnly && !GetInteractive());
  if(within && !failedInteractive)
    return this;
  
  return nullptr;
}

//**************************************************************************************************
void CastRectInternal(UiWidget* widget, UiRectParam worldRect, UiWidget* ignore, bool interactiveOnly,
                      UiWidgetArray& overlapping)
{
  // Skip ignored widget
  if(widget == ignore)
    return;

  // Skip non-interactive based on the filter
  if(interactiveOnly && !widget->GetInteractive())
    return;

  // Check to see if the current widget overlaps with the given rect
  Rectangle currWorldRect = widget->GetWorldRectangle();
  if(currWorldRect.Overlap(worldRect))
    overlapping.PushBack(widget);
  // If clipping is enabled and we didn't overlap, don't recurse the children
  else if(widget->GetClipChildren())
    return;

  // Walk children
  forRange(UiWidget& child, widget->GetChildren())
    CastRectInternal(&child, worldRect, ignore, interactiveOnly, overlapping);
}

//**************************************************************************************************
UiWidgetCastResultsRange UiWidget::CastRect(UiRectParam worldRect, UiWidget* ignore, bool interactiveOnly)
{
  UiWidgetArray overlapping;
  overlapping.Reserve(15);
  CastRectInternal(this, worldRect, ignore, interactiveOnly, overlapping);
  return UiWidgetCastResultsRange(overlapping);
}

//**************************************************************************************************
Rectangle UiWidget::GetLocalRectangle()
{
  return Rectangle::PointAndSize(GetLocalTranslation(), GetSize());
}

//**************************************************************************************************
Rectangle UiWidget::GetWorldRectangle()
{
  return Rectangle::PointAndSize(GetWorldTranslation(), GetSize());
}

//**************************************************************************************************
bool UiWidget::GetActive()
{
  return mFlags.IsSet(UiWidgetFlags::Active);
}

//**************************************************************************************************
void UiWidget::SetActive(bool state)
{
  if(GetActive() != state)
  {
    mFlags.SetState(UiWidgetFlags::Active, state);
    MarkAsNeedsUpdate();
    
    // We want our parent to re-layout
    //if(mParent)
      //mParent->MarkAsNeedsUpdate();

    ObjectEvent objectEvent(this);
    // Activated does not bubble as our parents can already be considered active
    if(state)
      DispatchEvent(Events::Activated, &objectEvent);
    else
      DispatchEvent(Events::Deactivated, &objectEvent);
  }
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalTranslation()
{
  return ToVector2(mTransform->GetLocalTranslation());
}

//**************************************************************************************************
void UiWidget::SetLocalTranslation(Vec2Param translation)
{
  float snapSize = GetSnapSize();

  Vec3 localTranslation = mTransform->GetLocalTranslation();

  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(mTransform, "Translation", localTranslation);

  Vec3 newPos = Vec3(Snap(translation, snapSize), localTranslation.z);

  mTransform->SetLocalTranslation(newPos);
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldTranslation()
{
  return ToVector2(mTransform->GetWorldTranslation());
}

//**************************************************************************************************
void UiWidget::SetWorldTranslation(Vec2Param worldTranslation)
{
  Vec2 localTranslation = worldTranslation;
  if (mParent)
    localTranslation = mParent->TransformPointInverse(worldTranslation);

  if (OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, "LocalTranslation", localTranslation);

  SetLocalTranslation(localTranslation);
}

//**************************************************************************************************
Vec2 UiWidget::TransformPoint(Vec2Param localPosition)
{
  return ToVector2(mTransform->TransformPoint(Vec3(localPosition)));
}

//**************************************************************************************************
Vec2 UiWidget::TransformPointInverse(Vec2Param worldPosition)
{
  return ToVector2(mTransform->TransformPointInverse(Vec3(worldPosition)));
}

//**************************************************************************************************
Vec2 UiWidget::GetSize()
{
  return mArea->GetSize();
}

//**************************************************************************************************
void UiWidget::SetSize(Vec2Param size)
{
  if(OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(mArea, "Size", GetSize());

  mArea->SetSize(Snap(size, GetSnapSize()));

  // We don't need to call MarkAsNeeds update as we will respond to the 
  // Events::AreaChanged event when setting the size on Area
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalTopLeft()
{
  return GetLocalTranslation() + Vec2(0, GetSize().y);
}

//**************************************************************************************************
void UiWidget::SetLocalTopLeft(Vec2Param localTopLeft)
{
  Vec2 translation = localTopLeft - Vec2(0, GetSize().y);
  SetLocalTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldTopLeft()
{
  return GetWorldTranslation() + Vec2(0, GetSize().y);
}

//**************************************************************************************************
void UiWidget::SetWorldTopLeft(Vec2Param worldTopLeft)
{
  Vec2 translation = worldTopLeft - Vec2(0, GetSize().y);
  SetWorldTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalTopRight()
{
  return GetLocalTranslation() + GetSize();
}

//**************************************************************************************************
void UiWidget::SetLocalTopRight(Vec2Param localTopRight)
{
  Vec2 translation = localTopRight - GetSize();
  SetLocalTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldTopRight()
{
  return GetWorldTranslation() + GetSize();
}

//**************************************************************************************************
void UiWidget::SetWorldTopRight(Vec2Param worldTopRight)
{
  Vec2 translation = worldTopRight - GetSize();
  SetWorldTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalBottomLeft()
{
  return GetLocalTranslation();
}

//**************************************************************************************************
void UiWidget::SetLocalBottomLeft(Vec2Param localBottomLeft)
{
  SetLocalTranslation(localBottomLeft);
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldBottomLeft()
{
  return GetWorldTranslation();
}

//**************************************************************************************************
void UiWidget::SetWorldBottomLeft(Vec2Param worldBottomLeft)
{
  SetWorldTranslation(worldBottomLeft);
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalBottomRight()
{
  return GetLocalTranslation() + Vec2(GetSize().x, 0);
}

//**************************************************************************************************
void UiWidget::SetLocalBottomRight(Vec2Param localBottomRight)
{
  Vec2 translation = localBottomRight - Vec2(GetSize().x, 0);
  SetLocalTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldBottomRight()
{
  return GetWorldTranslation() + Vec2(GetSize().x, 0);
}

//**************************************************************************************************
void UiWidget::SetWorldBottomRight(Vec2Param worldBottomRight)
{
  Vec2 translation = worldBottomRight - Vec2(GetSize().x, 0);
  SetWorldTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetLocalCenter()
{
  return GetLocalTranslation() + Snap(GetSize() * 0.5f, GetSnapSize());
}

//**************************************************************************************************
void UiWidget::SetLocalCenter(Vec2Param localCenter)
{
  Vec2 translation = localCenter - GetSize() * 0.5f;
  SetLocalTranslation(translation);
}

//**************************************************************************************************
Vec2 UiWidget::GetWorldCenter()
{
  return GetWorldTranslation() + Snap(GetSize() * 0.5f, GetSnapSize());
}

//**************************************************************************************************
void UiWidget::SetWorldCenter(Vec2Param worldCenter)
{
  Vec2 translation = worldCenter - GetSize() * 0.5f;
  SetWorldTranslation(translation);
}

//**************************************************************************************************
float UiWidget::GetLocalTop()
{
  return GetLocalTopRight().y;
}

//**************************************************************************************************
void UiWidget::SetLocalTop(float localTop)
{
  Vec2 topRight = GetLocalTopRight();
  topRight.y = localTop;
  SetLocalTopRight(topRight);
}

//**************************************************************************************************
float UiWidget::GetWorldTop()
{
  return GetWorldTopRight().y;
}

//**************************************************************************************************
void UiWidget::SetWorldTop(float worldTop)
{
  Vec2 topRight = GetWorldTopRight();
  topRight.y = worldTop;
  SetWorldTopRight(topRight);
}

//**************************************************************************************************
float UiWidget::GetLocalRight()
{
  return GetLocalTopRight().x;
}

//**************************************************************************************************
void UiWidget::SetLocalRight(float localRight)
{
  Vec2 topRight = GetLocalTopRight();
  topRight.x = localRight;
  SetLocalTopRight(topRight);
}

//**************************************************************************************************
float UiWidget::GetWorldRight()
{
  return GetWorldTopRight().x;
}

//**************************************************************************************************
void UiWidget::SetWorldRight(float worldRight)
{
  Vec2 topRight = GetWorldTopRight();
  topRight.x = worldRight;
  SetWorldTopRight(topRight);
}

//**************************************************************************************************
float UiWidget::GetLocalBottom()
{
  return GetLocalBottomLeft().y;
}

//**************************************************************************************************
void UiWidget::SetLocalBottom(float localBottom)
{
  Vec2 bottomLeft = GetLocalBottomLeft();
  bottomLeft.y = localBottom;
  SetLocalBottomLeft(bottomLeft);
}

//**************************************************************************************************
float UiWidget::GetWorldBottom()
{
  return GetWorldBottomLeft().y;
}

//**************************************************************************************************
void UiWidget::SetWorldBottom(float worldBottom)
{
  Vec2 bottomLeft = GetWorldBottomLeft();
  bottomLeft.y = worldBottom;
  SetWorldBottomLeft(bottomLeft);
}

//**************************************************************************************************
float UiWidget::GetLocalLeft()
{
  return GetLocalBottomLeft().x;
}

//**************************************************************************************************
void UiWidget::SetLocalLeft(float localLeft)
{
  Vec2 bottomLeft = GetLocalBottomLeft();
  bottomLeft.x = localLeft;
  SetLocalBottomLeft(bottomLeft);
}

//**************************************************************************************************
float UiWidget::GetWorldLeft()
{
  return GetWorldBottomLeft().x;
}

//**************************************************************************************************
void UiWidget::SetWorldLeft(float worldLeft)
{
  Vec2 bottomLeft = GetWorldBottomLeft();
  bottomLeft.x = worldLeft;
  SetWorldBottomLeft(bottomLeft);
}

//**************************************************************************************************
void UiWidget::OnAreaChanged(Event* e)
{
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
float UiWidget::GetSnapSize()
{
  float snapSize = cDefaultSnapSize;
  //if(UiRootWidget* rootWidget = GetRootWidget())
  //{
  //  snapSize = rootWidget->mSnapSize;
  //  if(snapSize == 0.0f)
  //    snapSize = 0.0001f;
  //}

  return snapSize;
}

//**************************************************************************************************
void UiWidget::MarkAsNeedsUpdate()
{
  MarkAsNeedsUpdateInternal(true);
}

//**************************************************************************************************
void UiWidget::MarkAsNeedsUpdateInternal(bool localUpdate)
{
  // If we weren't already marked for modification, we need to walk up the tree and mark
  // our parents for modification
  if(mTransformUpdateState == UiTransformUpdateState::Updated)
  {
    // Set the flag accordingly
    if(localUpdate)
      mTransformUpdateState = UiTransformUpdateState::LocalUpdate;
    else
      mTransformUpdateState = UiTransformUpdateState::ChildUpdate;

    // We want our parents to be marked as a child update
    // FIXME - Why??
    if(mParent)
      mParent->MarkAsNeedsUpdateInternal(false);
  }
  // Only let LocalUpdates override ChildUpdates
  // FIXME - WHYYYYYY???
  else if(localUpdate)
  {
    mTransformUpdateState = UiTransformUpdateState::LocalUpdate;
  }
}

//**************************************************************************************************
void UiWidget::UpdateTransform(UiTransformUpdateEvent* e)
{
  // Skip this if we're already on our way out
  if(GetOwner()->GetMarkedForDestruction())
    return;

  // Currently, BottomLeft is the only thing supported by the widget system
  mArea->SetOrigin(Location::BottomLeft);

  // Until the TransformUpdateState is fully functional, we should always update
  bool alwaysUpdate = true;
  if(mTransformUpdateState != UiTransformUpdateState::Updated || alwaysUpdate)
  {
    // Send the pre-update
    GetOwner()->DispatchEvent(Events::PreTransformUpdate, e);

    // Update our layout if it exists
    if(UiLayout* layout = GetOwner()->has(UiLayout))
    {
      Rectangle layoutData = Rectangle::PointAndSize(Vec2::cZero, GetSize());
      layout->DoLayout(layoutData, e);
    }
    else
    {
      // Update our active children
      forRange(UiWidget& child, GetChildren())
      {
        if(child.GetActive())
          child.UpdateTransform(e);
      }
    }

    // Send the post-update
    GetOwner()->DispatchEvent(Events::PostTransformUpdate, e);

    // We're now fully updated
    mTransformUpdateState = UiTransformUpdateState::Updated;
  }
}

//**************************************************************************************************
bool ParentSelected(Cog* selected, Cog* cog)
{
  while(cog)
  {
    if(selected == cog)
      return true;

    cog = cog->GetParent();
  }

  return false;
}

//**************************************************************************************************
void UiWidget::MoveChildrenToFront(float& currWorldDepth, float amount)
{
  // Recurse down
  forRange(UiWidget& childWidget, mChildren.All())
  {
    Cog* selectedObject = GetRootWidget()->mDebugSelectedWidget.GetOwner();
    bool parentSelected = ParentSelected(selectedObject, childWidget.GetOwner());

    Vec3 pos = childWidget.mTransform->GetWorldTranslation();
    float desiredPos = currWorldDepth;
    if(parentSelected)
      desiredPos += amount * 225.0f;
    pos.z = pos.z * 0.6f + desiredPos * 0.4f;
    childWidget.mTransform->SetWorldTranslation(pos);

    Vec3 local = childWidget.mTransform->GetLocalTranslation();
    local.z = Math::Max(local.z, amount);
    childWidget.mTransform->SetLocalTranslation(local);

    currWorldDepth += amount;

    // Recurse
    childWidget.MoveChildrenToFront(currWorldDepth, amount);
  }
}

//**************************************************************************************************
void UiWidget::TakeFocus()
{
  if(UiRootWidget* rootWidget = GetRootWidget())
    rootWidget->RootChangeFocus(this);
}

//**************************************************************************************************
void UiWidget::LoseFocus()
{
  if(UiRootWidget* rootWidget = GetRootWidget())
    rootWidget->RootChangeFocus(nullptr);
}

//**************************************************************************************************
bool UiWidget::TabJump(KeyboardEvent* e)
{
  if(e->Key == Keys::Tab)
  {
    if(e->ShiftPressed)
      TabJumpDirection(UiFocusDirection::Backwards);
    else
      TabJumpDirection(UiFocusDirection::Forward);

    e->Handled = true;
    return true;
  }

  return false;
}

//**************************************************************************************************
void UiWidget::TabJumpDirection(UiFocusDirection::Enum direction)
{
  FindNextFocus(this, direction);
}

//**************************************************************************************************
UiSizePolicy::Enum UiWidget::GetSizePolicy(Axis::Enum axis)
{
  return mSizePolicy[axis];
}

//**************************************************************************************************
UiSizePolicy::Enum UiWidget::GetSizePolicyX()
{
  return mSizePolicy[Axis::X];
}

//**************************************************************************************************
UiSizePolicy::Enum UiWidget::GetSizePolicyY()
{
  return mSizePolicy[Axis::Y];
}

//**************************************************************************************************
void UiWidget::SetSizePolicy(Axis::Enum axis, UiSizePolicy::Enum policy)
{
  mSizePolicy[axis] = policy;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::SetSizePolicyX(UiSizePolicy::Enum policy)
{
  if (OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, PropertyPath("Size"), GetSize());

  mSizePolicy[Axis::X] = policy;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::SetSizePolicyY(UiSizePolicy::Enum policy)
{
  if (OperationQueue::IsListeningForSideEffects())
    OperationQueue::RegisterSideEffect(this, PropertyPath("Size"), GetSize());

  mSizePolicy[Axis::Y] = policy;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
bool UiWidget::GetInLayout()
{
  return mFlags.IsSet(UiWidgetFlags::InLayout);
}

//**************************************************************************************************
void UiWidget::SetInLayout(bool state)
{
  // If the user enables in layout, hitting undo should move the translation and scale back
  // to their previous values
  if (OperationQueue::IsListeningForSideEffects())
  {
    OperationQueue::RegisterSideEffect(this, PropertyPath("LocalTranslation"), GetLocalTranslation());
    OperationQueue::RegisterSideEffect(this, PropertyPath("Size"), GetSize());
  }

  mFlags.SetState(UiWidgetFlags::InLayout, state);
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
Vec2 UiWidget::GetFlexSize()
{
  return mFlexSize;
}

//**************************************************************************************************
void UiWidget::SetFlexSize(Vec2Param flexSize)
{
  mFlexSize = flexSize;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
Vec2 UiWidget::GetAbsoluteMinSize()
{
  return mAbsoluteMinSize;
}

//**************************************************************************************************
void UiWidget::SetAbsoluteMinSize(Vec2Param minSize)
{
  mAbsoluteMinSize = minSize;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
UiVerticalAlignment::Enum UiWidget::GetVerticalAlignment()
{
  return mVerticalAlignment;
}

//**************************************************************************************************
void UiWidget::SetVerticalAlignment(UiVerticalAlignment::Enum alignment)
{
  mVerticalAlignment = alignment;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
UiHorizontalAlignment::Enum UiWidget::GetHorizontalAlignment()
{
  return mHorizontalAlignment;
}

//**************************************************************************************************
void UiWidget::SetHorizontalAlignment(UiHorizontalAlignment::Enum alignment)
{
  mHorizontalAlignment = alignment;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
UiDockMode::Enum UiWidget::GetDockMode()
{
  return mDockMode;
}

//**************************************************************************************************
void UiWidget::SetDockMode(UiDockMode::Enum dockMode)
{
  mDockMode = dockMode;
  Transform::sCacheWorldMatrices = !Transform::sCacheWorldMatrices;
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
const Thickness& UiWidget::GetMargins()
{
  return mMargins;
}

//**************************************************************************************************
float UiWidget::GetMarginLeft()
{
  return mMargins.Left;
}

//**************************************************************************************************
float UiWidget::GetMarginTop()
{
  return mMargins.Top;
}

//**************************************************************************************************
float UiWidget::GetMarginRight()
{
  return mMargins.Right;
}

//**************************************************************************************************
float UiWidget::GetMarginBottom()
{
  return mMargins.Bottom;
}

//**************************************************************************************************
void UiWidget::SetMarginLeft(float val)
{
  mMargins.Left = Snap(val, GetSnapSize());
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::SetMarginTop(float val)
{
  mMargins.Top = Snap(val, GetSnapSize());
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::SetMarginRight(float val)
{
  mMargins.Right = Snap(val, GetSnapSize());
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::SetMarginBottom(float val)
{
  mMargins.Bottom = Snap(val, GetSnapSize());
  MarkAsNeedsUpdate();
}

//**************************************************************************************************
void UiWidget::OnPropertyModified(PropertyEvent* e)
{
  BoundType* modifiedType = e->mObject.StoredType;
  if(modifiedType->IsA(ZilchTypeId(Transform)))
    SetLocalTranslation(GetLocalTranslation());
  else if (modifiedType->IsA(ZilchTypeId(Area)))
    SetSize(GetSize());
}

//**************************************************************************************************
void FindNextFocus(UiWidget* widget, UiFocusDirection::Enum direction)
{
  while(widget != NULL)
  {
    // Get the next widget in the given direction
    if(direction == UiFocusDirection::Forward)
      widget = widget->GetNextInHierarchyOrder();
    else
      widget = widget->GetPreviousInHierarchyOrder();

    if(widget && widget->GetActive() && widget->GetCanTakeFocus())
    {
      widget->TakeFocus();
      return;
    }
  }
}

}//namespace Zero
