///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                 NetChannel                                      //
//---------------------------------------------------------------------------------//

/// Network Channel.
/// Manages the replication of a set of properties on the network.
class NetChannel : public SafeId32Object, public ReplicaChannel
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetChannel(const String& name, NetChannelType* netChannelType);

  /// Destructor.
  ~NetChannel();

  //
  // Operations
  //

  /// Operating net peer.
  NetPeer* GetNetPeer() const;

  /// Operating net object.
  NetObject* GetNetObject() const;

  /// Net channel name.
  const String& GetName() const;

  /// Operating net channel type.
  NetChannelType* GetNetChannelType() const;

  /// Returns true if this net channel is scheduled for change observation, else false.
  bool IsScheduled() const;

  /// Returns true if the net channel is currently napping (performing change detection on longer intervals), else false.
  bool IsNapping() const;

  /// Forces the net channel to stop napping immediately.
  void WakeUp();
  /// Forces the net channel to start napping immediately.
  void TakeNap();

  /// Manual change flag (checked upon manual change observation).
  void SetChangeFlag(bool changeFlag = true);
  bool GetChangeFlag() const;

  /// Replicates net property changes immediately (only if changes are detected).
  /// Will also update nap state as configured.
  /// Returns true if changes were replicated, else false.
  bool ReplicateNow();

  /// Timestamp indicating when this net channel was last changed, else 0.
  float GetLastChangeTimestamp() const;

  /// Elapsed time passed since this net channel was last changed, else 0.
  float GetLastChangeTimePassed() const;

  /// Controls which peer has the authority to observe and replicate property changes.
  /// (Client: Indicates both the client and server are allowed to observe and replicate property changes)
  /// (Server: Indicates only the server is allowed to observe and replicate property changes)
  /// Only a single client, specified by NetObject::NetUserOwnerPeerId, may possess client authority at any given time.
  /// The server is still responsible for relaying contained property changes to other clients, but will not replicate contained property changes back to the authority client.
  /// However, the server is also still responsible for other replication commands (such as object creation/destruction), and these WILL be replicated to the authority client.
  void SetAuthority(Authority::Enum authority = Authority::Server);
  Authority::Enum GetAuthority() const;

  //
  // Property Management
  //

  /// Returns the combined net property name ("ComponentName_PropertyName"), else String().
  static String GetCombinedNetPropertyName(Component* component, StringParam propertyName);

  /// [Client/Server] Returns true if the net object has the specified net property, else false.
  bool HasNetProperty(Component* component, StringParam propertyName) const;

  /// [Client/Server] Returns the specified net property, else nullptr.
  NetProperty* GetNetProperty(Component* component, StringParam propertyName);

  /// [Client/Server] Adds the net property.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  /// Returns the net property if successful, else nullptr (a net property of that name already exists).
  NetProperty* AddNetProperty(Component* component, Property* property, const String& netPropertyTypeName, NetPropertyConfig* netPropertyConfig = nullptr);
  template <typename T>
  NetProperty* AddBasicNetProperty(const String& netPropertyName, T& propertyData,  NetPropertyConfig* netPropertyConfig = nullptr)
  {
    return AddBasicNetProperty(netPropertyName, Variant(&propertyData), NativeTypeOf(T), SerializeKnownExtendedVariant, GetDataValue<T>, SetDataValue<T>, netPropertyConfig);
  }
  NetProperty* AddBasicNetProperty(const String& netPropertyName, const Variant& propertyData, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn, NetPropertyConfig* netPropertyConfig = nullptr);

  /// [Client/Server] Removes the specified net property.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  /// Returns true if successful, else false (a net property of that name could not be found).
  bool RemoveNetProperty(Component* component, StringParam propertyName);

  /// [Client/Server] Removes all net properties.
  /// (Cannot be modified after net object component initialization)
  /// (Must not differ between client/server peers!)
  void ClearNetProperties();
};

//---------------------------------------------------------------------------------//
//                               NetChannelType                                    //
//---------------------------------------------------------------------------------//

/// Network Channel Type.
/// Configures the replication of a set of properties on the network.
class NetChannelType : public SafeId32Object, public ReplicaChannelType
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetChannelType(const String& name);

  /// Destructor.
  ~NetChannelType();

  //
  // Operations
  //

  /// Net channel type name.
  const String& GetName() const;

  //
  // Configuration
  //

  /// Resets all configuration settings.
  void ResetConfig();

  /// Sets all configuration settings according to the specified NetChannelConfig resource.
  void SetConfig(NetChannelConfig* netChannelConfig);

  /// Controls whether or not net channels should detect outgoing changes.
  void SetDetectOutgoingChanges(bool detectOutgoingChanges = true);
  bool GetDetectOutgoingChanges() const;

  /// Controls whether or not net channels should accept incoming changes.
  void SetAcceptIncomingChanges(bool acceptIncomingChanges = true);
  bool GetAcceptIncomingChanges() const;

  /// Controls whether or not net channels should dispatch NetChannelOutgoingPropertyChange when an outgoing net property change is detected.
  void SetEventOnOutgoingPropertyChange(bool eventOnOutgoingPropertyChange = true);
  bool GetEventOnOutgoingPropertyChange() const;

  /// Controls whether or not net channels should dispatch NetChannelIncomingPropertyChange when an incoming net property change is accepted.
  void SetEventOnIncomingPropertyChange(bool eventOnIncomingPropertyChange = true);
  bool GetEventOnIncomingPropertyChange() const;

  /// Controls when net channels can modify their change authority.
  /// (Dynamic: Authority may be modified at any time, even after a net object is brought online)
  /// (Fixed: Authority may be modified only before a net object is brought online)
  /// (Cannot be modified at game runtime)
  void SetAuthorityMode(AuthorityMode::Enum authorityMode = AuthorityMode::Fixed);
  AuthorityMode::Enum GetAuthorityMode() const;

  /// Controls which peer has the authority to observe and replicate property changes on each net channel by default.
  /// (Client: Indicates both the client and server are allowed to observe and replicate property changes)
  /// (Server: Indicates only the server is allowed to observe and replicate property changes)
  /// Only a single client, specified by NetObject::NetUserOwnerPeerId, may possess client authority at any given time.
  /// The server is still responsible for relaying contained property changes to other clients, but will not replicate contained property changes back to the authority client.
  /// However, the server is also still responsible for other replication commands (such as object creation/destruction), and these WILL be replicated to the authority client.
  void SetAuthorityDefault(Authority::Enum authorityDefault = Authority::Server);
  Authority::Enum GetAuthorityDefault() const;

  /// Controls whether or not net channels will have their changes immediately broadcast to all relevant, incidental peers (if any) once received.
  /// (Enabling this allows a server to automatically relay client authoritative changes to other clients, otherwise this must be done manually using NetChannel::ReplicateNow)
  void SetAllowRelay(bool allowRelay = true);
  bool GetAllowRelay() const;

  /// Controls whether or not net channels may nap (perform change detection on longer intervals) if they haven't changed in a while.
  void SetAllowNapping(bool allowNapping = true);
  bool GetAllowNapping() const;

  /// Controls the frame duration following the last detected change in which net channels are considered actively changing and will be kept awake.
  void SetAwakeDuration(uint awakeDuration = 10);
  uint GetAwakeDuration() const;

  /// Controls how net channel changes are detected.
  /// (Assume: Assumes something has changed)
  /// (Manual: Detects changes manually using change flags)
  /// (Automatic: Detects changes automatically using comparisons)
  /// (Manumatic: Detects changes manually using change flags and automatically using comparisons)
  void SetDetectionMode(DetectionMode::Enum detectionMode = DetectionMode::Manumatic);
  DetectionMode::Enum GetDetectionMode() const;

  /// Controls the frame interval in which awake net channels are observed for changes.
  /// (Cannot be modified at game runtime)
  void SetAwakeDetectionInterval(uint awakeDetectionInterval = 1);
  uint GetAwakeDetectionInterval() const;

  /// Controls the frame interval in which napping net channels are observed for changes.
  /// (Cannot be modified at game runtime)
  void SetNapDetectionInterval(uint napDetectionInterval = 2);
  uint GetNapDetectionInterval() const;

  /// Controls whether or not the net channel will be replicated when the net object comes online.
  /// If enabled, all net channel property values are guaranteed to be set immediately before the NetObjectOnline event.
  /// (Cannot be modified at game runtime)
  void SetReplicateOnOnline(bool replicateOnOnline = true);
  bool GetReplicateOnOnline() const;

  /// Controls whether or not the net channel will be replicated on net property change.
  /// (Cannot be modified at game runtime)
  void SetReplicateOnChange(bool replicateOnChange = true);
  bool GetReplicateOnChange() const;

  /// Controls whether or not the net channel will be replicated when the net object goes offline.
  /// If enabled, all net channel property values are guaranteed to be set immediately before the NetObjectOffline event.
  /// (Cannot be modified at game runtime)
  void SetReplicateOnOffline(bool replicateOnOffline = true);
  bool GetReplicateOnOffline() const;

  /// Controls how net channels are serialized.
  /// (All: Serialize all net properties)
  /// (Changed: Serialize only net properties that have changed, using bit flags in between)
  /// (Cannot be modified at game runtime)
  void SetSerializationMode(SerializationMode::Enum serializationMode = SerializationMode::Changed);
  SerializationMode::Enum GetSerializationMode() const;

  /// Controls whether or not net channel changes will be retransmitted should they get lost over the network.
  /// (Unreliable: Lost changes are not retransmitted)
  /// (Reliable: Lost changes are retransmitted)
  void SetReliabilityMode(ReliabilityMode::Enum reliabilityMode = ReliabilityMode::Reliable);
  ReliabilityMode::Enum GetReliabilityMode() const;

  /// Controls how net channel changes are to be ordered and released once received.
  /// (Immediate: Changes are released immediately once received, including late changes)
  /// (Sequenced: Changes are released immediately once received, discarding late changes)
  /// (Ordered: Changes are released immediately once preceding late changes have been received; forces all changes to be sent reliably)
  /// (Cannot be modified at game runtime)
  void SetTransferMode(TransferMode::Enum transferMode = TransferMode::Ordered);
  TransferMode::Enum GetTransferMode() const;

  /// Controls whether or not the net channel will serialize an accurate timestamp value when changed, or will instead accept an estimated timestamp value.
  /// (This setting may be overridden for net channels belonging to a specific net object by enabling the corresponding net object setting)
  void SetAccurateTimestampOnChange(bool accurateTimestampOnChange = false);
  bool GetAccurateTimestampOnChange() const;
};

//---------------------------------------------------------------------------------//
//                              NetChannelConfig                                   //
//---------------------------------------------------------------------------------//

/// Network Channel Configuration.
/// Defines a configuration for the replication of a set of properties on the network.
class NetChannelConfig : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  //
  // Data Resource Interface
  //

  /// Serializes the net channel configuration resource.
  void Serialize(Serializer& stream) override;

  //
  // Operations
  //

  /// Net channel configuration name.
  const String& GetName() const;

  //
  // Configuration Data
  //

  /// Controls whether or not net channels should detect outgoing changes.
  bool mDetectOutgoingChanges;

  /// Controls whether or not net channels should accept incoming changes.
  bool mAcceptIncomingChanges;

  /// Controls whether or not net channels should dispatch NetChannelOutgoingPropertyChange when an outgoing net property change is detected.
  bool mEventOnOutgoingPropertyChange;

  /// Controls whether or not net channels should dispatch NetChannelIncomingPropertyChange when an incoming net property change is accepted.
  bool mEventOnIncomingPropertyChange;

  /// Controls when net channels can modify their change authority.
  /// (Dynamic: Authority may be modified at any time, even after a net object is brought online)
  /// (Fixed: Authority may be modified only before a net object is brought online)
  AuthorityMode::Enum mAuthorityMode;

  /// Controls which peer has the authority to observe and replicate property changes on each net channel by default.
  /// (Client: Indicates both the client and server are allowed to observe and replicate property changes)
  /// (Server: Indicates only the server is allowed to observe and replicate property changes)
  /// Only a single client, specified by NetObject::NetUserOwnerPeerId, may possess client authority at any given time.
  /// The server is still responsible for relaying contained property changes to other clients, but will not replicate contained property changes back to the authority client.
  /// However, the server is also still responsible for other replication commands (such as object creation/destruction), and these WILL be replicated to the authority client.
  Authority::Enum mAuthorityDefault;

  /// Controls whether or not net channels will have their changes immediately broadcast to all relevant, incidental peers (if any) once received.
  /// (Enabling this allows a server to automatically relay client authoritative changes to other clients, otherwise this must be done manually using NetChannel::ReplicateNow)
  bool mAllowRelay;

  /// Controls whether or not net channels may nap (perform change detection on longer intervals) if they haven't changed in a while.
  bool mAllowNapping;

  /// Controls the frame duration following the last detected change in which net channels are considered actively changing and will be kept awake.
  uint mAwakeDuration;

  /// Controls how net channel changes are detected.
  /// (Assume: Assumes something has changed)
  /// (Manual: Detects changes manually using change flags)
  /// (Automatic: Detects changes automatically using comparisons)
  /// (Manumatic: Detects changes manually using change flags and automatically using comparisons)
  DetectionMode::Enum mDetectionMode;

  /// Controls the frame interval in which awake net channels are observed for changes.
  uint mAwakeDetectionInterval;

  /// Controls the frame interval in which napping net channels are observed for changes.
  uint mNapDetectionInterval;

  /// Controls whether or not the net channel will be replicated when the net object comes online.
  /// If enabled, all net channel property values are guaranteed to be set immediately before the NetObjectOnline event.
  bool mReplicateOnOnline;

  /// Controls whether or not the net channel will be replicated on net property change.
  bool mReplicateOnChange;

  /// Controls whether or not the net channel will be replicated when the net object goes offline.
  /// If enabled, all net channel property values are guaranteed to be set immediately before the NetObjectOffline event.
  bool mReplicateOnOffline;

  /// Controls how net channels are serialized.
  /// (All: Serialize all net properties)
  /// (Changed: Serialize only net properties that have changed, using bit flags in between)
  SerializationMode::Enum mSerializationMode;

  /// Controls whether or not net channel changes will be retransmitted should they get lost over the network.
  /// (Unreliable: Lost changes are not retransmitted)
  /// (Reliable: Lost changes are retransmitted)
  ReliabilityMode::Enum mReliabilityMode;

  /// Controls how net channel changes are to be ordered and released once received.
  /// (Immediate: Changes are released immediately once received, including late changes)
  /// (Sequenced: Changes are released immediately once received, discarding late changes)
  /// (Ordered: Changes are released immediately once preceding late changes have been received; forces all changes to be sent reliably)
  TransferMode::Enum mTransferMode;

  /// Controls whether or not the net channel will serialize an accurate timestamp value when changed, or will instead accept an estimated timestamp value.
  /// (This setting may be overridden for net channels belonging to a specific net object by enabling the corresponding net object setting)
  bool mAccurateTimestampOnChange;
};

//---------------------------------------------------------------------------------//
//                           NetChannelConfigManager                               //
//---------------------------------------------------------------------------------//

/// Manages all NetChannelConfig resources.
class NetChannelConfigManager : public ResourceManager
{
public:
  /// Initialize ResourceManager.
  DeclareResourceManager(NetChannelConfigManager, NetChannelConfig);

  /// Constructor.
  NetChannelConfigManager(BoundType* resourceType);
};

} // namespace Zero
