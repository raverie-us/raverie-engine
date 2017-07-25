///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Forward Declarations
namespace Zero
{
  class OperationQueue;
  class NetObject;
}

namespace Zero
{

//
// NetPropertyInfo Operations
//

/// Adds a net property info (specified by component type and property name) to the given net object
void AddNetPropertyInfo(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName);

/// Removes a net property info (specified by component type and property name) from the given net object
void RemoveNetPropertyInfo(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName);

/// Sets a net property info's net channel config resource
void SetNetPropertyInfoChannel(OperationQueue* queue, NetObject* netObject, BoundType* componentType, StringParam propertyName,
                                 NetChannelConfig* netChannelConfig);

//---------------------------------------------------------------------------------//
//                      AddRemoveNetPropertyInfoOperation                          //
//---------------------------------------------------------------------------------//

/// Specifies what action to take with the specified net property info
DeclareEnum2(NetPropertyInfoAction,
  Add,     /// Adds the net property info
  Remove); /// Removes the net property info

/// Add/Remove NetPropertyInfo Operation
class AddRemoveNetPropertyInfoOperation : public Operation
{
public:
  /// Constructor
  AddRemoveNetPropertyInfoOperation(NetObject* netObject, BoundType* componentType, StringParam propertyName,
                                      NetPropertyInfoAction::Enum action);

  //
  // Operation Interface
  //

  void Undo() override;
  void Redo() override;

private:
  /// Adds/Removes the net property info according to the specified action
  void SetNetPropertyInfo(NetPropertyInfoAction::Enum action);

  //
  // Data
  //

  /// Object handle
  UndoHandleOf<Cog>           mObjectHandle;
  /// Component meta type
  BoundType*                  mComponentType;
  /// Property variable name
  String                      mPropertyName;
  /// Net property data action
  NetPropertyInfoAction::Enum mAction;
};

//---------------------------------------------------------------------------------//
//                      SetNetPropertyInfoChannelOperation                         //
//---------------------------------------------------------------------------------//

/// Set NetPropertyInfo Channel Operation
class SetNetPropertyInfoChannelOperation : public Operation
{
public:
  /// Constructor
  SetNetPropertyInfoChannelOperation(NetObject* netObject, BoundType* componentType,
                                     StringParam propertyName, NetChannelConfig* netChannelConfig);

  //
  // Operation Interface
  //

  void Undo() override;
  void Redo() override;

private:
  /// Sets the net property info's net channel config resource
  void SetNetPropertyInfoChannel(NetChannelConfig* netChannelConfig);

  //
  // Data
  //

  /// Object handle
  UndoHandleOf<Cog>                   mObjectHandle;
  /// Component meta type
  BoundType*                          mComponentType;
  /// Property variable name
  String                              mPropertyName;
  /// Last net channel configuration resource
  HandleOf<NetChannelConfig> mLastNetChannelConfig;
  /// Current net channel configuration resource
  HandleOf<NetChannelConfig> mCurrentNetChannelConfig;
};

}//namespace Zero
