///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

// Includes
#include "NetOperations.hpp"
#include "Networking\NetObject.hpp"

namespace Zero
{

//
// NetPropertyInfo Operations
//

//******************************************************************************
void AddNetPropertyInfo(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName)
{
  // Create and perform the Add NetPropertyInfo operation
  Operation* op = new AddRemoveNetPropertyInfoOperation(netObject, componentType, propertyName, NetPropertyInfoAction::Add);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void RemoveNetPropertyInfo(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName)
{
  // Create and perform the Remove NetPropertyInfo operation
  Operation* op = new AddRemoveNetPropertyInfoOperation(netObject, componentType, propertyName, NetPropertyInfoAction::Remove);
  op->Redo();
  queue->Queue(op);
}

//******************************************************************************
void SetNetPropertyInfoChannel(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName,
                                 NetChannelConfig* netChannelConfig)
{
  // Create and perform the Set NetPropertyInfo Channel operation
  Operation* op = new SetNetPropertyInfoChannelOperation(netObject, componentType, propertyName, netChannelConfig);
  op->Redo();
  queue->Queue(op);
}

//---------------------------------------------------------------------------------//
//                      AddRemoveNetPropertyInfoOperation                          //
//---------------------------------------------------------------------------------//

//******************************************************************************
AddRemoveNetPropertyInfoOperation::AddRemoveNetPropertyInfoOperation(NetObject* netObject, BoundType* componentType, StringParam propertyName,
                                                                     NetPropertyInfoAction::Enum action)
  : mComponentType(componentType),
    mPropertyName(propertyName),
    mAction(action)

{
  ErrorIf(netObject->GetOwner() == nullptr, "NetObject doesn't have an Owner");

  String opType;
  if(action == NetPropertyInfoAction::Add)
    opType = BuildString("Add NetPropertyInfo \"", propertyName, "\" to ");
  else
    opType = BuildString("Remove NetPropertyInfo \"", propertyName, "\" from ");

  mName = BuildString("\"", opType, "\"", CogDisplayName(netObject->GetOwner()), "\"");

  // Set undo object handle
  mObjectHandle = netObject->GetOwner();
}

//
// Operation Interface
//

//******************************************************************************
void AddRemoveNetPropertyInfoOperation::Undo()
{
  // Add if we're undoing a remove operation
  // Remove if we're undoing an add operation
  NetPropertyInfoAction::Enum action = (mAction == NetPropertyInfoAction::Remove) ? NetPropertyInfoAction::Add
                                                                                  : NetPropertyInfoAction::Remove;

  // Perform net property info action
  SetNetPropertyInfo(action);
}

//******************************************************************************
void AddRemoveNetPropertyInfoOperation::Redo()
{
  // Perform net property info action
  SetNetPropertyInfo(mAction);
}

//******************************************************************************
void AddRemoveNetPropertyInfoOperation::SetNetPropertyInfo(NetPropertyInfoAction::Enum action)
{
  // Get Cog from handle
  Cog* object = mObjectHandle;
  ReturnIf(object == nullptr, , "Couldn't find undo object.");

  // Get NetObject from Cog
  NetObject* netObject = object->has(NetObject);
  ReturnIf(netObject == nullptr, , "Object doesn't have net object for redo. "
                                   "Operation queue is invalid.");

  // Adding a net property info?
  if(action == NetPropertyInfoAction::Add)
  {
    // Add net property info
    NetPropertyInfo* netProperty = netObject->AddNetPropertyInfo(mComponentType, mPropertyName);
    ReturnIf(netProperty == nullptr, , "Unable to add net property info for redo.");
  }
  // Removing a net property info?
  else
  {
    // Remove net property info
    netObject->RemoveNetPropertyInfo(mComponentType, mPropertyName);
  }
}

//---------------------------------------------------------------------------------//
//                      SetNetPropertyInfoChannelOperation                         //
//---------------------------------------------------------------------------------//

//******************************************************************************
SetNetPropertyInfoChannelOperation::SetNetPropertyInfoChannelOperation(NetObject* netObject, BoundType* componentType, StringParam propertyName,
                                                                       NetChannelConfig* netChannelConfig)
  : mComponentType(componentType),
    mPropertyName(propertyName),
    mLastNetChannelConfig(),
    mCurrentNetChannelConfig()
{
  ErrorIf(netObject->GetOwner() == nullptr, "NetObject doesn't have an Owner");

  String objectName = CogDisplayName(netObject->GetOwner());
  String propertyRef = BuildString(componentType->Name, propertyName);

  mName = BuildString(objectName, ".NetObject.NetPropertyInfo.", propertyRef, ".NetChannelConfig");

  // Set undo object handle
  mObjectHandle = netObject->GetOwner();

  // Set last and current channel config values
  NetPropertyInfo* netPropertyInfo = netObject->GetNetPropertyInfo(componentType, propertyName);
  mLastNetChannelConfig = netPropertyInfo->mNetChannelConfig;
  mCurrentNetChannelConfig = netChannelConfig;
}

//
// Operation Interface
//

//******************************************************************************
void SetNetPropertyInfoChannelOperation::Undo()
{
  // Set last channel config
  SetNetPropertyInfoChannel(mLastNetChannelConfig);
}

//******************************************************************************
void SetNetPropertyInfoChannelOperation::Redo()
{
  // Set current channel config
  SetNetPropertyInfoChannel(mCurrentNetChannelConfig);
}

//******************************************************************************
void SetNetPropertyInfoChannelOperation::SetNetPropertyInfoChannel(NetChannelConfig* netChannelConfig)
{
  // Get Cog from handle
  Cog* object = mObjectHandle.Get<Cog*>();
  ReturnIf(object == nullptr, , "Couldn't find undo object.");

  // Get NetObject from Cog
  NetObject* netObject = object->has(NetObject);
  ReturnIf(netObject == nullptr, , "Object doesn't have net object for redo. "
                                   "Operation queue is invalid.");

  // Get net property info
  NetPropertyInfo* netPropertyInfo = netObject->GetNetPropertyInfo(mComponentType, mPropertyName);

  // Set net channel config resource
  netPropertyInfo->mNetChannelConfig = netChannelConfig;

  // The object has been modified
  object->GetSpace()->MarkModified();
}

}//namespace Zero
