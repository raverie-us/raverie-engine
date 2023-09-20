// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Meta Creation Context
/// See comment above MetaCreationContext.
struct CogMetaCreationContext : public MetaCreationContext
{
  ~CogMetaCreationContext();

  /// Finds an Initializer for the given space. If it has not yet been created,
  /// it will be created for you.
  CogInitializer* GetInitializer(Space* space);
  HashMap<Space*, CogInitializer*> mInitializers;
};

// Cog Meta Composition
class CogMetaComposition : public MetaComposition
{
public:
  RaverieDeclareType(CogMetaComposition, TypeCopyMode::ReferenceType);
  CogMetaComposition();

  /// MetaComposition Interface.
  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
  bool CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info = nullptr) override;
  Handle MakeObject(BoundType* typeToCreate) override;
  BoundType* MakeProxy(StringParam typeName, ProxyReason::Enum reason) override;

  MetaCreationContext* GetCreationContext() override;
  void AddComponent(HandleParam owner, HandleParam component, int index = -1, bool ignoreDependencies = false, MetaCreationContext* creationContext = nullptr) override;
  void FinalizeCreation(MetaCreationContext* context) override;

  bool CanRemoveComponent(HandleParam owner, HandleParam component, String& reason) override;
  void RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies = false) override;
  void MoveComponent(HandleParam owner, HandleParam component, uint destination) override;
};

} // namespace Raverie
