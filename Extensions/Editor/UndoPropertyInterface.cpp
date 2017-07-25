////////////////////////////////////////////////////////////////////////////////
///
/// \file CogPropertyInterface.cpp
/// Implementation of the property interface for Cogs.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2013-2014, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//******************************************************************************
Handle GetActingObject(HandleParam object, Cog* cog)
{
  if(cog == nullptr)
    return Handle();

  if(object.Get<MetaSelection*>())
    return cog;

  return cog->QueryComponentType(object.StoredType);
}

//--------------------------------------------------------------- PropertyToUndo
//******************************************************************************
PropertyToUndo::PropertyToUndo(OperationQueue* propQueue)
{
  mOperationQueue = propQueue;
}

//******************************************************************************
void PropertyToUndo::ChangeProperty(HandleParam object, 
                                    PropertyPathParam property, PropertyState& state,
                                    PropertyAction::Enum action)
{
  //if this was a commit operation then queue the undo
  if(action == PropertyAction::Commit)
  {
    ChangeAndQueueProperty(mOperationQueue, object, property, state.Value);
  }
  else
  {
    Any oldValue = property.GetValue(object);
    property.SetValue(object, state.Value);

    MetaOperations::NotifyPropertyModified(object, property, oldValue, state.Value, true);
  }
}

//******************************************************************************
void PropertyToUndo::InvokeFunction(HandleParam object, Function* function)
{
  OperationQueue::StartListeningForSideEffects();
  PropertyInterface::InvokeFunction(object, function);
  mOperationQueue->QueueRegisteredSideEffects();
}

//******************************************************************************
HandleOf<MetaComposition> PropertyToUndo::GetMetaComposition(BoundType* objectType)
{
  // If the object itself doesn't have a meta composition, we don't want to return our custom one
  if(MetaComposition* objectComposition = objectType->HasInherited<MetaComposition>())
    return new UndoMetaComposition(this, objectType, mOperationQueue);
  return nullptr;
}

//******************************************************************************
void PropertyToUndo::Undo()
{
  mOperationQueue->Undo();
}

//******************************************************************************
void PropertyToUndo::Redo()
{
  mOperationQueue->Redo();
}

//---------------------------------------------------------------------------- Undo Meta Composition
//******************************************************************************
UndoMetaComposition::UndoMetaComposition(PropertyInterface* propertyInterface,
                                         BoundType* objectType, OperationQueue* opQueue) : 
  EventMetaComposition(propertyInterface, objectType),
  mOperationQueue(opQueue)
{

}

//******************************************************************************
void UndoMetaComposition::AddComponent(HandleParam owner, BoundType* typeToAdd, int index, bool ignoreDependencies)
{
  QueueAddComponent(mOperationQueue, owner, typeToAdd);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

//******************************************************************************
void UndoMetaComposition::RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies)
{
  QueueRemoveComponent(mOperationQueue, owner, component.StoredType);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

//******************************************************************************
void UndoMetaComposition::MoveComponent(HandleParam owner, HandleParam component, uint destination)
{
  QueueMoveComponent(mOperationQueue, owner, component, destination);
  mPropertyInterface->SendComponentsModifiedOnGrid(owner);
}

}//namespace Zero
