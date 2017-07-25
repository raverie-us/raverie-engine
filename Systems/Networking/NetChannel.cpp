///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                 NetChannel                                      //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetChannel, builder, type)
{
  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Bind operations
  ZilchBindGetterProperty(Name);
  ZilchBindGetterProperty(NetChannelType);
  ZilchBindCustomGetterProperty(IsScheduled);
  ZilchBindCustomGetterProperty(IsNapping);
  ZilchBindMethod(WakeUp);
  ZilchBindMethod(TakeNap);
  ZilchBindGetterSetterProperty(ChangeFlag);
  ZilchBindMethod(ReplicateNow);
  ZilchBindGetterProperty(LastChangeTimestamp);
  ZilchBindGetterProperty(LastChangeTimePassed);
  ZilchBindGetterSetterProperty(Authority);

  // Bind property management
  ZilchBindMethod(HasNetProperty);
  ZilchBindMethod(GetNetProperty);
}

NetChannel::NetChannel(const String& name, NetChannelType* netChannelType)
  : ReplicaChannel(name, netChannelType)
{
}

NetChannel::~NetChannel()
{
}

//
// Operations
//

NetPeer* NetChannel::GetNetPeer() const
{
  NetObject* netObject = GetNetObject();
  return netObject ? netObject->GetNetPeer() : nullptr;
}

NetObject* NetChannel::GetNetObject() const
{
  return static_cast<NetObject*>(ReplicaChannel::GetReplica());
}

const String& NetChannel::GetName() const
{
  return ReplicaChannel::GetName();
}

NetChannelType* NetChannel::GetNetChannelType() const
{
  return (NetChannelType*)ReplicaChannel::GetReplicaChannelType();
}

bool NetChannel::IsScheduled() const
{
  return ReplicaChannel::IsScheduled();
}

bool NetChannel::IsNapping() const
{
  return ReplicaChannel::IsNapping();
}

void NetChannel::WakeUp()
{
  return ReplicaChannel::WakeUp();
}
void NetChannel::TakeNap()
{
  return ReplicaChannel::TakeNap();
}

void NetChannel::SetChangeFlag(bool changeFlag)
{
  return ReplicaChannel::SetChangeFlag(changeFlag);
}
bool NetChannel::GetChangeFlag() const
{
  return ReplicaChannel::GetChangeFlag();
}

bool NetChannel::ReplicateNow()
{
  // Not valid? (Net object is not online?)
  if(!ReplicaChannel::IsValid())
    return false;

  return ReplicaChannel::ObserveAndReplicateChanges(true, true);
}

float NetChannel::GetLastChangeTimestamp() const
{
  // Get last change timestamp
  TimeMs timestamp = ReplicaChannel::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  return TimeMsToFloatSeconds(timestamp);
}

float NetChannel::GetLastChangeTimePassed() const
{
  // Get replicator
  Replicator* replicator = ReplicaChannel::GetReplicator();
  if(!replicator) // Unable?
    return 0;

  // Get current time
  TimeMs now = replicator->GetPeer()->GetLocalTime();

  // Get last change timestamp
  TimeMs timestamp = ReplicaChannel::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  // Compute time passed since last change (duration between now and last change timestamp)
  TimeMs timePassed = (now - timestamp);
  return TimeMsToFloatSeconds(timePassed);
}

void NetChannel::SetAuthority(Authority::Enum authority)
{
  //     Already valid? (Net object is already online?)
  // AND Replica channel type is using a fixed authority mode?
  if(ReplicaChannel::IsValid()
  && ReplicaChannel::GetReplicaChannelType()->GetAuthorityMode() == AuthorityMode::Fixed)
  {
    // Unable to modify authority
    DoNotifyError("NetChannel", "Unable to modify NetChannel authority - NetObject is already online and NetChannelType specifies a fixed authority mode");
    return;
  }

  ReplicaChannel::SetAuthority(authority);

  // Get net object
  if(NetObject* netObject = GetNetObject())
  {
    // Is server?
    if(netObject->IsServer())
    {
      // Replicate authority changes now (if any)
      if(NetChannel* netObjectChannel = netObject->GetNetChannel("NetObject"))
        netObjectChannel->ReplicateNow();
      else
        Assert(false);
    }
  }
}
Authority::Enum NetChannel::GetAuthority() const
{
  return ReplicaChannel::GetAuthority();
}

//
// Property Management
//

String NetChannel::GetCombinedNetPropertyName(Component* component, StringParam propertyName)
{
  // Invalid component?
  if(component == nullptr)
    return String();

  // Return combined net property name ("ComponentName_PropertyName")
  return String::Format("%s_%s", ZilchVirtualTypeId(component)->Name.c_str(), propertyName.c_str());
}

bool NetChannel::HasNetProperty(Component* component, StringParam propertyName) const
{
  // Get combined net property name
  String combinedNetPropertyName = GetCombinedNetPropertyName(component, propertyName);

  // Has net property?
  return ReplicaChannel::HasReplicaProperty(combinedNetPropertyName);
}

NetProperty* NetChannel::GetNetProperty(Component* component, StringParam propertyName)
{
  // Get combined net property name
  String combinedNetPropertyName = GetCombinedNetPropertyName(component, propertyName);

  // Get net property
  return static_cast<NetProperty*>(ReplicaChannel::GetReplicaProperty(combinedNetPropertyName));
}

NetProperty* NetChannel::AddNetProperty(Component* component, Property* property, const String& netPropertyTypeName, NetPropertyConfig* netPropertyConfig)
{
  // Get combined net property name
  String combinedNetPropertyName = GetCombinedNetPropertyName(component, property->Name);

  // Already valid? (Net object is already online?)
  if(ReplicaChannel::IsValid())
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - NetObject is already online", combinedNetPropertyName.c_str()));
    return nullptr;
  }

  // Invalid component?
  if(component == nullptr)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Specified component is null", combinedNetPropertyName.c_str()));
    return nullptr;
  }

  // Invalid meta property?
  if(property == nullptr)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Specified meta property is null", combinedNetPropertyName.c_str()));
    return nullptr;
  }

  // (Should be a valid net property type)
  Assert(IsValidNetPropertyType(property->PropertyType));

  // (Net property should not already belong to another net channel)
  Assert(!GetNetObject()->DoesThisNetPropertyAlreadyBelongToAChannel(component, property->Name));

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Unable to get net peer", combinedNetPropertyName.c_str()));
    return nullptr;
  }

  // Add net property type based on meta property type
  NetPropertyType* netPropertyType = nullptr;

  // Is Cog type?
  if(property->PropertyType == ZilchTypeId(Cog))
  {
    // Get or add corresponding net property type
    netPropertyType = netPeer->GetOrAddReplicaPropertyType(netPropertyTypeName, NativeTypeOf(Cog), SerializeKnownExtendedVariant, GetComponentCogProperty, SetComponentCogProperty, netPropertyConfig);
  }
  // Is CogPath type?
  else if(property->PropertyType == ZilchTypeId(CogPath))
  {
    // Get or add corresponding net property type
    netPropertyType = netPeer->GetOrAddReplicaPropertyType(netPropertyTypeName, NativeTypeOf(CogPath), SerializeKnownExtendedVariant, GetComponentCogPathProperty, SetComponentCogPathProperty, netPropertyConfig);
  }
  // Is other (Any) type?
  else
  {
    // Get basic native type from the underlying property type
    NativeType* nativeType = ZilchTypeToBasicNativeType(property->PropertyType);
    if(!nativeType) // Unable? (The underlying property type is not a basic native type?)
    {
      // Treat the property as an opaque Any, ignoring it's true underlying type here
      // (The only purpose of knowing the property's underlying type here is so we can provide intelligent replication options on basic native types)
      nativeType = NativeTypeOf(Any);
    }

    // Get or add corresponding net property type
    netPropertyType = netPeer->GetOrAddReplicaPropertyType(netPropertyTypeName, nativeType, SerializeKnownExtendedVariant, GetComponentAnyProperty, SetComponentAnyProperty, netPropertyConfig);
  }

  // Unable to get or add net property type?
  if(!netPropertyType)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Unable to get or add NetPropertyType named '%s'", combinedNetPropertyName.c_str(), netPropertyTypeName.c_str()));
    return nullptr;
  }

  // Create property data to pass along with our replica property getter/setters
  Variant propertyData(ComponentPropertyInstanceData(property->Name, component));

  // Add net property
  NetProperty* netProperty = static_cast<NetProperty*>(ReplicaChannel::AddReplicaProperty(ReplicaPropertyPtr(new NetProperty(combinedNetPropertyName, netPropertyType, propertyData))));

  // Unable to add net property?
  if(!netProperty)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - A NetProperty with that name already exists for this NetChannel", combinedNetPropertyName.c_str()));
    return nullptr;
  }

  // Success
  return netProperty;
}
NetProperty* NetChannel::AddBasicNetProperty(const String& netPropertyName, const Variant& propertyData, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn, NetPropertyConfig* netPropertyConfig)
{
  // Already valid? (Net object is already online?)
  if(ReplicaChannel::IsValid())
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - NetObject is already online", netPropertyName.c_str()));
    return nullptr;
  }

  // TODO: Support this assert
  // // (Should be a valid net property type)
  // Assert(IsValidNetPropertyType(propertyData.StoredType));

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Unable to get net peer", netPropertyName.c_str()));
    return nullptr;
  }

  // Get or add corresponding net property type
  NetPropertyType* netPropertyType = netPeer->GetOrAddReplicaPropertyType(netPropertyName, nativeType, serializeValueFn, getValueFn, setValueFn, netPropertyConfig);

  // Unable to get or add net property type?
  if(!netPropertyType)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - Unable to get or add NetPropertyType named '%s'", netPropertyName.c_str(), netPropertyName.c_str()));
    return nullptr;
  }

  // Add net property
  NetProperty* netProperty = static_cast<NetProperty*>(ReplicaChannel::AddReplicaProperty(ReplicaPropertyPtr(new NetProperty(netPropertyName, netPropertyType, propertyData))));

  // Unable to add net property?
  if(!netProperty)
  {
    DoNotifyError("NetChannel", String::Format("Unable to add NetProperty named '%s' - A NetProperty with that name already exists for this NetChannel", netPropertyName.c_str()));
    return nullptr;
  }

  // Success
  return netProperty;
}

bool NetChannel::RemoveNetProperty(Component* component, StringParam propertyName)
{
  // Get combined net property name
  String combinedNetPropertyName = GetCombinedNetPropertyName(component, propertyName);

  // Already valid? (Net object is already online?)
  if(ReplicaChannel::IsValid())
  {
    DoNotifyError("NetChannel", String::Format("Unable to remove NetProperty named '%s' - NetObject is already online", combinedNetPropertyName.c_str()));
    return false;
  }

  // Remove net property
  return ReplicaChannel::RemoveReplicaProperty(combinedNetPropertyName);
}

void NetChannel::ClearNetProperties()
{
  // Already valid? (Net object is already online?)
  if(ReplicaChannel::IsValid())
  {
    DoNotifyError("NetChannel", String::Format("Unable to clear NetProperties - NetObject is already online"));
    return;
  }

  // Clear net properties
  ReplicaChannel::ClearReplicaProperties();
}

//---------------------------------------------------------------------------------//
//                               NetChannelType                                    //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetChannelType, builder, type)
{
  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Bind operations
  ZilchBindGetterProperty(Name);

  // Bind configuration
  ZilchBindMethod(ResetConfig);
  ZilchBindMethod(SetConfig);
  ZilchBindGetterSetterProperty(DetectOutgoingChanges);
  ZilchBindGetterSetterProperty(AcceptIncomingChanges);
  ZilchBindGetterSetterProperty(EventOnOutgoingPropertyChange);
  ZilchBindGetterSetterProperty(EventOnIncomingPropertyChange);
  ZilchBindGetterSetterProperty(AuthorityMode);
  ZilchBindGetterSetterProperty(AuthorityDefault);
  ZilchBindGetterSetterProperty(AllowRelay);
  ZilchBindGetterSetterProperty(AllowNapping);
  ZilchBindGetterSetterProperty(AwakeDuration);
  ZilchBindGetterSetterProperty(DetectionMode); 
  ZilchBindGetterSetterProperty(AwakeDetectionInterval);
  ZilchBindGetterSetterProperty(NapDetectionInterval);
  ZilchBindGetterSetterProperty(ReplicateOnOnline);
  //ZilchBindGetterSetterProperty(ReplicateOnChange);
  ZilchBindGetterSetterProperty(ReplicateOnOffline);
  ZilchBindGetterSetterProperty(SerializationMode);
  ZilchBindGetterSetterProperty(ReliabilityMode);
  ZilchBindGetterSetterProperty(TransferMode);
  ZilchBindGetterSetterProperty(AccurateTimestampOnChange);
}

NetChannelType::NetChannelType(const String& name)
  : ReplicaChannelType(name)
{
  ResetConfig();
}

NetChannelType::~NetChannelType()
{
}

//
// Operations
//

const String& NetChannelType::GetName() const
{
  return ReplicaChannelType::GetName();
}

//
// Configuration
//

void NetChannelType::ResetConfig()
{
  // Not valid yet?
  if(!IsValid())
  {
    // Set non-runtime config options
    SetAuthorityMode();
    SetAwakeDetectionInterval();
    SetNapDetectionInterval();
    SetReplicateOnOnline();
    SetReplicateOnChange();
    SetReplicateOnOffline();
    SetSerializationMode();
    SetTransferMode();
  }

  // Set runtime config options
  SetDetectOutgoingChanges();
  SetAcceptIncomingChanges();
  SetEventOnOutgoingPropertyChange();
  SetEventOnIncomingPropertyChange();
  SetAuthorityDefault();
  SetAllowRelay();
  SetAllowNapping();
  SetAwakeDuration();
  SetDetectionMode();
  SetReliabilityMode();
  SetAccurateTimestampOnChange();
}

void NetChannelType::SetConfig(NetChannelConfig* netChannelConfig)
{
  // Not valid yet?
  if(!IsValid())
  {
    // Set non-runtime config options
    SetAuthorityMode(netChannelConfig->mAuthorityMode);
    SetAwakeDetectionInterval(netChannelConfig->mAwakeDetectionInterval);
    SetNapDetectionInterval(netChannelConfig->mNapDetectionInterval);
    SetReplicateOnOnline(netChannelConfig->mReplicateOnOnline);
    SetReplicateOnChange(netChannelConfig->mReplicateOnChange);
    SetReplicateOnOffline(netChannelConfig->mReplicateOnOffline);
    SetSerializationMode(netChannelConfig->mSerializationMode);
    SetTransferMode(netChannelConfig->mTransferMode);
  }

  // Set runtime config options
  SetDetectOutgoingChanges(netChannelConfig->mDetectOutgoingChanges);
  SetAcceptIncomingChanges(netChannelConfig->mAcceptIncomingChanges);
  SetEventOnOutgoingPropertyChange(netChannelConfig->mEventOnOutgoingPropertyChange);
  SetEventOnIncomingPropertyChange(netChannelConfig->mEventOnIncomingPropertyChange);
  SetAuthorityDefault(netChannelConfig->mAuthorityDefault);
  SetAllowRelay(netChannelConfig->mAllowRelay);
  SetAllowNapping(netChannelConfig->mAllowNapping);
  SetAwakeDuration(netChannelConfig->mAwakeDuration);
  SetDetectionMode(netChannelConfig->mDetectionMode);
  SetReliabilityMode(netChannelConfig->mReliabilityMode);
  SetAccurateTimestampOnChange(netChannelConfig->mAccurateTimestampOnChange);
}

void NetChannelType::SetDetectOutgoingChanges(bool detectOutgoingChanges)
{
  ReplicaChannelType::SetDetectOutgoingChanges(detectOutgoingChanges);
}
bool NetChannelType::GetDetectOutgoingChanges() const
{
  return ReplicaChannelType::GetDetectOutgoingChanges();
}

void NetChannelType::SetAcceptIncomingChanges(bool acceptIncomingChanges)
{
  ReplicaChannelType::SetAcceptIncomingChanges(acceptIncomingChanges);
}
bool NetChannelType::GetAcceptIncomingChanges() const
{
  return ReplicaChannelType::GetAcceptIncomingChanges();
}

void NetChannelType::SetEventOnOutgoingPropertyChange(bool eventOnOutgoingPropertyChange)
{
  ReplicaChannelType::SetNotifyOnOutgoingPropertyChange(eventOnOutgoingPropertyChange);
}
bool NetChannelType::GetEventOnOutgoingPropertyChange() const
{
  return ReplicaChannelType::GetNotifyOnOutgoingPropertyChange();
}

void NetChannelType::SetEventOnIncomingPropertyChange(bool eventOnIncomingPropertyChange)
{
  ReplicaChannelType::SetNotifyOnIncomingPropertyChange(eventOnIncomingPropertyChange);
}
bool NetChannelType::GetEventOnIncomingPropertyChange() const
{
  return ReplicaChannelType::GetNotifyOnIncomingPropertyChange();
}

void NetChannelType::SetAuthorityMode(AuthorityMode::Enum authorityMode)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  ReplicaChannelType::SetAuthorityMode(authorityMode);
}
AuthorityMode::Enum NetChannelType::GetAuthorityMode() const
{
  return ReplicaChannelType::GetAuthorityMode();
}

void NetChannelType::SetAuthorityDefault(Authority::Enum authorityDefault)
{
  ReplicaChannelType::SetAuthorityDefault(authorityDefault);
}
Authority::Enum NetChannelType::GetAuthorityDefault() const
{
  return ReplicaChannelType::GetAuthorityDefault();
}

void NetChannelType::SetAllowRelay(bool allowRelay)
{
  ReplicaChannelType::SetAllowRelay(allowRelay);
}
bool NetChannelType::GetAllowRelay() const
{
  return ReplicaChannelType::GetAllowRelay();
}

void NetChannelType::SetAllowNapping(bool allowNapping)
{
  ReplicaChannelType::SetAllowNapping(allowNapping);
}
bool NetChannelType::GetAllowNapping() const
{
  return ReplicaChannelType::GetAllowNapping();
}

void NetChannelType::SetAwakeDuration(uint awakeDuration)
{
  ReplicaChannelType::SetAwakeDuration(awakeDuration);
}
uint NetChannelType::GetAwakeDuration() const
{
  return ReplicaChannelType::GetAwakeDuration();
}

void NetChannelType::SetDetectionMode(DetectionMode::Enum detectionMode)
{
  ReplicaChannelType::SetDetectionMode(detectionMode);
}
DetectionMode::Enum NetChannelType::GetDetectionMode() const
{
  return ReplicaChannelType::GetDetectionMode();
}

void NetChannelType::SetAwakeDetectionInterval(uint awakeDetectionInterval)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  ReplicaChannelType::SetAwakeDetectionInterval(awakeDetectionInterval);
}
uint NetChannelType::GetAwakeDetectionInterval() const
{
  return ReplicaChannelType::GetAwakeDetectionInterval();
}

void NetChannelType::SetNapDetectionInterval(uint napDetectionInterval)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  ReplicaChannelType::SetNapDetectionInterval(napDetectionInterval);
}
uint NetChannelType::GetNapDetectionInterval() const
{
  return ReplicaChannelType::GetNapDetectionInterval();
}

// Create serialization flags
static const SerializationFlags::Type sCreateFlags = (SerializationFlags::OnSpawn
                                                    | SerializationFlags::OnCloneEmplace
                                                    | SerializationFlags::OnCloneSpawn);

// Change serialization flags
static const SerializationFlags::Type sChangeFlags = (SerializationFlags::OnChange);

// Destroy serialization flags
static const SerializationFlags::Type sDestroyFlags = (SerializationFlags::OnForget
                                                     | SerializationFlags::OnDestroy);

void NetChannelType::SetReplicateOnOnline(bool replicateOnOnline)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  // Set serialization flags
  uint flags = replicateOnOnline ? (ReplicaChannelType::GetSerializationFlags() |  sCreateFlags)
                                 : (ReplicaChannelType::GetSerializationFlags() & ~sCreateFlags);
  ReplicaChannelType::SetSerializationFlags(flags);
}
bool NetChannelType::GetReplicateOnOnline() const
{
  return ReplicaChannelType::GetSerializationFlags() & sCreateFlags;
}

void NetChannelType::SetReplicateOnChange(bool replicateOnChange)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  // Set serialization flags
  uint flags = replicateOnChange ? (ReplicaChannelType::GetSerializationFlags() |  sChangeFlags)
                                 : (ReplicaChannelType::GetSerializationFlags() & ~sChangeFlags);
  ReplicaChannelType::SetSerializationFlags(flags);
}
bool NetChannelType::GetReplicateOnChange() const
{
  return ReplicaChannelType::GetSerializationFlags() & sChangeFlags;
}

void NetChannelType::SetReplicateOnOffline(bool replicateOnOffline)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  // Set serialization flags
  uint flags = replicateOnOffline ? (ReplicaChannelType::GetSerializationFlags() |  sDestroyFlags)
                                  : (ReplicaChannelType::GetSerializationFlags() & ~sDestroyFlags);
  ReplicaChannelType::SetSerializationFlags(flags);
}
bool NetChannelType::GetReplicateOnOffline() const
{
  return ReplicaChannelType::GetSerializationFlags() & sDestroyFlags;
}

void NetChannelType::SetSerializationMode(SerializationMode::Enum serializationMode)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  ReplicaChannelType::SetSerializationMode(serializationMode);
}
SerializationMode::Enum NetChannelType::GetSerializationMode() const
{
  return ReplicaChannelType::GetSerializationMode();
}

void NetChannelType::SetReliabilityMode(ReliabilityMode::Enum reliabilityMode)
{
  ReplicaChannelType::SetReliabilityMode(reliabilityMode);
}
ReliabilityMode::Enum NetChannelType::GetReliabilityMode() const
{
  return ReplicaChannelType::GetReliabilityMode();
}

void NetChannelType::SetTransferMode(TransferMode::Enum transferMode)
{
  // Already valid?
  if(ReplicaChannelType::IsValid())
  {
    // Unable to modify configuration
    DoNotifyError("NetChannelType", "Unable to modify this NetChannelType configuration option at game runtime");
    return;
  }

  ReplicaChannelType::SetTransferMode(transferMode);
}
TransferMode::Enum NetChannelType::GetTransferMode() const
{
  return ReplicaChannelType::GetTransferMode();
}

void NetChannelType::SetAccurateTimestampOnChange(bool accurateTimestampOnChange)
{
  ReplicaChannelType::SetAccurateTimestampOnChange(accurateTimestampOnChange);
}
bool NetChannelType::GetAccurateTimestampOnChange() const
{
  return ReplicaChannelType::GetAccurateTimestampOnChange();
}

//---------------------------------------------------------------------------------//
//                              NetChannelConfig                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetChannelConfig, builder, type)
{
  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind data members
  ZilchBindFieldProperty(mDetectOutgoingChanges);
  ZilchBindFieldProperty(mAcceptIncomingChanges);
  ZilchBindFieldProperty(mEventOnOutgoingPropertyChange);
  ZilchBindFieldProperty(mEventOnIncomingPropertyChange);
  ZilchBindFieldProperty(mAuthorityMode); 
  ZilchBindFieldProperty(mAuthorityDefault);
  ZilchBindFieldProperty(mAllowRelay);
  ZilchBindFieldProperty(mAllowNapping);
  ZilchBindFieldProperty(mAwakeDuration);
  ZilchBindFieldProperty(mDetectionMode);
  ZilchBindFieldProperty(mAwakeDetectionInterval);
  ZilchBindFieldProperty(mNapDetectionInterval);
  ZilchBindFieldProperty(mReplicateOnOnline);
  //ZilchBindFieldProperty(mReplicateOnChange);
  ZilchBindFieldProperty(mReplicateOnOffline);
  ZilchBindFieldProperty(mSerializationMode);
  ZilchBindFieldProperty(mReliabilityMode);
  ZilchBindFieldProperty(mTransferMode);
  ZilchBindFieldProperty(mAccurateTimestampOnChange);
}

void NetChannelConfig::Serialize(Serializer& stream)
{
  // Serialize data members
  SerializeNameDefault(mDetectOutgoingChanges, true);
  SerializeNameDefault(mAcceptIncomingChanges, true);
  SerializeNameDefault(mEventOnOutgoingPropertyChange, true);
  SerializeNameDefault(mEventOnIncomingPropertyChange, true);
  SerializeEnumNameDefault(AuthorityMode, mAuthorityMode, AuthorityMode::Fixed);
  SerializeEnumNameDefault(Authority, mAuthorityDefault, Authority::Server);
  SerializeNameDefault(mAllowRelay, true);
  SerializeNameDefault(mAllowNapping, true);
  SerializeNameDefault(mAwakeDuration, uint(10));
  SerializeEnumNameDefault(DetectionMode, mDetectionMode, DetectionMode::Manumatic);
  SerializeNameDefault(mAwakeDetectionInterval, uint(1));
  SerializeNameDefault(mNapDetectionInterval, uint(2));
  SerializeNameDefault(mReplicateOnOnline, true);
  SerializeNameDefault(mReplicateOnChange, true);
  SerializeNameDefault(mReplicateOnOffline, true);
  SerializeEnumNameDefault(SerializationMode, mSerializationMode, SerializationMode::Changed);
  SerializeEnumNameDefault(ReliabilityMode, mReliabilityMode, ReliabilityMode::Reliable);
  SerializeEnumNameDefault(TransferMode, mTransferMode, TransferMode::Ordered);
  SerializeNameDefault(mAccurateTimestampOnChange, false);
}

//
// Operations
//

const String& NetChannelConfig::GetName() const
{
  return Name;
}

//---------------------------------------------------------------------------------//
//                           NetChannelConfigManager                               //
//---------------------------------------------------------------------------------//

ImplementResourceManager(NetChannelConfigManager, NetChannelConfig);
NetChannelConfigManager::NetChannelConfigManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("NetChannelConfig", new TextDataFileLoader<NetChannelConfigManager>());
  mCategory = "Networking";
  DefaultResourceName = "Default";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.NetChannelConfig.data"));
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

} // namespace Zero
