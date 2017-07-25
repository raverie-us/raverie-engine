///////////////////////////////////////////////////////////////////////////////
///
/// \file Hierarchy.cpp
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(Attached);
  DefineEvent(Detached);
  DefineEvent(ChildAttached);
  DefineEvent(ChildDetached);
  DefineEvent(ChildrenOrderChanged);
}// namespace Events

//---------------------------------------------------------------------------------- Hierarchy Event
ZilchDefineType(HierarchyEvent, builder, type)
{
  ZeroBindDocumented();
  ZeroBindEvent(Events::Attached, HierarchyEvent);
  ZeroBindEvent(Events::Detached, HierarchyEvent);
  ZeroBindEvent(Events::ChildAttached, HierarchyEvent);
  ZeroBindEvent(Events::ChildDetached, HierarchyEvent);
  ZeroBindEvent(Events::ChildrenOrderChanged, Event);
  ZilchBindFieldProperty(Parent);
  ZilchBindFieldProperty(Child);
}

ZilchDefineType(HierarchyComposition, builder, type)
{
}

//---------------------------------------------------------------------------------------- Hierarchy
//-------------------------------------------------------------------- Hierarchy
ZilchDefineType(Hierarchy, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZilchBindGetter(Children);

  type->AddAttribute(ObjectAttributes::cHidden);
  type->Add(new HierarchyComposition());
}

//**************************************************************************************************
Hierarchy::Hierarchy()
{

}

//**************************************************************************************************
Hierarchy::~Hierarchy()
{
  HierarchyList::range range = Children.All();
  while(!range.Empty())
  {
    Cog* current = &range.Front();
    range.PopFront();

    //Queue up cog for destruction.
    current->Destroy();

    //Destroy all the components directly so components are destroyed
    //in depth first order (children then parents) but do not
    //destroy the cog because the factory destroy list has a direct
    //pointer to the cog.
    current->DeleteComponents();
  }
}

//**************************************************************************************************
void Hierarchy::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    CogSerialization::SaveHierarchy(stream, this);
  }
  else
  {
    CogCreationContext* context = static_cast<CogCreationContext*>(stream.GetSerializationContext());
    CogSerialization::LoadHierarchy(stream, context, this);
  }
}

//**************************************************************************************************
void Hierarchy::Initialize(CogInitializer& initializer)
{
  HierarchyList::range r = Children.All();
  while(!r.Empty())
  {
    initializer.mParent = this->GetOwner();

    Cog* cog = &r.Front();
    cog->Initialize(initializer);
    r.PopFront();
  }

  initializer.mParent = NULL;
}

//**************************************************************************************************
void Hierarchy::OnAllObjectsCreated(CogInitializer& initializer)
{
  //Do not need to call children handled by CogInitializer CreationList
}

//**************************************************************************************************
void Hierarchy::OnDestroy(uint flags)
{
  HierarchyList::range range = Children.All();
  while(!range.Empty())
  {
    Cog* current = &range.Front();
    range.PopFront();
    
    current->OnDestroy();
  }
}

//**************************************************************************************************
void Hierarchy::TransformUpdate(TransformUpdateInfo& info)
{
  HierarchyList::range r = Children.All();
  while(!r.Empty())
  {
    Cog* cog = &r.Front();
    cog->TransformUpdate(info);
    r.PopFront();
  }
}

//**************************************************************************************************
void Hierarchy::AttachTo(AttachmentInfo& info)
{
  HierarchyList::range children = Children.All();
  for(; !children.Empty(); children.PopFront())
  {
    Cog* child = &children.Front();
    Cog::ComponentRange range = child->GetComponents();
    for(;!range.Empty();range.PopFront())
      range.Front()->AttachTo(info);
  }
}

//**************************************************************************************************
void Hierarchy::Detached(AttachmentInfo& info)
{
  HierarchyList::range children = Children.All();
  for(; !children.Empty(); children.PopFront())
  {
    Cog* child = &children.Front();
    Cog::ComponentRange range = child->GetComponents();
    for(;!range.Empty();range.PopFront())
      range.Front()->Detached(info);
  }
}

//**************************************************************************************************
HierarchyList::range Hierarchy::GetChildren()
{
  return Children.All();
}

//**************************************************************************************************
void Hierarchy::DestroyChildren()
{
  HierarchyList::range range = Children.All();
  while(!range.Empty())
  {
    Cog* current = &range.Front();
    range.PopFront();
    current->Destroy();
  }
}

//---------------------------------------------------------------------------- Hierarchy Composition
//**************************************************************************************************
HierarchyComposition::HierarchyComposition() : MetaComposition(ZilchTypeId(Cog))
{
  
}

//**************************************************************************************************
uint HierarchyComposition::GetComponentCount(HandleParam instance)
{
  Hierarchy* hierarchy = instance.Get<Hierarchy*>(GetOptions::AssertOnNull);
  uint count = 0;
  forRange(Cog& child, hierarchy->GetChildren())
    ++count;
  return count;
}

//**************************************************************************************************
Handle HierarchyComposition::GetComponentAt(HandleParam instance, uint index)
{
  Hierarchy* hierarchy = instance.Get<Hierarchy*>(GetOptions::AssertOnNull);
  uint currIndex = 0;
  forRange(Cog& child, hierarchy->GetChildren())
  {
    if(currIndex == index)
      return Handle(&child);
    ++currIndex;
  }

  return Handle();
}

//**************************************************************************************************
bool HierarchyComposition::CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info)
{
  if (typeToAdd == ZilchTypeId(Cog))
    return true;
  info->Reason = "Only Cogs can be added to Hierarchy";
  return false;
}

//**************************************************************************************************
void RelativeAttach(Transform* child, Transform* parent)
{
  Mat4 childMatrix = child->GetWorldMatrix();
  Mat4 parentMatrix = parent->GetWorldMatrix();

  parentMatrix.Invert();

  Mat4 local = parentMatrix * childMatrix;

  Vec3 scale;
  Vec3 shear;
  Mat3 rotation;
  Vec3 translation;

  local.Decompose(&scale, &shear, &rotation, & translation);

  child->SetRotation(Math::ToQuaternion(rotation));
  child->SetScale(scale);
  child->SetTranslation(translation);
  child->GetOwner()->AttachTo(parent->GetOwner());
}

}//namespace Zero
