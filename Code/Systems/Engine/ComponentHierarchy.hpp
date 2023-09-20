// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Needed for an intrusive link
template <typename ComponentType>
class ComponentHierarchyBase : public Component
{
public:
  IntrusiveLink(ComponentHierarchyBase<ComponentType>, HierarchyLink);
};

/// Maintains a hierarchy of specific Components. When a hierarchy of Components
/// is required, this removes the need to manually manage the hierarchy, and
/// also removes the overhead of component lookups while iterating over the
/// hierarchy.
template <typename ComponentType>
class ComponentHierarchy : public ComponentHierarchyBase<ComponentType>
{
public:
  RaverieDeclareType(ComponentHierarchy, TypeCopyMode::ReferenceType);

  // Typedefs.
  typedef ComponentHierarchy<ComponentType> self_type;
  typedef ComponentHierarchyBase<ComponentType> base_type;
  typedef BaseInList<base_type, ComponentType, &base_type::HierarchyLink> ChildList;
  typedef typename ChildList::range ChildListRange;
  typedef typename ChildList::reverse_range ChildListReverseRange;

  /// Constructor.
  ComponentHierarchy();
  ~ComponentHierarchy();

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;

  /// When a child is attached or detached, we need to update our child list.
  void OnChildAttached(HierarchyEvent* e);
  void OnChildDetached(HierarchyEvent* e);
  void OnChildrenOrderChanged(Event* e);

  /// Tree traversal helpers.
  ComponentType* GetPreviousSibling();
  ComponentType* GetNextSibling();
  ComponentType* GetLastDirectChild();
  ComponentType* GetLastDeepestChild();
  ComponentType* GetNextInHierarchyOrder();
  ComponentType* GetPreviousInHierarchyOrder();
  ComponentType* GetRoot();

  /// Returns whether or not we are a descendant of the given Cog.
  bool IsDescendantOf(ComponentType& ancestor);

  /// Returns whether or not we are an ancestor of the given Cog.
  bool IsAncestorOf(ComponentType& descendant);

  ChildListRange GetChildren()
  {
    return mChildren.All();
  }
  ChildListReverseRange GetChildrenReverse()
  {
    return mChildren.ReverseAll();
  }

  /// Returns the amount of children. Note that this function has to iterate
  /// over all children to calculate the count.
  uint GetChildCount();

  ComponentType* mParent;
  ChildList mChildren;
};

template <typename ComponentType>
RaverieDefineType(ComponentHierarchy<ComponentType>, builder, type)
{
  RaverieBindDocumented();
  RaverieBindGetter(PreviousSibling);
  RaverieBindGetter(NextSibling);
  RaverieBindGetter(LastDirectChild);
  RaverieBindGetter(LastDeepestChild);
  RaverieBindGetter(NextInHierarchyOrder);
  RaverieBindGetter(PreviousInHierarchyOrder);
  RaverieBindFieldGetter(mParent);
  RaverieBindGetter(Root);
  RaverieBindGetter(ChildCount);

  RaverieBindMethod(GetChildren);
  RaverieBindMethod(IsDescendantOf);
  RaverieBindMethod(IsAncestorOf);
}

template <typename ComponentType>
ComponentHierarchy<ComponentType>::ComponentHierarchy() : mParent(nullptr)
{
}

template <typename ComponentType>
ComponentHierarchy<ComponentType>::~ComponentHierarchy()
{
  forRange (ComponentType& child, GetChildren())
    child.mParent = nullptr;

  if (mParent)
    mParent->mChildren.Erase(this);
  mChildren.Clear();
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::Initialize(CogInitializer& initializer)
{
  // Add ourself to our parent
  if (Cog* parent = base_type::GetOwner()->GetParent())
  {
    if (ComponentType* component = parent->has(ComponentType))
    {
      mParent = component;
      mParent->mChildren.PushBack(this);
    }
  }

  // If we were dynamically added, we need to add all of our children. If we
  // aren't dynamically added, our children will add themselves to our child
  // list in their initialize
  if (initializer.Flags & CreationFlags::DynamicallyAdded)
  {
    forRange (Cog& childCog, base_type::GetOwner()->GetChildren())
    {
      if (ComponentType* child = childCog.has(ComponentType))
      {
        child->mParent = (ComponentType*)this;
        mChildren.PushBack(child);
      }
    }
  }

  ConnectThisTo(base_type::GetOwner(), Events::ChildAttached, OnChildAttached);
  ConnectThisTo(base_type::GetOwner(), Events::ChildDetached, OnChildDetached);
  ConnectThisTo(base_type::GetOwner(), Events::ChildrenOrderChanged, OnChildrenOrderChanged);
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildAttached(HierarchyEvent* e)
{
  if (e->Parent != base_type::GetOwner())
    return;

  if (ComponentType* component = e->Child->has(ComponentType))
    mChildren.PushBack(component);
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildDetached(HierarchyEvent* e)
{
  if (e->Parent != base_type::GetOwner())
    return;

  if (ComponentType* component = e->Child->has(ComponentType))
    mChildren.Erase(component);
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildrenOrderChanged(Event* e)
{
  // The event isn't currently sent information as to which object moved
  // where, so we're just going to re-build the child list
  mChildren.Clear();

  forRange (Cog& child, base_type::GetOwner()->GetChildren())
  {
    if (ComponentType* childComponent = child.has(ComponentType))
      mChildren.PushBack(childComponent);
  }
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::AttachTo(AttachmentInfo& info)
{
  if (info.Child == base_type::GetOwner())
    mParent = info.Parent->has(ComponentType);
}

template <typename ComponentType>
void ComponentHierarchy<ComponentType>::Detached(AttachmentInfo& info)
{
  if (info.Child == base_type::GetOwner())
    mParent = nullptr;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetPreviousSibling()
{
  ComponentType* prev = (ComponentType*)ChildList::Prev(this);
  if (mParent && prev != mParent->mChildren.End())
    return prev;
  return nullptr;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetNextSibling()
{
  ComponentType* next = (ComponentType*)ChildList::Next(this);
  if (mParent && next != mParent->mChildren.End())
    return next;
  return nullptr;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetLastDirectChild()
{
  if (!mChildren.Empty())
    return &mChildren.Back();
  return nullptr;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetLastDeepestChild()
{
  if (ComponentType* lastChild = GetLastDirectChild())
    return lastChild->GetLastDeepestChild();
  else
    return (ComponentType*)this;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetNextInHierarchyOrder()
{
  // If this object has children return the first child
  if (!mChildren.Empty())
    return &mChildren.Front();

  // Return next sibling if there is one
  ComponentType* nextSibling = GetNextSibling();
  if (nextSibling)
    return nextSibling;

  // Loop until the root or a parent has a sibling
  ComponentType* parent = mParent;
  while (parent != nullptr)
  {
    ComponentType* parentSibling = parent->GetNextSibling();
    if (parentSibling)
      return parentSibling;
    else
      parent = parent->mParent;
  }
  return nullptr;
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetPreviousInHierarchyOrder()
{
  // Get prev sibling if there is one
  ComponentType* prevSibling = GetPreviousSibling();

  // If this is first node of a child it is previous node
  if (prevSibling == nullptr)
    return mParent;

  // Return the last child of the sibling
  return prevSibling->GetLastDeepestChild();
}

template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetRoot()
{
  ComponentType* curr = (ComponentType*)this;
  while (curr->mParent)
    curr = curr->mParent;
  return curr;
}

template <typename ComponentType>
bool ComponentHierarchy<ComponentType>::IsDescendantOf(ComponentType& ancestor)
{
  return ancestor.IsAncestorOf(*(ComponentType*)this);
}

template <typename ComponentType>
bool ComponentHierarchy<ComponentType>::IsAncestorOf(ComponentType& descendant)
{
  ComponentType* current = &descendant;
  while (current)
  {
    current = current->mParent;
    if (current == this)
      return true;
  }

  return false;
}

template <typename ComponentType>
uint ComponentHierarchy<ComponentType>::GetChildCount()
{
  uint count = 0;
  forRange (ComponentType& child, GetChildren())
    ++count;
  return count;
}

} // namespace Raverie
