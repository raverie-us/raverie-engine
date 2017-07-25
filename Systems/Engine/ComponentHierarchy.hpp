///////////////////////////////////////////////////////////////////////////////
///
/// \file ComponentHierarchy.cpp
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------- Component Hierarchy Base
/// Needed for an intrusive link
template <typename ComponentType>
class ComponentHierarchyBase : public Component
{
public:
  IntrusiveLink(ComponentHierarchyBase<ComponentType>, HierarchyLink);
};

//---------------------------------------------------------- Component Hierarchy
/// Maintains a hierarchy of specific Components. When a hierarchy of Components
/// is required, this removes the need to manually manage the hierarchy, and
/// also removes the overhead of component lookups while iterating over the
/// hierarchy.
template <typename ComponentType>
class ComponentHierarchy : public ComponentHierarchyBase<ComponentType>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Typedefs.
  typedef ComponentHierarchy<ComponentType> self_type;
  typedef ComponentHierarchyBase<ComponentType> BaseType;
  typedef BaseInList<BaseType, ComponentType, &BaseType::HierarchyLink> ChildList;
  typename typedef ChildList::range ChildListRange;
  typename typedef ChildList::reverse_range ChildListReverseRange;

  /// Constructor.
  ComponentHierarchy() : mParent(nullptr) {}

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  void OnDestroy(uint flags = 0) override;

  /// When a child is attached or detached, we need to update our child list.
  void OnChildAttached(HierarchyEvent* e);
  void OnChildDetached(HierarchyEvent* e);
  void OnChildrenOrderChanged(Event* e);

  /// Tree traversal helpers.
  ComponentType* GetPreviousSibling();
  ComponentType* GetNextSibling();
  ComponentType* GetLastChild();
  ComponentType* GetNextInHierarchyOrder();
  ComponentType* GetPreviousInHierarchyOrder();
  ComponentType* GetRoot();

  typename ChildListRange GetChildren(){return mChildren.All();}
  typename ChildListReverseRange GetChildrenReverse() { return mChildren.ReverseAll(); }

  ComponentType* mParent;
  ChildList mChildren;
};

//******************************************************************************
template <typename ComponentType>
ZilchDefineType(ComponentHierarchy<ComponentType>, builder, type)
{
  ZilchBindGetterProperty(PreviousSibling);
  ZilchBindGetterProperty(NextSibling);
  ZilchBindGetterProperty(LastChild);
  ZilchBindGetterProperty(NextInHierarchyOrder);
  ZilchBindGetterProperty(PreviousInHierarchyOrder);
  ZilchBindGetterProperty(Root);
  
  // Temporarily unbound until an issue is fixed
  //InitMetaRangeAdapter(ChildListRange);
  //ZilchBindMethod(GetChildren);
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::Initialize(CogInitializer& initializer)
{
  // Add ourself to our parent
  if(Cog* parent = GetOwner()->GetParent())
  {
    if(ComponentType* component = parent->has(ComponentType))
    {
      mParent = component;
      mParent->mChildren.PushBack(this);
    }
  }

  ConnectThisTo(GetOwner(), Events::ChildAttached, OnChildAttached);
  ConnectThisTo(GetOwner(), Events::ChildDetached, OnChildDetached);
  ConnectThisTo(GetOwner(), Events::ChildrenOrderChanged, OnChildrenOrderChanged);
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildAttached(HierarchyEvent* e)
{
  if(e->Parent != GetOwner())
    return;

  if(ComponentType* component = e->Child->has(ComponentType))
    mChildren.PushBack(component);
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildDetached(HierarchyEvent* e)
{
  if(e->Parent != GetOwner())
    return;

  if(ComponentType* component = e->Child->has(ComponentType))
    mChildren.Erase(component);
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnChildrenOrderChanged(Event* e)
{
  // The event isn't currently sent information as to which object moved
  // where, so we're just going to re-build the child list
  mChildren.Clear();

  forRange(Cog& child, GetOwner()->GetChildren())
  {
    if(ComponentType* childComponent = child.has(ComponentType))
      mChildren.PushBack(childComponent);
  }
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::AttachTo(AttachmentInfo& info)
{
  if(info.Child == GetOwner())
    mParent = info.Parent->has(ComponentType);
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::Detached(AttachmentInfo& info)
{
  if(info.Child == GetOwner())
    mParent = nullptr;
}

//******************************************************************************
template <typename ComponentType>
void ComponentHierarchy<ComponentType>::OnDestroy(uint flags)
{
  if(mParent)
    mParent->mChildren.Erase(this);
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetPreviousSibling()
{
  ComponentType* prev = (ComponentType*)ChildList::Prev(this);
  if(mParent && prev != mParent->mChildren.End())
    return prev;
  return nullptr;
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetNextSibling()
{
  ComponentType* next = (ComponentType*)ChildList::Next(this);
  if(mParent && next != mParent->mChildren.End())
    return next;
  return nullptr;
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetLastChild()
{
  if(!mChildren.Empty())
    return mChildren.Back().GetLastChild();
  else
    return (ComponentType*)this;
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetNextInHierarchyOrder()
{
  // If this object has children return the first child
  if(!mChildren.Empty())
    return &mChildren.Front();

  // Return next sibling if there is one
  ComponentType* nextSibling = GetNextSibling();
  if(nextSibling)
    return nextSibling;

  // Loop until the root or a parent has a sibling
  ComponentType* parent = mParent;
  while(parent != nullptr)
  {
    ComponentType* parentSibling = parent->GetNextSibling();
    if(parentSibling)
      return parentSibling;
    else
      parent = parent->mParent;
  }
  return nullptr;
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetPreviousInHierarchyOrder()
{
  // Get prev sibling if there is one
  ComponentType* prevSibling = GetPreviousSibling();

  // If this is first node of a child it is previous node
  if(prevSibling == nullptr)
    return mParent;

  // Return the last child of the sibling
  return prevSibling->GetLastChild();
}

//******************************************************************************
template <typename ComponentType>
ComponentType* ComponentHierarchy<ComponentType>::GetRoot()
{
  ComponentType* curr = (ComponentType*)this;
  while(curr->mParent)
    curr = curr->mParent;
  return curr;
}

}//namespace Zero
