// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

bool NeedsLayout(Widget& widget)
{
  return widget.mActive && !widget.mNotInLayout;
}
FilterLayoutChildren::FilterLayoutChildren(Composite* widget)
{
  // Remove all invalid children from the end
  WidgetListRange children = widget->GetChildren();
  while (!children.Empty() && !NeedsLayout(children.Back()))
    children.PopBack();

  mChildren = children;
  SkipInvalid();
}

Widget& FilterLayoutChildren::Front()
{
  return mChildren.Front();
}

WidgetListRange FilterLayoutChildren::All()
{
  return mChildren;
}

bool FilterLayoutChildren::Empty()
{
  return mChildren.Empty();
}

void FilterLayoutChildren::PopFront()
{
  mChildren.PopFront();
  SkipInvalid();
}

void FilterLayoutChildren::SkipInvalid()
{
  while (!mChildren.Empty())
  {
    if (NeedsLayout(mChildren.Front()))
      return;

    mChildren.PopFront();
  }
}

RaverieDefineType(Composite, builder, type)
{
}

Composite::Composite(Composite* parent, AttachType::Enum attachType) : Widget(parent, attachType)
{
  mLayout = nullptr;
  mMinSize = Vec2(10, 10);
  DebugValidate();
}

Composite::~Composite()
{
  SafeDelete(mLayout);
  ErrorIf(!mChildren.Empty(),
          "Composite still has children. Something is "
          "wrong with OnDestroy!");
}

void Composite::AttachChildWidget(Widget* child, AttachType::Enum attachType)
{
  DebugValidate();
  InternalAttach(this, child);
}

void Composite::InternalDetach(Composite* parent, Widget* child)
{
  parent->DebugValidate();
  child->DebugValidate();
  ErrorIf(child->mParent != parent, "Object is not a child of the parent.");
  parent->mChildren.Erase(child);
  child->mParent = nullptr;
}

void Composite::InternalAttach(Composite* parent, Widget* child)
{
  parent->DebugValidate();
  child->DebugValidate();
  ErrorIf(child->mDestroyed, "Widget is being destroyed and is now attaching to another widget.");
  ErrorIf(parent->mDestroyed, "Widget is attaching to widget that is being destroyed");

  if (child->mParent)
    InternalDetach(child->GetParent(), child);

  // Set parent and attach
  child->mParent = parent;
  parent->mChildren.PushBack(child);

  if (child->mRootWidget != parent->mRootWidget)
    child->ChangeRoot(parent->mRootWidget);

  parent->MarkAsNeedsUpdate();
}

void Composite::ChangeRoot(RootWidget* newRoot)
{
  DebugValidate();
  newRoot->DebugValidate();
  if (mRootWidget == newRoot)
    return;

  mRootWidget = newRoot;

  WidgetListRange children = GetChildren();
  while (!children.Empty())
  {
    children.Front().ChangeRoot(newRoot);
    children.PopFront();
  }
}

void Composite::DestroyChildren()
{
  DebugValidate();
  Widget* child = mChildren.Begin();
  while (child != mChildren.End())
  {
    Widget* next = WidgetList::Next(child);
    child->DebugValidate();
    child->Destroy();
    child = next;
  }
}

void Composite::OnDestroy()
{
  DebugValidate();
  DestroyChildren();
  Widget::OnDestroy();
}

void Composite::SetLayout(Layout* layout)
{
  DebugValidate();
  SafeDelete(mLayout);
  mLayout = layout;
}

void Composite::DoLayout()
{
  DebugValidate();
  LayoutArea data;
  data.Size = mSize;
  data.HorizLimit = LimitMode::Limited;
  data.VerticalLimit = LimitMode::Limited;
  data.Offset = Vec3::cZero;
  mLayout->DoLayout(this, data);
  DebugValidate();
}

void Composite::ShiftOntoScreen(Vec3 offset)
{
  DebugValidate();
  Vec2 screenSize = this->GetParent()->GetSize();
  Vec2 thisSize = this->GetSize();

  if (offset.y + thisSize.y > screenSize.y)
    offset.y -= (offset.y + thisSize.y) - screenSize.y;

  if (offset.x + thisSize.x > screenSize.x)
    offset.x -= (offset.x + thisSize.x) - screenSize.x;

  this->SetTranslation(offset);
}

void Composite::RenderUpdate(
    ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, WidgetRect clipRect)
{
  DebugValidate();
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  if (mClipping)
  {
    WidgetRect rect; // = {mWorldTx.m30, mWorldTx.m31, mSize.x, mSize.y};
    rect.X = mWorldTx.m30;
    rect.Y = mWorldTx.m31;
    rect.SizeX = mSize.x;
    rect.SizeY = mSize.y;
    WidgetRect newRect;
    newRect.X = Math::Max(clipRect.X, rect.X);
    newRect.Y = Math::Max(clipRect.Y, rect.Y);
    newRect.SizeX = Math::Min(clipRect.X + clipRect.SizeX, rect.X + rect.SizeX) - newRect.X;
    newRect.SizeY = Math::Min(clipRect.Y + clipRect.SizeY, rect.Y + rect.SizeY) - newRect.Y;
    clipRect = newRect;
    if (clipRect.SizeX <= 0 || clipRect.SizeY <= 0)
      return;
  }

  colorTx.ColorMultiply *= mColor;

  forRange (Widget& child, GetChildren())
  {
    if (child.mActive && child.mVisible && !child.mDestroyed)
      child.RenderUpdate(viewBlock, frameBlock, mWorldTx, colorTx, clipRect);
  }
}

Widget* Composite::HitTest(Vec2 location, Widget* ignore)
{
  DebugValidate();
  // Skip inactive object
  if (InputBlocked())
    return nullptr;

  if (ignore == this)
    return nullptr;

  // Check for containment
  if (!CheckClipping(location))
    return nullptr;

  WidgetList::iterator it = mChildren.ReverseBegin();
  WidgetList::iterator end = mChildren.ReverseEnd();
  while (it != end)
  {
    Widget* child = *it;
    Widget* hit = child->HitTest(location, ignore);
    if (hit)
      return hit;
    --it;
  }

  if (!Contains(location))
    return nullptr;

  return nullptr;
}

void Composite::UpdateChildTransforms()
{
  DebugValidate();
  static const size_t MaxIterations = 1000;
  size_t counter = 0;
  forRange (auto& child, GetChildren())
  {
    if (counter++ > MaxIterations)
    {
      Error("Max child iterations reached in Composite::UpdateChildTransforms");
      return;
    }

    child.DebugValidate();

    if (!child.mDestroyed && child.GetActive())
      child.UpdateTransformExternal();
  }
}

void Composite::UpdateTransform()
{
  DebugValidate();
  if (mIsUpdatingTransform)
  {
    Widget::UpdateTransform();
    Error("Recursive update in Composite::UpdateTransform");
    return;
  }

  // Skip this if we're already on our way out
  if (mDestroyed)
    return;

  mIsUpdatingTransform = true;
  if (mTransformUpdateState == TransformUpdateState::LocalUpdate)
  {
    if (mLayout)
    {
      DoLayout();
    }
    else
    {
      UpdateChildTransforms();
    }
  }
  else if (mTransformUpdateState == TransformUpdateState::ChildUpdate)
  {
    UpdateChildTransforms();
  }

  Widget::UpdateTransform();
  mIsUpdatingTransform = false;
}

void Composite::DispatchDown(StringParam eventId, Event* event)
{
  DebugValidate();
  forRange (Widget& widget, mChildren.All())
  {
    widget.DispatchEvent(eventId, event);
    widget.DispatchDown(eventId, event);
  }
}

Vec2 Composite::GetMinSize()
{
  DebugValidate();
  LayoutArea area;
  if (mLayout && !mChildren.Empty())
  {
    return Measure(area);
  }
  else
    return mMinSize;
}

void Composite::SetMinSize(Vec2 newMin)
{
  DebugValidate();
  mMinSize = newMin;
}

Vec2 Composite::Measure(LayoutArea& data)
{
  DebugValidate();
  // If we're fixed, no need to do anything
  if (mSizePolicy.XPolicy == SizePolicy::Fixed && mSizePolicy.YPolicy == SizePolicy::Fixed)
    return mSize;

  // If there is a layout active use it
  if (mLayout)
  {
    Vec2 measureSize = mLayout->Measure(this, data);
    if (measureSize.x < mMinSize.x)
      measureSize.x = mMinSize.x;
    if (measureSize.y < mMinSize.y)
      measureSize.y = mMinSize.y;
    if (mSizePolicy.XPolicy == SizePolicy::Fixed)
      measureSize.x = mSize.x;
    if (mSizePolicy.YPolicy == SizePolicy::Fixed)
      measureSize.y = mSize.y;
    return measureSize;
  }
  else
    return Widget::Measure(data);
}

Widget* Find(StringParam name, UiTraversal::Enum traversalType, size_t& index, Widget* parent)
{
  // Get itself as a composite
  Composite* composite = parent->GetSelfAsComposite();

  // If the parent's type is actually a composite...
  if (composite != nullptr)
  {
    // Get the children range iterator
    WidgetListRange children = composite->GetChildren();

    // Loop through all the children
    while (children.Empty() == false)
    {
      // Get the current child
      Widget* child = &children.Front();

      // If the name of the child matches... (using whatever comparison we were
      // given)
      if (child->mName == name)
      {
        // If the index is zero, then we hit the one we wanted!
        if (index == 0)
        {
          // Return the child!
          return child;
        }
        else
        {
          // Keep looking, but since we found this one then decrement the index
          --index;
        }
      }

      // If we got here, then we didn't find the child yet (though we may have
      // found one of the same name, the index must be 0)
      if (traversalType == UiTraversal::DepthFirst)
      {
        Widget* result = Find(name, traversalType, index, child);

        if (result != nullptr)
        {
          return result;
        }
      }

      children.PopFront();
    }

    // If we got here, then we didn't find the child yet (though we may have
    // found one of the same name, the index must be 0)
    if (traversalType == UiTraversal::BreadthFirst)
    {
      // Loop through all the children
      while (children.Empty() == false)
      {
        // Get the current child
        Widget* child = &children.Front();

        Widget* result = Find(name, traversalType, index, child);

        if (result != nullptr)
        {
          return result;
        }

        children.PopFront();
      }
    }
  }

  return nullptr;
}

// Find any child widget by name
Widget* FindWidgetByName(StringParam name, UiTraversal::Enum traversalType, size_t index, Widget* parent)
{
  return Find(name, traversalType, index, parent);
}

// Colored Composite
ColoredComposite::ColoredComposite(Composite* parent, Vec4Param color, AttachType::Enum attachType) :
    Composite(parent, attachType)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetColor(color);
}

void ColoredComposite::UpdateTransform()
{
  mBackground->SetSize(GetSize());
  Composite::UpdateTransform();
}

} // namespace Raverie
