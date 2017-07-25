////////////////////////////////////////////////////////////////////////////////
///
/// \file MultiPropertyInterface.hpp
/// Declaration of the property interface for meta selections.
///
/// Authors: Joshua Claeys
/// Copyright 2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class OperationQueue;
class MetaSelection;

//------------------------------------------------------------------------------------ MultiProperty
/// This property interface handles the modification of multiple objects
/// at once, as well as queuing operations for each object modification.
/// The object given to the property view with this interface should 
/// be a Selection object.
class MultiPropertyInterface : public PropertyInterface
{
public:
  MultiPropertyInterface(OperationQueue* queue, MetaSelection* selection);

  /// PropertyInterface Interface.
  void ChangeProperty(HandleParam object, PropertyPathParam property, 
                      PropertyState& state, PropertyAction::Enum action) override;
  PropertyState GetValue(HandleParam object, PropertyPathParam property) override;
  void InvokeFunction(HandleParam object, Function* method) override;
  HandleOf<MetaComposition> GetMetaComposition(BoundType* objectType) override;
  ObjectPropertyNode* BuildObjectTree(ObjectPropertyNode* parent, HandleParam instance, Property* objectProperty = nullptr) override;
  void GetObjects(HandleParam instance, Array<Handle>& objects) override;

  MetaSelection* mSelection;

private:
  friend class MultiMetaComposition;

  /// Finds a shared meta of all objects in the selection. Can return NULL
  /// if no meta is shared at all.
  BoundType* GetTargetType();

  void Undo() override;
  void Redo() override;
  void CaptureState(PropertyStateCapture& capture, HandleParam object, Property* property) override;

  OperationQueue* mOperationQueue;
};

//--------------------------------------------------------------------------- Multi Meta Composition
class MultiMetaComposition : public UndoMetaComposition
{
public:
  MultiMetaComposition(PropertyInterface* propertyInterface, BoundType* objectType, OperationQueue* opQueue);

  /// MetaComposition Interface.
  uint GetComponentCount(HandleParam object) override;
  Handle GetComponentAt(HandleParam object, uint index) override;

  bool CanAddComponent(HandleParam object, BoundType* typeToAdd, AddInfo* info) override;
  void AddComponent(HandleParam owner, BoundType* typeToAdd, int index = -1, bool ignoreDependencies = false) override;

  bool CanRemoveComponent(HandleParam object, HandleParam subObject, String& reason) override;
  void RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies = false) override;

  void Enumerate(Array<BoundType*>& addTypes, EnumerateAction::Enum action, HandleParam object) override;

private:
  /// We can only display components that all objects in the selection
  /// have. This finds all shared components between the selected objects.
  void GetSharedComponents(MetaSelection* selection, Array<BoundType*>& sharedComponents);
};

}//namespace Zero
