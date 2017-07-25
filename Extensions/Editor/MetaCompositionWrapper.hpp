////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------------------------------- Meta Composition
class MetaCompositionWrapper : public MetaComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MetaCompositionWrapper(BoundType* typeToWrap) : 
    MetaComposition(nullptr)
  {
    mContainedComposition = typeToWrap->HasInherited<MetaComposition>();
    mComponentType = mContainedComposition->mComponentType;
    mSupportsComponentRemoval = mContainedComposition->mSupportsComponentRemoval;
    mSupportsComponentReorder = mContainedComposition->mSupportsComponentReorder;
  }

  uint GetComponentCount(HandleParam owner) override
  {
    return mContainedComposition->GetComponentCount(owner);
  }

  bool HasComponent(HandleParam owner, BoundType* componentType) override
  {
    return mContainedComposition->HasComponent(owner, componentType);
  }

  Handle GetComponent(HandleParam owner, BoundType* componentType) override
  {
    return mContainedComposition->GetComponent(owner, componentType);
  }

  Handle GetComponentAt(HandleParam owner, uint index) override
  {
    return mContainedComposition->GetComponentAt(owner, index);
  }

  Handle GetComponentUniqueId(HandleParam owner, u64 uniqueId) override
  {
    return mContainedComposition->GetComponentUniqueId(owner, uniqueId);
  }

  uint GetComponentIndex(HandleParam owner, BoundType* componentType) override
  {
    return mContainedComposition->GetComponentIndex(owner, componentType);
  }

  uint GetComponentIndex(HandleParam owner, HandleParam component) override
  {
    return mContainedComposition->GetComponentIndex(owner, component);
  }

  void Enumerate(Array<BoundType*>& addTypes, EnumerateAction::Enum action, HandleParam owner = nullptr) override
  {
    mContainedComposition->Enumerate(addTypes, action, owner);
  }

  bool CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info = nullptr) override
  {
    return mContainedComposition->CanAddComponent(owner, typeToAdd, info);
  }

  Handle MakeObject(BoundType* typeToCreate) override
  {
    return mContainedComposition->MakeObject(typeToCreate);
  }

  void AddComponent(HandleParam owner, HandleParam component, int index = -1, bool ignoreDependencies = false) override
  {
    mContainedComposition->AddComponent(owner, component, index, ignoreDependencies);
  }

  void AddComponent(HandleParam owner, BoundType* componentType, int index = -1, bool ignoreDependencies = false) override
  {
    mContainedComposition->AddComponent(owner, componentType, index, ignoreDependencies);
  }

  bool CanRemoveComponent(HandleParam owner, HandleParam component, String& reason) override
  {
    return mContainedComposition->CanRemoveComponent(owner, component, reason);
  }

  void RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies = false) override
  {
    mContainedComposition->RemoveComponent(owner, component, ignoreDependencies);
  }

  void MoveComponent(HandleParam owner, HandleParam component, uint destination) override
  {
    mContainedComposition->MoveComponent(owner, component, destination);
  }

  String GetAddName() override
  {
    return mContainedComposition->GetAddName();
  }

  // We want to cache this so we aren't constantly looking it up.
  MetaComposition* mContainedComposition;
};

}//namespace Zero
