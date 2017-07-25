////////////////////////////////////////////////////////////////////////////////
///
/// \file CogPropertyInterface.hpp
/// Declaration of the property interface for Cogs.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2013-2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward declarations
class OperationQueue;

//--------------------------------------------------------------- PropertyToUndo
/// This property interface queue's single object operations for undo / redo.
class PropertyToUndo : public PropertyInterface
{
public:
  PropertyToUndo(OperationQueue* propQueue);

  /// PropertyInterface Interface.
  void ChangeProperty(HandleParam object, PropertyPathParam property,
                      PropertyState& state, PropertyAction::Enum action) override;
  void InvokeFunction(HandleParam object, Function* function) override;
  HandleOf<MetaComposition> GetMetaComposition(BoundType* objectType) override;

  void Undo() override;
  void Redo() override;

  OperationQueue* mOperationQueue;
};

//---------------------------------------------------------------------------- Undo Meta Composition
class UndoMetaComposition : public EventMetaComposition
{
public:
  // The objectType should be the type of objects the selection contains.
  UndoMetaComposition(PropertyInterface* propertyInterface, BoundType* objectType, OperationQueue* opQueue);

  void AddComponent(HandleParam owner, BoundType* typeToAdd, int index = -1, bool ignoreDependencies = false) override;
  void RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies = false) override;
  void MoveComponent(HandleParam owner, HandleParam component, uint destination) override;

  OperationQueue* mOperationQueue;
};

}//namespace Zero
