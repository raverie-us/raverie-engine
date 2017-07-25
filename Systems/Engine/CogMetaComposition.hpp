////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------------------- Cog Meta Composition
class CogMetaComposition : public MetaComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  CogMetaComposition();

  /// MetaComposition Interface.
  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
  bool CanAddComponent(HandleParam owner, BoundType* typeToAdd, AddInfo* info = nullptr) override;
  Handle MakeObject(BoundType* typeToCreate) override;
  BoundType* MakeProxy(StringParam typeName, ProxyReason::Enum reason) override;
  void AddComponent(HandleParam owner, HandleParam component, int index = -1,
                    bool ignoreDependencies = false) override;
  bool CanRemoveComponent(HandleParam owner, HandleParam component, String& reason) override;
  void RemoveComponent(HandleParam owner, HandleParam component,
                       bool ignoreDependencies = false) override;  
  void MoveComponent(HandleParam owner, HandleParam component, uint destination) override;
};

}//namespace Zero
