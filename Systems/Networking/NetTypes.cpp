///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Editor Tags
namespace Tags
{
  DefineTag(Networking);
}

bool IsValidNetProperty(Property* property)
{
  // Is read-only?
  if(property->IsReadOnly())
  {
    // Unsupported property
    return false;
  }

  // Is supported type?
  return IsValidNetPropertyType(property->PropertyType);
}
bool IsValidNetPropertyType(Type* propertyType)
{
  //    Bitstream can serialize the underlying property type?
  // OR This a cog type?
  // OR This is a cog path type?
  // (We allow NetObject net properties to serialize these types, but not NetEvents, for now)
  return BitStreamCanSerializeType(propertyType)
      || propertyType == ZilchTypeId(Cog)
      || propertyType == ZilchTypeId(CogPath);
}
bool IsValidNetPeerIdPropertyType(Type* propertyType)
{
  return propertyType == ZilchTypeId(int);
}

//---------------------------------------------------------------------------------//
//                                 Network Types                                   //
//---------------------------------------------------------------------------------//

ComponentPropertyInstanceData::ComponentPropertyInstanceData(String propertyName, Component* component)
  : mPropertyName(propertyName),
    mComponent(component)
{
}

//
// NetChannel Authority Property Getter / Setter
//

Variant GetNetChannelAuthorityProperty(const Variant& propertyData)
{
  // Get associated property instance data
  ReplicaChannel* replicaChannel = propertyData.GetOrError<ReplicaChannel*>();

  // Success
  // (We wrap the enum in an Any to take advantage of Zilch meta later during serialization
  // to serialize the enum quantized, using only the bits necessary to represent all enum values)
  return Variant(Any(replicaChannel->GetAuthority()));
}
void SetNetChannelAuthorityProperty(const Variant& value, Variant& propertyData)
{
  // Get associated property instance data
  ReplicaChannel* replicaChannel = propertyData.GetOrError<ReplicaChannel*>();

  // Success
  replicaChannel->SetAuthority(value.GetOrError<Any>().Get<Authority::Enum>());
}

//
// NetObject Parent Property Getter / Setter
//

Variant GetNetObjectParentProperty(const Variant& propertyData)
{
  // Get associated property instance data
  NetObject* netObject = propertyData.GetOrError<NetObject*>();
  Cog*       owner     = netObject->GetOwner();

  // Get parent
  Cog* parent = owner->GetParent();
  if(!parent) // Empty?
    return Variant(NetObjectId(0));

  // Get parent net object
  NetObject* parentNetObject = parent->has(NetObject);
  if(!parentNetObject) // Unable?
    return Variant(NetObjectId(0));

  // Success
  return Variant(parentNetObject->GetNetObjectId());
}
void SetNetObjectParentProperty(const Variant& value, Variant& propertyData)
{
  // Get associated property instance data
  NetObject* netObject     = propertyData.GetOrError<NetObject*>();
  Cog*       owner         = netObject->GetOwner();
  Cog*       currentParent = owner->GetParent();

  // Parent set?
  NetObjectId newParentNetObjectId = value.GetOrError<NetObjectId>();
  if(newParentNetObjectId)
  {
    // Has current parent?
    if(currentParent)
    {
      // The new parent is our current parent?
      NetObject* currentParentNetObject = currentParent->has(NetObject);
      if(currentParentNetObject && (currentParentNetObject->GetNetObjectId() == newParentNetObjectId))
      {
        // Done (This case needs to be explicitly checked because of internal hierarchies!)
        return;
      }
    }

    // Get net peer
    NetPeer* netPeer = netObject->GetNetPeer();
    if(!netPeer)
    {
      Assert(false);
      return;
    }

    // Find new parent
    Cog* newParent = netPeer->GetNetObject(newParentNetObjectId);
    if(!newParent) // Unable?
    {
      // Get net space
      NetSpace* netSpace = netObject->GetNetSpace();

      // We are receiving a net game clone?
      if(netSpace && netPeer->IsReceivingNetGame())
      {
        Assert(netPeer->IsClient());

        // Add delayed attachment, will be performed when the parent exists locally
        netSpace->AddDelayedAttachment(netObject->GetNetObjectId(), newParentNetObjectId);
      }
      // We are not receiving a net game clone?
      else
      {
        DoNotifyWarning("Unable To Set NetObject Parent",
                      String::Format("There was an error setting the parent of NetObject '%s' - Parent NetObject does not exist locally!",
                      owner->GetDescription().c_str()));
      }
      return;
    }

    // Parent has changed?
    if(currentParent != newParent)
    {
      // Detach from old parent
      owner->Detach();

      // Attach to new parent
      owner->AttachTo(newParent);
    }
  }
  // Parent Cleared?
  else
  {
    // Has current parent?
    if(currentParent)
    {
      // Detach from parent
      owner->Detach();
    }
  }
}

//
// Component Cog Property Getter / Setter
//

Variant GetComponentCogProperty(const Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return Variant();

  // Result
  return Variant(GetNetPropertyCogAsNetObjectId(property, component));
}
void SetComponentCogProperty(const Variant& value, Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);
  NetObject* netObject          = component->GetOwner()->has(NetObject);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return;

  // Result
  SetNetPropertyCogAsNetObjectId(property, component, netObject->GetNetPeer(), value.GetOrError<NetObjectId>());
}

//
// Component CogPath Property Getter / Setter
//

Variant GetComponentCogPathProperty(const Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return Variant();

  // Get cog path value
  Any cogPathAny = property->GetValue(component);
  if(!cogPathAny.IsHoldingValue()) // Unable?
  {
    DoNotifyError("NetProperty", "Error getting CogPath NetProperty - Unable to get property instance value");
    return Variant();
  }
  CogPath* cogPath = cogPathAny.Get<CogPath*>();
  Assert(cogPath);

  // Get cog path string
  String cogPathString = cogPath->GetPath();
  return Variant(cogPathString);
}
void SetComponentCogPathProperty(const Variant& value, Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return;

  // Get cog path value
  Any cogPathAny = property->GetValue(component);
  if(!cogPathAny.IsHoldingValue()) // Unable?
  {
    DoNotifyError("NetProperty", "Error setting CogPath NetProperty - Unable to get property instance value");
    return;
  }
  CogPath* cogPath = cogPathAny.Get<CogPath*>();
  Assert(cogPath);

  // Set cog path string
  String cogPathString = value.GetOrError<String>();
  cogPath->SetPath(cogPathString);
}

//
// Component Any Property Getter / Setter
//

Variant GetComponentAnyProperty(const Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return Variant();

  // Get any value
  Any anyValue = property->GetValue(component);

  // Attempt to convert basic any value to variant value
  Variant variantValue = ConvertBasicAnyToVariant(anyValue);
  if(variantValue.IsEmpty())// Unable? (The any's stored type is not a basic native type?)
  {
    // Assign the any value itself to the variant value
    // (Some property types like enums, resources, and bitstream are expected to be wrapped in an any this way)
    variantValue = anyValue;
  }

  // Return the property value
  return variantValue;
}
void SetComponentAnyProperty(const Variant& value, Variant& propertyData)
{
  // Get associated property instance data
  String     propertyName       = propertyData.GetOrError<ComponentPropertyInstanceData>().mPropertyName;
  Component* component          = propertyData.GetOrError<ComponentPropertyInstanceData>().mComponent;
  BoundType* componentBoundType = ZilchVirtualTypeId(component);

  // Get property instance
  Property* property = componentBoundType->GetProperty(propertyName);
  if(!property) // Unable?
    return;

  // Get variant value
  const Variant& variantValue = value;

  // Attempt to convert basic variant value to any value
  Any anyValue = ConvertBasicVariantToAny(variantValue);
  if(!anyValue.IsHoldingValue())// Unable? (The variant's stored type is not a basic native type?)
  {
    // Get the any value itself from the variant value
    // (Some property types like enums, resources, and bitstream are expected to be wrapped in an any this way)
    anyValue = variantValue.GetOrError<Any>();
  }

  // Set the property value
  property->SetValue(component, anyValue);
}

//
// Helper Methods
//

BasicNativeType::Enum BasicNetworkToNativeTypeEnum(BasicNetType::Enum value)
{
  switch(value)
  {
  default:
  case BasicNetType::Other:
    return BasicNativeType::Unknown;

  case BasicNetType::Byte:
    return BasicNativeType::Uint8;

  case BasicNetType::Boolean:
    return BasicNativeType::Bool;
  case BasicNetType::Boolean2:
    return BasicNativeType::BoolVector2;
  case BasicNetType::Boolean3:
    return BasicNativeType::BoolVector3;
  case BasicNetType::Boolean4:
    return BasicNativeType::BoolVector4;

  case BasicNetType::Integer:
    return BasicNativeType::Int32;
  case BasicNetType::Integer2:
    return BasicNativeType::IntVector2;
  case BasicNetType::Integer3:
    return BasicNativeType::IntVector3;
  case BasicNetType::Integer4:
    return BasicNativeType::IntVector4;

  case BasicNetType::Real:
    return BasicNativeType::Float;
  case BasicNetType::Real2:
    return BasicNativeType::Vector2;
  case BasicNetType::Real3:
    return BasicNativeType::Vector3;
  case BasicNetType::Real4:
    return BasicNativeType::Vector4;

  case BasicNetType::Quaternion:
    return BasicNativeType::Quaternion;

  case BasicNetType::Real2x2:
    return BasicNativeType::Matrix2;
  case BasicNetType::Real3x3:
    return BasicNativeType::Matrix3;
  case BasicNetType::Real4x4:
    return BasicNativeType::Matrix4;

  case BasicNetType::DoubleInteger:
    return BasicNativeType::Int64;
  case BasicNetType::DoubleReal:
    return BasicNativeType::Double;

  case BasicNetType::String:
    return BasicNativeType::String;
  }
}
BasicNetType::Enum BasicNativeToNetworkTypeEnum(BasicNativeType::Enum value)
{
  switch(value)
  {
  default:
  case BasicNativeType::Unknown:
    return BasicNetType::Other;

  case BasicNativeType::Uint8:
    return BasicNetType::Byte;

  case BasicNativeType::Bool:
    return BasicNetType::Boolean;
  case BasicNativeType::BoolVector2:
    return BasicNetType::Boolean2;
  case BasicNativeType::BoolVector3:
    return BasicNetType::Boolean3;
  case BasicNativeType::BoolVector4:
    return BasicNetType::Boolean4;

  case BasicNativeType::Int32:
    return BasicNetType::Integer;
  case BasicNativeType::IntVector2:
    return BasicNetType::Integer2;
  case BasicNativeType::IntVector3:
    return BasicNetType::Integer3;
  case BasicNativeType::IntVector4:
    return BasicNetType::Integer4;

  case BasicNativeType::Float:
    return BasicNetType::Real;
  case BasicNativeType::Vector2:
    return BasicNetType::Real2;
  case BasicNativeType::Vector3:
    return BasicNetType::Real3;
  case BasicNativeType::Vector4:
    return BasicNetType::Real4;

  case BasicNativeType::Quaternion:
    return BasicNetType::Quaternion;

  case BasicNativeType::Matrix2:
    return BasicNetType::Real2x2;
  case BasicNativeType::Matrix3:
    return BasicNetType::Real3x3;
  case BasicNativeType::Matrix4:
    return BasicNetType::Real4x4;

  case BasicNativeType::Int64:
    return BasicNetType::DoubleInteger;
  case BasicNativeType::Double:
    return BasicNetType::DoubleReal;

  case BasicNativeType::String:
    return BasicNetType::String;
  }
}

bool HasNetPeerIdProperty(Event* event)
{
  // Get event type
  BoundType* eventType = ZilchVirtualTypeId(event);
  if(!eventType)
  {
    Assert(false);
    return false;
  }

  // For all properties
  MemberRange<Property> properties = eventType->GetProperties(Members::InheritedInstanceExtension);
  forRange(Property* property, properties)
  {
    // Is a net peer ID property?
    if(property->HasAttribute(cNetPeerId)
    && IsValidNetPeerIdPropertyType(property->GetValue(event).StoredType))
      return true;
  }

  return false;
}
void SetNetPeerIdProperties(Event* event, NetPeerId netPeerId)
{
  // Get event type
  BoundType* eventType = ZilchVirtualTypeId(event);
  if(!eventType)
  {
    Assert(false);
    return;
  }

  // For all properties
  MemberRange<Property> properties = eventType->GetProperties(Members::InheritedInstanceExtension);
  forRange(Property* property, properties)
  {
    // Is a net peer ID property?
    if(property->HasAttribute(cNetPeerId)
    && IsValidNetPeerIdPropertyType(property->GetValue(event).StoredType))
    {
      // Set net peer ID
      property->SetValue(event, netPeerId);
    }
  }
}

void SetNetPropertyCogAsNetObjectId(Property* property, const Any& instance, NetPeer* netPeer, NetObjectId netObjectId)
{
  // Cog set?
  Cog* cog = nullptr;
  if(netObjectId)
  {
    // Find cog
    cog = netPeer->GetNetObject(netObjectId);
    if(!cog) // Unable?
    {
      DoNotifyWarning("NetProperty", "Unable to set Cog NetProperty - Network object does not exist locally");
      return;
    }
  }

  // Success
  property->SetValue(instance, cog);
}
NetObjectId GetNetPropertyCogAsNetObjectId(Property* property, const Any& instance)
{
  // Get cog value
  Cog* cog = property->GetValue(instance).Get<Cog*>();
  if(!cog) // Empty?
    return NetObjectId(0);

  // Get cog net object
  NetObject* cogNetObject = cog->has(NetObject);
  if(!cogNetObject) // Unable?
  {
    DoNotifyError("NetProperty", "Error getting Cog NetProperty - Cog being referenced must have a NetObject component");
    return NetObjectId(0);
  }

  // Success
  return cogNetObject->GetNetObjectId();
}

//
// NetPeer Protocol Message Types
//

//---------------------------------------------------------------------------------//
//                             NetUserAddRequestData                               //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserAddRequestData& netUserAddRequestData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write dummy bit
    // TODO: Refactor BitStream interface to allow for serialization of empty structures without appearing to be an error
    bitStream.WriteBit(true);

    // Write event bundle data (if any)
    bitStream.AppendAll(netUserAddRequestData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read dummy bit
    // TODO: Refactor BitStream interface to allow for serialization of empty structures without appearing to be an error
    bool dummy;
    ReturnIf(!bitStream.Read(dummy), 0);

    // Read event bundle data (if any)
    netUserAddRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                             NetUserAddResponseData                              //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserAddResponseData& netUserAddResponseData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write network user add response
    bitStream.WriteQuantized(netUserAddResponseData.mAddResponse, NetUserAddResponseMin, NetUserAddResponseMax);

    // Accepted?
    if(netUserAddResponseData.mAddResponse == NetUserAddResponse::Accept)
    {
      // Write network user identifier
      bitStream.Write(netUserAddResponseData.mNetUserId);
    }

    // Write event bundle data (if any)
    bitStream.AppendAll(netUserAddResponseData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read network user add response
    ReturnIf(!bitStream.ReadQuantized(netUserAddResponseData.mAddResponse, NetUserAddResponseMin, NetUserAddResponseMax), 0);

    // Accepted?
    if(netUserAddResponseData.mAddResponse == NetUserAddResponse::Accept)
    {
      // Read network user identifier
      ReturnIf(!bitStream.Read(netUserAddResponseData.mNetUserId), 0);
    }

    // Read event bundle data (if any)
    netUserAddResponseData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                            NetUserRemoveRequestData                             //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserRemoveRequestData& netUserRemoveRequestData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write network user identifier
    bitStream.Write(netUserRemoveRequestData.mNetUserId);

    // Write event bundle data (if any)
    bitStream.AppendAll(netUserRemoveRequestData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read network user identifier
    ReturnIf(!bitStream.Read(netUserRemoveRequestData.mNetUserId), 0);

    // Read event bundle data (if any)
    netUserRemoveRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                            NetLevelLoadStartedData                              //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetLevelLoadStartedData& netLevelLoadStartedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Get network space object identifier
    // (Using ReplicaId to take advantage of WriteQuantized)
    ReplicaId netSpaceObjectId = netLevelLoadStartedData.mNetSpaceObjectId;

    // Write network space object identifier
    bitStream.Write(netSpaceObjectId);

    // Write level resource identifier
    bitStream.Write(netLevelLoadStartedData.mLevelResourceId);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read network space object identifier
    // (Using ReplicaId to take advantage of ReadQuantized)
    ReplicaId netSpaceObjectId;
    if(!bitStream.Read(netSpaceObjectId)) // Unable?
      return 0;

    // Set network space object identifier
    netLevelLoadStartedData.mNetSpaceObjectId = netSpaceObjectId.value();

    // Read level resource identifier
    ReturnIf(!bitStream.Read(netLevelLoadStartedData.mLevelResourceId), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                            NetLevelLoadFinishedData                             //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetLevelLoadFinishedData& netLevelLoadFinishedData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Get network space object identifier
    // (Using ReplicaId to take advantage of WriteQuantized)
    ReplicaId netSpaceObjectId = netLevelLoadFinishedData.mNetSpaceObjectId;

    // Write network space object identifier
    bitStream.Write(netSpaceObjectId);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read network space object identifier
    // (Using ReplicaId to take advantage of ReadQuantized)
    ReplicaId netSpaceObjectId;
    if(!bitStream.Read(netSpaceObjectId)) // Unable?
      return 0;

    // Set network space object identifier
    netLevelLoadFinishedData.mNetSpaceObjectId = netSpaceObjectId.value();

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              NetConnectRequestData                              //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetConnectRequestData& netConnectRequestData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write pending user add request count
    bitStream.Write(netConnectRequestData.mAddUserRequestCount);

    // Write event bundle data (if any)
    bitStream.AppendAll(netConnectRequestData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read pending user add request count
    ReturnIf(!bitStream.Read(netConnectRequestData.mAddUserRequestCount), 0);

    // Read event bundle data (if any)
    netConnectRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                                 NetHostPingData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPingData& netHostPingData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write unique project identifier
    bitStream.Write((u64)netHostPingData.mProjectGuid);

    // Write unique ping request identifier
    bitStream.Write(netHostPingData.mPingId);

    // Write unique send attempt identifier
    bitStream.Write(netHostPingData.mSendAttemptId);

    // Write unique manager id
    bitStream.Write(netHostPingData.mManagerId);

    // Write event bundle data (if any)
    bitStream.AppendAll(netHostPingData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read unique project identifier
    ReturnIf(!bitStream.Read((u64&)netHostPingData.mProjectGuid), 0);

    // Read unique ping request identifier
    ReturnIf(!bitStream.Read(netHostPingData.mPingId), 0);

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPingData.mSendAttemptId), 0);

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPingData.mManagerId), 0);

    // Read event bundle data (if any)
    netHostPingData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                                 NetHostPongData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPongData& netHostPongData)
{
  // Write operation?
  if(direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write unique project identifier
    bitStream.Write((u64)netHostPongData.mProjectGuid);

    // Write unique ping request identifier
    bitStream.Write(netHostPongData.mPingId);

    // Write unique send attempt identifier
    bitStream.Write(netHostPongData.mSendAttemptId);

    // Write unique manager id.
    bitStream.Write(netHostPongData.mManagerId);

    // Write event bundle data (if any)
    bitStream.AppendAll(netHostPongData.mEventBundleData);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read unique project identifier
    ReturnIf(!bitStream.Read((u64&)netHostPongData.mProjectGuid), 0);

    // Read unique ping request identifier
    ReturnIf(!bitStream.Read(netHostPongData.mPingId), 0);

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPongData.mSendAttemptId), 0);

    // Read unique manager id
    ReturnIf(!bitStream.Read(netHostPongData.mManagerId), 0);

    // Read event bundle data (if any)
    netHostPongData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                                NetHostRecordList                                //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostRecordListData& netHostRecordList)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // For each NetHostRecord
      // Write NetHostRecord
        // Write IpAddress
        // Write EventBundle (BasicNetHostInfo)
          // Get tempBitStream from EventBundle
          // Write tempBitStream size
          // Write bits (from tempBitStream as buffer)

    //write how many elements there are.
    bitStream.Write(netHostRecordList.mNetHostRecords.Size());

    forRange(NetHostRecord& record, netHostRecordList.mNetHostRecords.All())
    {
      //Serialize the IP address
      bitStream.Write(record.mIpAddress);
      //Serialize the bitstream
      BitStream& tempBitStream = record.mBasicHostInfo.GetBitStream();
      Bits bitsWritten = tempBitStream.GetBitsWritten();
      bitStream.Write(bitsWritten);
      bitStream.WriteBits(tempBitStream.GetDataExposed(), bitsWritten);
    }

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Until bitstream is empty
      // Create NetHostRecord
      // Read NetHostRecord
        // Read IpAddress
        // Read EventBundle (BasicNetHostInfo)
          // Create tempBitStream, size int
          // Read tempBitStream size
          // Read bits (to tempBitStream as buffer)
          // Set bits written on tempBitStream
          // Assign tempBitStream into EventBundle (==)
    // Pushback NetHostRecord Into Our Array

    size_t elements = 0;
    bitStream.Read(elements);
    //TODO: Potentially need to also write how many elements were in this array. Its possible this isn't the only thing in a message.
    //If there is anything written after this in a message, it would clobber it.
    for(size_t i = 0; i < elements; i += 1 )
    {
      NetHostRecord record;
      //Read in IP address of record.
      ReturnIf(!bitStream.Read(record.mIpAddress), 0);
      //Read in the size of the BitStream written.
      Bits bitsToRead = 0;
      ReturnIf(!bitStream.Read(bitsToRead), 0);
      //Create a temp bitstream, and have Reserve enough space for it to read the bitstream in.
      BitStream tempBitStream;
      tempBitStream.Reserve(bitsToRead / 8); // Reserve enough space to write the bits into the stream directly.
      //Read in the bitstream containing the BasicHostInfo eventbundle.
      ReturnIf(!bitStream.ReadBits(tempBitStream.GetDataExposed(), bitsToRead), 0);
      //Set the number of bits we wrote into it (because we manually assigned bit data into it)
      tempBitStream.SetBitsWritten(bitsToRead);
      //move the bitstream
      record.mBasicHostInfo = ZeroMove(tempBitStream);
      //Move the record into the array.
      netHostRecordList.mNetHostRecords.PushBack( record );
    }

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              NetHostPublishData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPublishData& netHostPingData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write unique project identifier
    bitStream.Write((u64)netHostPingData.mProjectGuid);

    // Write event bundle data (if any)
    bitStream.AppendAll(netHostPingData.mBasicHostInfo);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read unique project identifier
    ReturnIf(!bitStream.Read((u64&)netHostPingData.mProjectGuid), 0);

    // Read event bundle data (if any)
    netHostPingData.mBasicHostInfo.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              NetRequestHostRefreshData                          //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetRequestHostRefreshData& netRequestHostRefreshData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write unique project identifier
    bitStream.Write((u64)netRequestHostRefreshData.mProjectGuid);

    // Write event bundle data (if any)
    bitStream.Write(netRequestHostRefreshData.mHostIp);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read unique project identifier
    ReturnIf(!bitStream.Read((u64&)netRequestHostRefreshData.mProjectGuid), 0);

    // Read event bundle data (if any)
    ReturnIf(!bitStream.Read(netRequestHostRefreshData.mHostIp), 0);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                              NetHostRefreshData                                 //
//---------------------------------------------------------------------------------//

Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostRefreshData& netHostRefreshData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write IP address of host that is being refreshed.
    bitStream.Write(netHostRefreshData.mHostIp);
    // Write event bundle data (if any)
    bitStream.AppendAll(netHostRefreshData.mBasicHostInfo);

    // Success
    return bitStream.GetBitsWritten() - bitsWrittenStart;
  }
  // Read operation?
  else
  {
    const Bits bitsReadStart = bitStream.GetBitsRead();

    // Read IP address of host that is being refreshed.
    ReturnIf(!bitStream.Read(netHostRefreshData.mHostIp), 0);
    // Read event bundle data (if any)
    netHostRefreshData.mBasicHostInfo.AssignRemainder(bitStream);
    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
}

//---------------------------------------------------------------------------------//
//                                 FamilyTree                                      //
//---------------------------------------------------------------------------------//

FamilyTree::FamilyTree()
  : mFamilyTreeId(0),
    mAncestorDisplayName(),
    mAncestorCreateContext(),
    mAncestorReplicaType(),
    mReplicas()
{
  // (Should not actually get called)
  Assert(false);
}
FamilyTree::FamilyTree(FamilyTreeId familyTreeId, NetObject* ancestor)
  : mFamilyTreeId(familyTreeId),
    mAncestorDisplayName(ancestor->GetOwner()->GetDescription()),
    mAncestorCreateContext(ancestor->GetCreateContext()),
    mAncestorReplicaType(ancestor->GetReplicaType()),
    mReplicas()
{
  // (Family tree ID should be valid)
  Assert(familyTreeId != 0);

  AddNetObject(ancestor);
}

FamilyTree::~FamilyTree()
{
}

bool FamilyTree::operator ==(const FamilyTree& rhs) const
{
  return mFamilyTreeId == rhs.mFamilyTreeId;
}
bool FamilyTree::operator !=(const FamilyTree& rhs) const
{
  return mFamilyTreeId != rhs.mFamilyTreeId;
}
bool FamilyTree::operator  <(const FamilyTree& rhs) const
{
  return mFamilyTreeId < rhs.mFamilyTreeId;
}
bool FamilyTree::operator ==(const FamilyTreeId& rhs) const
{
  return mFamilyTreeId == rhs;
}
bool FamilyTree::operator !=(const FamilyTreeId& rhs) const
{
  return mFamilyTreeId != rhs;
}
bool FamilyTree::operator  <(const FamilyTreeId& rhs) const
{
  return mFamilyTreeId < rhs;
}

//
// Operations
//

bool FamilyTree::AddNetObject(NetObject* netObject)
{
  // Add net object
  mReplicas.PushBack(static_cast<Replica*>(netObject));

  // Set net object's family tree ID
  netObject->SetFamilyTreeId(mFamilyTreeId);

  // Success
  return true;
}
bool FamilyTree::RemoveNetObject(NetObject* netObject)
{
  // Find pointer to net object
  if(Replica** pointer = mReplicas.FindPointer(static_cast<Replica*>(netObject)))
  {
    // Clear net object's family tree ID
    netObject->SetFamilyTreeId(0);

    // Clear pointer (we intentionally leave the nullptr element in the array)
    *pointer = nullptr;

    // Found
    return true;
  }

  // Not found
  return false;
}

FamilyTreeId FamilyTree::GetFamilyTreeId() const
{
  return mFamilyTreeId;
}

const String& FamilyTree::GetAncestorDisplayName() const
{
  return mAncestorDisplayName;
}
const CreateContext& FamilyTree::GetAncestorCreateContext() const
{
  return mAncestorCreateContext;
}
const ReplicaType& FamilyTree::GetAncestorReplicaType() const
{
  return mAncestorReplicaType;
}

const ReplicaArray& FamilyTree::GetReplicas() const
{
  return mReplicas;
}

bool FamilyTree::IsEmpty() const
{
  // For All net objects
  forRange(Replica* netObject, mReplicas.All())
    if(netObject) // Is present?
    {
      // There is at least one present net object
      return false;
    }

  // All net objects are absent
  return true;
}

} // namespace Zero
