///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Editor Tags
namespace Tags
{
  DeclareTag(Networking);
}

/// Constants.
static const String cNetProperty  = "NetProperty";
static const String cNetPeerId    = "NetPeerId";
static const String cScriptSource = "ScriptSource";
static const String cEventId      = "EventId";
static const String cGameSetup    = "GameSetup";

/// Returns true if the specified property instance is a supported net property, else false.
bool IsValidNetProperty(Property* property);
/// Returns true if the specified property type is a supported net property type, else false.
bool IsValidNetPropertyType(Type* propertyType);
bool IsValidNetPropertyType(NativeType* propertyType);
/// Returns true if the specified property type is a supported net peer ID property type, else false.
bool IsValidNetPeerIdPropertyType(Type* propertyType);

//---------------------------------------------------------------------------------//
//                            NetObject Configuration                              //
//---------------------------------------------------------------------------------//

/// Use Archetype "ResourceId:Name" String as ReplicaType?
/// Otherwise we will use ResourceId u64 as ReplicaType (much more efficient).
/// This impacts bandwidth performance when first spawning/cloning new NetObject types to remote peers.
#ifdef ZeroDebug
  #define NETOBJECT_USE_RESOURCE_ID_NAME_STRING
#endif

//---------------------------------------------------------------------------------//
//                                 Network Types                                   //
//---------------------------------------------------------------------------------//

/// Basic network types (subset of basic native types).
DeclareEnum21(BasicNetType,
  Other,
  Byte,
  Boolean,
  Boolean2,
  Boolean3,
  Boolean4,
  Integer,
  Integer2,
  Integer3,
  Integer4,
  Real,
  Real2,
  Real3,
  Real4,
  Quaternion,
  Real2x2,
  Real3x3,
  Real4x4,
  DoubleInteger,
  DoubleReal,
  String);

/// Unique Network Peer Identifier.
typedef uint NetPeerId;

/// Unique Network User Identifier.
typedef uint NetUserId;

/// Unique Network Object Identifier.
typedef uint NetObjectId;

/// Unique Family Tree Identifier.
typedef uint FamilyTreeId;

/// Stores component property instance data.
/// Used when getting / setting values on a specific component's property instance during replication.
struct ComponentPropertyInstanceData
{
  /// Constructor.
  ComponentPropertyInstanceData(String propertyName = String(), Component* component = nullptr);

  // Data
  String     mPropertyName;
  Component* mComponent;
};

//
// NetChannel Authority Property Getter / Setter
//

// Serialized Data Type: Any (storing type Authority::Enum)
Variant GetNetChannelAuthorityProperty(const Variant& propertyData);
void SetNetChannelAuthorityProperty(const Variant& value, Variant& propertyData);

//
// NetObject Parent Property Getter / Setter
//

// Serialized Data Type: NetObjectId
Variant GetNetObjectParentProperty(const Variant& propertyData);
void SetNetObjectParentProperty(const Variant& value, Variant& propertyData);

//
// Component Cog Property Getter / Setter
//

// Serialized Data Type: NetObjectId
Variant GetComponentCogProperty(const Variant& propertyData);
void SetComponentCogProperty(const Variant& value, Variant& propertyData);

//
// Component CogPath Property Getter / Setter
//

// Serialized Data Type: String
Variant GetComponentCogPathProperty(const Variant& propertyData);
void SetComponentCogPathProperty(const Variant& value, Variant& propertyData);

//
// Component Any Property Getter / Setter
//

// Serialized Data Type: A Basic Native Type or Any
Variant GetComponentAnyProperty(const Variant& propertyData);
void SetComponentAnyProperty(const Variant& value, Variant& propertyData);

//
// Helper Methods
//

/// Returns the BasicNativeType enum equivalent of the BasicNetType enum value, else BasicNativeType::Unknown.
BasicNativeType::Enum BasicNetworkToNativeTypeEnum(BasicNetType::Enum value);
/// Returns the BasicNetType enum equivalent of the BasicNativeType enum value, else BasicNetType::Other.
BasicNetType::Enum BasicNativeToNetworkTypeEnum(BasicNativeType::Enum value);

/// Returns true if the event has a property with a NetPeerId attribute, else false.
bool HasNetPeerIdProperty(Event* event);
/// Sets all properties with a NetPeerId attribute in the event.
void SetNetPeerIdProperties(Event* event, NetPeerId netPeerId);

/// Sets the Cog net property as a NetObjectId, else nullptr.
void SetNetPropertyCogAsNetObjectId(Property* property, const Any& instance, NetPeer* netPeer, NetObjectId netObjectId);
/// Returns the Cog net property as a NetObjectId, else 0.
NetObjectId GetNetPropertyCogAsNetObjectId(Property* property, const Any& instance);

/// Network specification.
DeclareEnum2(Network,
  LAN,       /// The local area network.
  Internet); ///< The internet.

/// Network refresh result.
DeclareEnum4(NetRefreshResult,
  NoResponse,
  IndirectBasicHostInfo,
  DirectBasicHostInfo,
  ExtraHostInfo);

/// Network user add response.
DeclareEnum2(NetUserAddResponse,
  Accept, /// User add request accepted.
  Deny);  ///< User add request denied.

/// Network user add response range.
static const NetUserAddResponse::Type NetUserAddResponseMin = NetUserAddResponse::Accept;
static const NetUserAddResponse::Type NetUserAddResponseMax = NetUserAddResponse::Deny;

/// Network user add response bits.
static const Bits NetUserAddResponseBits = BITS_NEEDED_TO_REPRESENT(NetUserAddResponseMax);

/// NetPeer protocol message types.
DeclareEnum12(NetPeerMessageType,
  NetEvent,               /// Network dispatch event.
  NetUserAddRequest,      /// Network user add request.
  NetUserAddResponse,     /// Network user add response.
  NetUserRemoveRequest,   /// Network user remove request.
  NetLevelLoadStarted,    /// Network level load started.
  NetLevelLoadFinished,   /// Network level load finished.
  NetGameLoadStarted,     /// Network game load started.
  NetGameLoadFinished,    /// Network game load finished.
  NetHostPing,            /// Network host ping.
  NetHostPong,            /// Network host pong.
  NetHostRecordList,      /// Network host record list.
  NetHostPublish);        ///< Network host publish.

//
// NetPeer Protocol Message Types
//

//---------------------------------------------------------------------------------//
//                             NetUserAddRequestData                               //
//---------------------------------------------------------------------------------//

/// Network user add request protocol message data.
struct NetUserAddRequestData
{
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

/// Serializes network user add request data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserAddRequestData& netUserAddRequestData);

//---------------------------------------------------------------------------------//
//                             NetUserAddResponseData                              //
//---------------------------------------------------------------------------------//

/// Network user add response protocol message data.
struct NetUserAddResponseData
{
  /// User add response.
  NetUserAddResponse::Enum mAddResponse;
  /// Optional, network user identifier (set only if accepted).
  NetUserId                mNetUserId;
  /// Optional, event bundle data.
  BitStream                mEventBundleData;
};

/// Serializes network user add response data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserAddResponseData& netUserAddResponseData);

//---------------------------------------------------------------------------------//
//                            NetUserRemoveRequestData                             //
//---------------------------------------------------------------------------------//

/// Network user remove request protocol message data.
struct NetUserRemoveRequestData
{
  /// Network user identifier.
  NetUserId mNetUserId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

/// Serializes network user remove request data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetUserRemoveRequestData& netUserRemoveRequestData);

//---------------------------------------------------------------------------------//
//                            NetLevelLoadStartedData                              //
//---------------------------------------------------------------------------------//

/// Network level load started protocol message data.
struct NetLevelLoadStartedData
{
  /// Network space object identifier.
  NetObjectId mNetSpaceObjectId;
  /// Level resource identifier.
  u64         mLevelResourceId;
};

/// Serializes network level load started protocol data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetLevelLoadStartedData& netLevelLoadStartedData);

//---------------------------------------------------------------------------------//
//                            NetLevelLoadFinishedData                             //
//---------------------------------------------------------------------------------//

/// Network level load finished protocol message data.
struct NetLevelLoadFinishedData
{
  /// Network space object identifier.
  NetObjectId mNetSpaceObjectId;
};

/// Serializes network level load finished protocol data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetLevelLoadFinishedData& netLevelLoadFinishedData);

//---------------------------------------------------------------------------------//
//                              NetConnectRequestData                              //
//---------------------------------------------------------------------------------//

/// Network connect link request protocol message data.
struct NetConnectRequestData
{
  /// Pending user add request count.
  uint      mAddUserRequestCount;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

/// Serializes network connect link request protocol data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetConnectRequestData& netConnectRequestData);

//---------------------------------------------------------------------------------//
//                                 NetHostPingData                                 //
//---------------------------------------------------------------------------------//

/// Network host ping protocol message data.
struct NetHostPingData
{
  /// Unique project identifier.
  Guid      mProjectGuid;
  /// Unique ping request identifier.
  uint      mPingId;
  /// Unique send attempt identifier.
  uint      mSendAttemptId;
  /// Manager id, groups pings.
  uint      mManagerId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

/// Serializes network host ping protocol data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPingData& netHostPingData);

//---------------------------------------------------------------------------------//
//                                 NetHostPongData                                 //
//---------------------------------------------------------------------------------//

/// Network host pong protocol message data.
struct NetHostPongData
{
  /// Unique project identifier.
  Guid      mProjectGuid;
  /// Unique ping request identifier.
  uint      mPingId;
  /// Unique send attempt identifier.
  uint      mSendAttemptId;
  /// Manager id, describes which ping manager this pong belongs to.
  uint      mManagerId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

/// Serializes network host pong protocol data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPongData& netHostPongData);

//---------------------------------------------------------------------------------//
//                                NetHostRecordList                                //
//---------------------------------------------------------------------------------//

/// Network host record list message data.
struct NetHostRecordListData
{
  // Network host records
  Array<NetHostRecord> mNetHostRecords;
};

/// Serializes network host record list data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostRecordListData& netHostRecordList);

//---------------------------------------------------------------------------------//
//                                NetHostPublishData                               //
//---------------------------------------------------------------------------------//

/// When servers publish themselves to the master server they use this data.
struct NetHostPublishData
{
  /// Unique project identifier.
  Guid      mProjectGuid;
  /// Optional, event bundle data.
  BitStream mBasicHostInfo;
};

/// Serializes network host record list data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostPublishData& netHostRecordList);

//---------------------------------------------------------------------------------//
//                                NetRequestHostRefreshData                        //
//---------------------------------------------------------------------------------//

/// When servers publish themselves to the master server they use this data.
struct NetRequestHostRefreshData
{
  /// Unique project identifier.
  Guid       mProjectGuid;
  /// Host we want new basic host info for.
  IpAddress  mHostIp;
};

/// Serializes network host record list data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetRequestHostRefreshData& netRequestHostRefreshData);

//---------------------------------------------------------------------------------//
//                                NetHostRefreshData                               //
//---------------------------------------------------------------------------------//

/// When servers publish themselves to the master server they use this data.
struct NetHostRefreshData
{
  /// IP address the host info is from.
  IpAddress  mHostIp;
  /// Optional, event bundle data.
  BitStream mBasicHostInfo;
};

/// Serializes network host record list data.
/// Returns the number of bits serialized if successful, else 0.
Bits Serialize(SerializeDirection::Enum direction, BitStream& bitStream, NetHostRefreshData& netRequestHostRefreshData);

//---------------------------------------------------------------------------------//
//                                 FamilyTree                                      //
//---------------------------------------------------------------------------------//

/// Family Tree.
/// Maintains an original non-emplaced network object archetype hierarchy.
class FamilyTree
{
public:
  /// Constructors.
  FamilyTree();
  FamilyTree(FamilyTreeId familyTreeId, NetObject* ancestor);

  /// Destructor.
  ~FamilyTree();

  /// Comparison Operators (compares family tree IDs).
  bool operator ==(const FamilyTree& rhs) const;
  bool operator !=(const FamilyTree& rhs) const;
  bool operator  <(const FamilyTree& rhs) const;
  bool operator ==(const FamilyTreeId& rhs) const;
  bool operator !=(const FamilyTreeId& rhs) const;
  bool operator  <(const FamilyTreeId& rhs) const;

  //
  // Operations
  //

  /// Adds a non-emplaced net object (ancestor or descendant) to the family tree.
  /// These MUST be added in depth-first pre-order traversal order!
  /// Returns true if successful, else false.
  bool AddNetObject(NetObject* netObject);
  /// Removes a non-emplaced net object (ancestor or descendant) from the family tree.
  /// These may be removed in any order!
  /// When removed the net object is actually just marked absent (pointer is cleared to null).
  /// Returns true if successful, else false.
  bool RemoveNetObject(NetObject* netObject);

  /// Returns the family tree ID.
  FamilyTreeId GetFamilyTreeId() const;

  /// Returns the ancestor's cog display name string.
  const String& GetAncestorDisplayName() const;
  /// Returns the ancestor's create context (space net object ID).
  const CreateContext& GetAncestorCreateContext() const;
  /// Returns the ancestor's replica type (archetype resource ID).
  const ReplicaType& GetAncestorReplicaType() const;

  /// Returns all net objects in the family tree.
  /// First the ancestor, then descendants in depth-first pre-order traversal order.
  /// Absent (destroyed/forgotten) net objects are represented with nullptrs.
  const ReplicaArray& GetReplicas() const;

  /// Returns true if the family tree is empty (all net objects are absent), else false.
  bool IsEmpty() const;

private:
  // Data
  FamilyTreeId  mFamilyTreeId;          ///< Family tree ID.
  String        mAncestorDisplayName;   ///< Ancestor's cog display name string.
  CreateContext mAncestorCreateContext; ///< Ancestor's create context (space net object ID).
  ReplicaType   mAncestorReplicaType;   ///< Ancestor's replica type (archetype resource ID).
  ReplicaArray  mReplicas;              ///< All net objects in the family tree.
                                        /// First the ancestor, then descendants in depth-first pre-order traversal order.
                                        /// Absent (destroyed/forgotten) net objects are represented with nullptrs.
};

/// Typedefs.
typedef UniquePointer<FamilyTree>                                   FamilyTreePtr;
typedef ArraySet< FamilyTreePtr, PointerSortPolicy<FamilyTreePtr> > FamilyTreeSet;
typedef ArraySet<FamilyTreeId>                                      FamilyTreeIdSet;

} // namespace Zero
