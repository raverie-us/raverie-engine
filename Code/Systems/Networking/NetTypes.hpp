// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

// Editor Tags
namespace Tags
{
DeclareTag(Networking);
}

/// Constants.
static const String cScriptSource = "ScriptSource";
static const String cEventId = "EventId";
static const String cGameSetup = "GameSetup";

/// Returns true if the specified property instance is a supported net property,
/// else false.
bool IsValidNetProperty(Property* property);
/// Returns true if the specified property type is a supported net property
/// type, else false.
bool IsValidNetPropertyType(Type* propertyType);
bool IsValidNetPropertyType(NativeType* propertyType);
/// Returns true if the specified property type is a supported net peer ID
/// property type, else false.
bool IsValidNetPeerIdPropertyType(Type* propertyType);

//                            NetObject Configuration //

/// Use Archetype "ResourceId:Name" String as ReplicaType?
/// Otherwise we will use ResourceId u64 as ReplicaType (much more efficient).
/// This impacts bandwidth performance when first spawning/cloning new NetObject
/// types to remote peers.
#ifdef RaverieDebug
#  define NETOBJECT_USE_RESOURCE_ID_NAME_STRING
#endif

//                                 Network Types //

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
/// Used when getting / setting values on a specific component's property
/// instance during replication.
struct ComponentPropertyInstanceData
{
  /// Constructor.
  ComponentPropertyInstanceData(String propertyName = String(), Component* component = nullptr);

  // Data
  String mPropertyName;
  Component* mComponent;
};

//
// NetChannel Authority Property Getter / Setter
//

// Serialized Data Type: Any (storing type Authority::Enum)
Variant GetNetChannelAuthorityProperty(const Variant& propertyData);
void SetNetChannelAuthorityProperty(const Variant& value, Variant& propertyData);

//
// NetObject Name Property Getter / Setter
//

// Serialized Data Type: String
Variant GetNetObjectNameProperty(const Variant& propertyData);
void SetNetObjectNameProperty(const Variant& value, Variant& propertyData);

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

/// Returns the BasicNativeType enum equivalent of the BasicNetType enum value,
/// else BasicNativeType::Unknown.
BasicNativeType::Enum BasicNetworkToNativeTypeEnum(BasicNetType::Enum value);
/// Returns the BasicNetType enum equivalent of the BasicNativeType enum value,
/// else BasicNetType::Other.
BasicNetType::Enum BasicNativeToNetworkTypeEnum(BasicNativeType::Enum value);

/// Returns true if the event has a property with a NetPeerId attribute, else
/// false.
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
DeclareEnum4(NetRefreshResult, NoResponse, IndirectBasicHostInfo, DirectBasicHostInfo, ExtraHostInfo);

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
              NetEvent,             /// Network dispatch event.
              NetUserAddRequest,    /// Network user add request.
              NetUserAddResponse,   /// Network user add response.
              NetUserRemoveRequest, /// Network user remove request.
              NetLevelLoadStarted,  /// Network level load started.
              NetLevelLoadFinished, /// Network level load finished.
              NetGameLoadStarted,   /// Network game load started.
              NetGameLoadFinished,  /// Network game load finished.
              NetHostPing,          /// Network host ping.
              NetHostPong,          /// Network host pong.
              NetHostRecordList,    /// Network host record list.
              NetHostPublish);      ///< Network host publish.

//
// NetPeer Protocol Message Types
//

//                             NetUserAddRequestData //

/// Network user add request protocol message data.
struct NetUserAddRequestData
{
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                             NetUserAddResponseData //

/// Network user add response protocol message data.
struct NetUserAddResponseData
{
  /// User add response.
  NetUserAddResponse::Enum mAddResponse;
  /// Optional, network user identifier (set only if accepted).
  NetUserId mNetUserId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                            NetUserRemoveRequestData //

/// Network user remove request protocol message data.
struct NetUserRemoveRequestData
{
  /// Network user identifier.
  NetUserId mNetUserId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                            NetLevelLoadStartedData //

/// Network level load started protocol message data.
struct NetLevelLoadStartedData
{
  /// Network space object identifier.
  NetObjectId mNetSpaceObjectId;
  /// Level resource identifier.
  u64 mLevelResourceId;
};

//                            NetLevelLoadFinishedData //

/// Network level load finished protocol message data.
struct NetLevelLoadFinishedData
{
  /// Network space object identifier.
  NetObjectId mNetSpaceObjectId;
};

//                              NetConnectRequestData //

/// Network connect link request protocol message data.
struct NetConnectRequestData
{
  /// Pending user add request count.
  uint mAddUserRequestCount;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                                 NetHostPingData //

/// Network host ping protocol message data.
struct NetHostPingData
{
  /// Unique project identifier.
  Guid mProjectGuid;
  /// Unique ping request identifier.
  uint mPingId;
  /// Unique send attempt identifier.
  uint mSendAttemptId;
  /// Manager id, groups pings.
  uint mManagerId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                                 NetHostPongData //

/// Network host pong protocol message data.
struct NetHostPongData
{
  /// Unique project identifier.
  Guid mProjectGuid;
  /// Unique ping request identifier.
  uint mPingId;
  /// Unique send attempt identifier.
  uint mSendAttemptId;
  /// Manager id, describes which ping manager this pong belongs to.
  uint mManagerId;
  /// Optional, event bundle data.
  BitStream mEventBundleData;
};

//                                NetHostRecordList //

/// Network host record list message data.
struct NetHostRecordListData
{
  // Network host records
  Array<NetHostRecord> mNetHostRecords;
};

//                                NetHostPublishData //

/// When servers publish themselves to the master server they use this data.
struct NetHostPublishData
{
  /// Unique project identifier.
  Guid mProjectGuid;
  /// Optional, event bundle data.
  BitStream mBasicHostInfo;
};

//                                NetRequestHostRefreshData //

/// When servers publish themselves to the master server they use this data.
struct NetRequestHostRefreshData
{
  /// Unique project identifier.
  Guid mProjectGuid;
  /// Host we want new basic host info for.
  IpAddress mHostIp;
};

//                                NetHostRefreshData //

/// When servers publish themselves to the master server they use this data.
struct NetHostRefreshData
{
  /// IP address the host info is from.
  IpAddress mHostIp;
  /// Optional, event bundle data.
  BitStream mBasicHostInfo;
};

/// Serializes network host record list data.
/// Returns the number of bits serialized if successful, else 0.
template <>
Bits Serialize<NetHostRefreshData>(SerializeDirection::Enum direction,
                                   BitStream& bitStream,
                                   NetHostRefreshData& netRequestHostRefreshData);

//                                 FamilyTree //

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
  bool operator==(const FamilyTree& rhs) const;
  bool operator!=(const FamilyTree& rhs) const;
  bool operator<(const FamilyTree& rhs) const;
  bool operator==(const FamilyTreeId& rhs) const;
  bool operator!=(const FamilyTreeId& rhs) const;
  bool operator<(const FamilyTreeId& rhs) const;

  //
  // Operations
  //

  /// Adds a non-emplaced net object (ancestor or descendant) to the family
  /// tree. These MUST be added in depth-first pre-order traversal order!
  /// Returns true if successful, else false.
  bool AddNetObject(NetObject* netObject);
  /// Removes a non-emplaced net object (ancestor or descendant) from the family
  /// tree. These may be removed in any order! When removed the net object is
  /// actually just marked absent (pointer is cleared to null). Returns true if
  /// successful, else false.
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
  /// First the ancestor, then descendants in depth-first pre-order traversal
  /// order. Absent (destroyed/forgotten) net objects are represented with
  /// nullptrs.
  const ReplicaArray& GetReplicas() const;

  /// Returns true if the family tree is empty (all net objects are absent),
  /// else false.
  bool IsEmpty() const;

private:
  // Data
  FamilyTreeId mFamilyTreeId;           ///< Family tree ID.
  String mAncestorDisplayName;          ///< Ancestor's cog display name string.
  CreateContext mAncestorCreateContext; ///< Ancestor's create context (space
                                        ///< net object ID).
  ReplicaType mAncestorReplicaType;     ///< Ancestor's replica type (archetype
                                        ///< resource ID).
  ReplicaArray mReplicas;               ///< All net objects in the family tree.
                                        /// First the ancestor, then descendants in depth-first
                                        /// pre-order traversal order. Absent (destroyed/forgotten) net
                                        /// objects are represented with nullptrs.
};

/// Typedefs.
typedef UniquePointer<FamilyTree> FamilyTreePtr;
typedef ArraySet<FamilyTreePtr, PointerSortPolicy<FamilyTreePtr>> FamilyTreeSet;
typedef ArraySet<FamilyTreeId> FamilyTreeIdSet;

//
// NetPeer Protocol Message Types
//

//                             NetUserAddRequestData //
template <>
inline Bits Serialize<NetUserAddRequestData>(SerializeDirection::Enum direction,
                                             BitStream& bitStream,
                                             NetUserAddRequestData& netUserAddRequestData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write dummy bit
    // TODO: Refactor BitStream interface to allow for serialization of empty
    // structures without appearing to be an error
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
    // TODO: Refactor BitStream interface to allow for serialization of empty
    // structures without appearing to be an error
    bool dummy;
    ReturnIf(!bitStream.Read(dummy), 0, "");

    // Read event bundle data (if any)
    netUserAddRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                             NetUserAddResponseData //
template <>
inline Bits Serialize<NetUserAddResponseData>(SerializeDirection::Enum direction,
                                              BitStream& bitStream,
                                              NetUserAddResponseData& netUserAddResponseData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
  {
    const Bits bitsWrittenStart = bitStream.GetBitsWritten();

    // Write network user add response
    bitStream.WriteQuantized(netUserAddResponseData.mAddResponse, NetUserAddResponseMin, NetUserAddResponseMax);

    // Accepted?
    if (netUserAddResponseData.mAddResponse == NetUserAddResponse::Accept)
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
    ReturnIf(
        !bitStream.ReadQuantized(netUserAddResponseData.mAddResponse, NetUserAddResponseMin, NetUserAddResponseMax),
        0,
        "");

    // Accepted?
    if (netUserAddResponseData.mAddResponse == NetUserAddResponse::Accept)
    {
      // Read network user identifier
      ReturnIf(!bitStream.Read(netUserAddResponseData.mNetUserId), 0, "");
    }

    // Read event bundle data (if any)
    netUserAddResponseData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                            NetUserRemoveRequestData //
template <>
inline Bits Serialize<NetUserRemoveRequestData>(SerializeDirection::Enum direction,
                                                BitStream& bitStream,
                                                NetUserRemoveRequestData& netUserRemoveRequestData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(netUserRemoveRequestData.mNetUserId), 0, "");

    // Read event bundle data (if any)
    netUserRemoveRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                            NetLevelLoadStartedData //
template <>
inline Bits Serialize<NetLevelLoadStartedData>(SerializeDirection::Enum direction,
                                               BitStream& bitStream,
                                               NetLevelLoadStartedData& netLevelLoadStartedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    if (!bitStream.Read(netSpaceObjectId)) // Unable?
      return 0;

    // Set network space object identifier
    netLevelLoadStartedData.mNetSpaceObjectId = netSpaceObjectId.value();

    // Read level resource identifier
    ReturnIf(!bitStream.Read(netLevelLoadStartedData.mLevelResourceId), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                            NetLevelLoadFinishedData //
template <>
inline Bits Serialize<NetLevelLoadFinishedData>(SerializeDirection::Enum direction,
                                                BitStream& bitStream,
                                                NetLevelLoadFinishedData& netLevelLoadFinishedData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    if (!bitStream.Read(netSpaceObjectId)) // Unable?
      return 0;

    // Set network space object identifier
    netLevelLoadFinishedData.mNetSpaceObjectId = netSpaceObjectId.value();

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              NetConnectRequestData //
template <>
inline Bits Serialize<NetConnectRequestData>(SerializeDirection::Enum direction,
                                             BitStream& bitStream,
                                             NetConnectRequestData& netConnectRequestData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read(netConnectRequestData.mAddUserRequestCount), 0, "");

    // Read event bundle data (if any)
    netConnectRequestData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                                 NetHostPingData //
template <>
inline Bits Serialize<NetHostPingData>(SerializeDirection::Enum direction,
                                       BitStream& bitStream,
                                       NetHostPingData& netHostPingData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read((u64&)netHostPingData.mProjectGuid), 0, "");

    // Read unique ping request identifier
    ReturnIf(!bitStream.Read(netHostPingData.mPingId), 0, "");

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPingData.mSendAttemptId), 0, "");

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPingData.mManagerId), 0, "");

    // Read event bundle data (if any)
    netHostPingData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                                 NetHostPongData //
template <>
inline Bits Serialize<NetHostPongData>(SerializeDirection::Enum direction,
                                       BitStream& bitStream,
                                       NetHostPongData& netHostPongData)
{
  // Write operation?
  if (direction == SerializeDirection::Write)
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
    ReturnIf(!bitStream.Read((u64&)netHostPongData.mProjectGuid), 0, "");

    // Read unique ping request identifier
    ReturnIf(!bitStream.Read(netHostPongData.mPingId), 0, "");

    // Read unique send attempt identifier
    ReturnIf(!bitStream.Read(netHostPongData.mSendAttemptId), 0, "");

    // Read unique manager id
    ReturnIf(!bitStream.Read(netHostPongData.mManagerId), 0, "");

    // Read event bundle data (if any)
    netHostPongData.mEventBundleData.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                                NetHostRecordList //
template <>
inline Bits Serialize<NetHostRecordListData>(SerializeDirection::Enum direction,
                                             BitStream& bitStream,
                                             NetHostRecordListData& netHostRecordList)
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

    // write how many elements there are.
    bitStream.Write(netHostRecordList.mNetHostRecords.Size());

    forRange (NetHostRecord& record, netHostRecordList.mNetHostRecords.All())
    {
      // Serialize the IP address
      bitStream.Write(record.mIpAddress);
      // Serialize the bitstream
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
    // TODO: Potentially need to also write how many elements were in this
    // array. Its possible this isn't the only thing in a message. If there is
    // anything written after this in a message, it would clobber it.
    for (size_t i = 0; i < elements; i += 1)
    {
      NetHostRecord record;
      // Read in IP address of record.
      ReturnIf(!bitStream.Read(record.mIpAddress), 0, "");
      // Read in the size of the BitStream written.
      Bits bitsToRead = 0;
      ReturnIf(!bitStream.Read(bitsToRead), 0, "");
      // Create a temp bitstream, and have Reserve enough space for it to read
      // the bitstream in.
      BitStream tempBitStream;
      tempBitStream.Reserve(bitsToRead / 8); // Reserve enough space to write the bits into the stream
                                             // directly.
                                             // Read in the bitstream containing the BasicHostInfo eventbundle.
      ReturnIf(!bitStream.ReadBits(tempBitStream.GetDataExposed(), bitsToRead), 0, "");
      // Set the number of bits we wrote into it (because we manually assigned
      // bit data into it)
      tempBitStream.SetBitsWritten(bitsToRead);
      // move the bitstream
      record.mBasicHostInfo = RaverieMove(tempBitStream);
      // Move the record into the array.
      netHostRecordList.mNetHostRecords.PushBack(record);
    }

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              NetHostPublishData //
template <>
inline Bits Serialize<NetHostPublishData>(SerializeDirection::Enum direction,
                                          BitStream& bitStream,
                                          NetHostPublishData& netHostPingData)
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
    ReturnIf(!bitStream.Read((u64&)netHostPingData.mProjectGuid), 0, "");

    // Read event bundle data (if any)
    netHostPingData.mBasicHostInfo.AssignRemainder(bitStream);

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              NetRequestHostRefreshData //
template <>
inline Bits Serialize<NetRequestHostRefreshData>(SerializeDirection::Enum direction,
                                                 BitStream& bitStream,
                                                 NetRequestHostRefreshData& netRequestHostRefreshData)
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
    ReturnIf(!bitStream.Read((u64&)netRequestHostRefreshData.mProjectGuid), 0, "");

    // Read event bundle data (if any)
    ReturnIf(!bitStream.Read(netRequestHostRefreshData.mHostIp), 0, "");

    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

//                              NetHostRefreshData //
template <>
inline Bits Serialize<NetHostRefreshData>(SerializeDirection::Enum direction,
                                          BitStream& bitStream,
                                          NetHostRefreshData& netHostRefreshData)
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
    ReturnIf(!bitStream.Read(netHostRefreshData.mHostIp), 0, "");
    // Read event bundle data (if any)
    netHostRefreshData.mBasicHostInfo.AssignRemainder(bitStream);
    // Success
    return bitStream.GetBitsRead() - bitsReadStart;
  }
};

} // namespace Raverie
