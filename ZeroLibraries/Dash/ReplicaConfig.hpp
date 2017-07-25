///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

//---------------------------------------------------------------------------------//
//                           Replica Configuration                                 //
//---------------------------------------------------------------------------------//

/// Replicator ID bits
/// Determines the maximum number of replicators (peers) in the network graph
#define REPLICATOR_ID_BITS 8
StaticAssertWithinRange(Range10, REPLICATOR_ID_BITS, 1, UINTMAX_BITS);

/// Replica ID bits
/// Determines the maximum number of replicas (objects) in the application
#define REPLICA_ID_BITS 16
StaticAssertWithinRange(Range11, REPLICA_ID_BITS, 1, UINTMAX_BITS);

/// Emplace ID bits
/// Determines the maximum number of replicas (objects) in an emplace context
#define EMPLACE_ID_BITS 16
StaticAssertWithinRange(Range12, EMPLACE_ID_BITS, 1, REPLICA_ID_BITS);

/// Create Context ID bits
/// Determines the maximum number of create contexts
#define CREATE_CONTEXT_ID_BITS 10
StaticAssertWithinRange(Range13, CREATE_CONTEXT_ID_BITS, 1, UINTMAX_BITS);

/// Replica Type ID bits
/// Determines the maximum number of replica types
#define REPLICA_TYPE_ID_BITS 12
StaticAssertWithinRange(Range14, REPLICA_TYPE_ID_BITS, 1, UINTMAX_BITS);

/// Emplace Context ID bits
/// Determines the maximum number of emplace contexts
#define EMPLACE_CONTEXT_ID_BITS 11
StaticAssertWithinRange(Range15, EMPLACE_CONTEXT_ID_BITS, 1, UINTMAX_BITS);

/// Replica should use a virtual destructor?
/// Enable this if you're relying on replica polymorphism for deletion
#define REPLICA_USE_VIRTUAL_DESTRUCTOR 0

#if REPLICA_USE_VIRTUAL_DESTRUCTOR
  #define REPLICA_VIRTUAL virtual
#else
  #define REPLICA_VIRTUAL
#endif

/// ReplicaChannel should use a virtual destructor?
/// Enable this if you're relying on replica channel polymorphism for deletion
#define REPLICA_CHANNEL_USE_VIRTUAL_DESTRUCTOR 1

#if REPLICA_CHANNEL_USE_VIRTUAL_DESTRUCTOR
  #define REPLICA_CHANNEL_VIRTUAL virtual
#else
  #define REPLICA_CHANNEL_VIRTUAL
#endif

/// ReplicaChannelType should use a virtual destructor?
/// Enable this if you're relying on replica channel type polymorphism for deletion
#define REPLICA_CHANNEL_TYPE_USE_VIRTUAL_DESTRUCTOR 1

#if REPLICA_CHANNEL_TYPE_USE_VIRTUAL_DESTRUCTOR
  #define REPLICA_CHANNEL_TYPE_VIRTUAL virtual
#else
  #define REPLICA_CHANNEL_TYPE_VIRTUAL
#endif

/// ReplicaProperty should use a virtual destructor?
/// Enable this if you're relying on replica property polymorphism for deletion
#define REPLICA_PROPERTY_USE_VIRTUAL_DESTRUCTOR 1

#if REPLICA_PROPERTY_USE_VIRTUAL_DESTRUCTOR
  #define REPLICA_PROPERTY_VIRTUAL virtual
#else
  #define REPLICA_PROPERTY_VIRTUAL
#endif

/// ReplicaPropertyType should use a virtual destructor?
/// Enable this if you're relying on replica property type polymorphism for deletion
#define REPLICA_PROPERTY_TYPE_USE_VIRTUAL_DESTRUCTOR 1

#if REPLICA_PROPERTY_TYPE_USE_VIRTUAL_DESTRUCTOR
  #define REPLICA_PROPERTY_TYPE_VIRTUAL virtual
#else
  #define REPLICA_PROPERTY_TYPE_VIRTUAL
#endif

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               Replicator ID                                     //
//---------------------------------------------------------------------------------//

/// Replicator ID
/// Identifies a replicator (peer) in the network graph
static const Bits ReplicatorIdBits = REPLICATOR_ID_BITS;
typedef UintN<ReplicatorIdBits> ReplicatorId;

//---------------------------------------------------------------------------------//
//                                Replica ID                                       //
//---------------------------------------------------------------------------------//

/// Replica ID
/// Identifies a replica (object) in the application
static const Bits ReplicaIdBits = REPLICA_ID_BITS;
typedef UintN<ReplicaIdBits> ReplicaId;

//---------------------------------------------------------------------------------//
//                                Emplace ID                                       //
//---------------------------------------------------------------------------------//

/// Emplace ID
/// Identifies a replica (object) in an emplace context
static const Bits EmplaceIdBits = EMPLACE_ID_BITS;
typedef UintN<EmplaceIdBits> EmplaceId;

//---------------------------------------------------------------------------------//
//                               Create Context                                    //
//---------------------------------------------------------------------------------//

/// Create Context
/// Contains an application-specific replica (object) create context
/// Used as a unique identifier to represent a created set of replicas associated with an arbitrary context
/// For example, this might represent the conceptual "space" an object is created in
/// This is given to the user-defined CreateReplica function and interpreted by the user
typedef Variant CreateContext;

//---------------------------------------------------------------------------------//
//                               Replica Type                                      //
//---------------------------------------------------------------------------------//

/// Replica Type
/// Contains an application-specific replica (object) type identifier
/// Used as a unique identifier to represent an arbitrary archetype
/// For example, this usually represents the application-specific type of object (ex. "player", "ammo", "enemy", etc.)
/// This is given to the user-defined CreateReplica function and interpreted by the user
typedef Variant ReplicaType;

//---------------------------------------------------------------------------------//
//                              Emplace Context                                    //
//---------------------------------------------------------------------------------//

/// Emplace Context
/// Contains an application-specific replica (object) emplace context
/// Used as a unique identifier to represent an emplaced set of replicas associated with an arbitrary context
/// For example, this would typically be a level name and all network objects loaded in would be emplaced on it by all peers
/// This is used internally, it just needs to be unique to represent a different set of emplaced replicas
typedef Variant EmplaceContext;

//---------------------------------------------------------------------------------//
//                              Create Context ID                                  //
//---------------------------------------------------------------------------------//

/// Create Context ID
/// Identifies a create context value (serialized in place of the create context to conserve bandwidth)
static const Bits CreateContextIdBits = CREATE_CONTEXT_ID_BITS;
typedef UintN<CreateContextIdBits> CreateContextId;

//---------------------------------------------------------------------------------//
//                              Replica Type ID                                    //
//---------------------------------------------------------------------------------//

/// Replica Type ID
/// Identifies a replica type value (serialized in place of the replica type to conserve bandwidth)
static const Bits ReplicaTypeIdBits = REPLICA_TYPE_ID_BITS;
typedef UintN<ReplicaTypeIdBits> ReplicaTypeId;

//---------------------------------------------------------------------------------//
//                             Emplace Context ID                                  //
//---------------------------------------------------------------------------------//

/// Emplace Context ID
/// Identifies an emplace context value (serialized in place of the emplace context to conserve bandwidth)
static const Bits EmplaceContextIdBits = EMPLACE_CONTEXT_ID_BITS;
typedef UintN<EmplaceContextIdBits> EmplaceContextId;

//---------------------------------------------------------------------------------//
//                             Property Functions                                  //
//---------------------------------------------------------------------------------//

/// Property Serializer
/// Serializes the current property value
/// Returns the number of bits serialized if successful, else 0
typedef Bits (*SerializeValueFn)(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value);

/// Property Getter
/// Returns the current property value
typedef Variant (*GetValueFn)(const Variant& propertyData);

/// Property Setter
/// Sets the current property value
typedef void (*SetValueFn)(const Variant& value, Variant& propertyData);

//---------------------------------------------------------------------------------//
//                                 Typedefs                                        //
//---------------------------------------------------------------------------------//

/// Typedefs
typedef Array<Replica*>                                   ReplicaArray;
typedef ArraySet< Replica*, PointerSortPolicy<Replica*> > ReplicaSet;
typedef ArrayMap<CreateContext, ReplicaSet>               CreateMap;
typedef ArrayMap<ReplicaType, ReplicaSet>                 ReplicaMap;
typedef ArrayMap<EmplaceContext, ReplicaSet>              EmplaceMap;
typedef ArrayMap<EmplaceContext, IdStore<EmplaceId> >     EmplaceIdStores;
typedef ArraySet<ReplicatorId>                            ReplicatorIdSet;
typedef ArrayMap<ReplicaChannel*, MessageChannelId>       OutReplicaChannels;
typedef ArrayMap<MessageChannelId, ReplicaChannel*>       InReplicaChannels;
typedef ArrayMap<ReplicaChannel*, MessageChannelId>       InReplicaChannelsFlipped;
typedef Pair<Message, TransmissionDirection::Enum>        MessageDirectionPair;

//---------------------------------------------------------------------------------//
//                                  Enums                                          //
//---------------------------------------------------------------------------------//

/// Network Role
// (NOTE: Corresponding enum values MUST match up with Authority!)
DeclareEnum5(Role,
  Client,        /// Act as an online client, able to connect to a single server
  Server,        /// Act as an online server, able to accept connections from multiple clients
  Unspecified,   /// Unspecified network role
  Offline,       /// Act as an offline peer (provided as an API simulator to enable networked games to use the same code in offline contexts)
  MasterServer); /// Act as an online master server, able to provide host lists and facilitate connections

/// ReplicaChannel Change Authority
// (NOTE: Corresponding enum values MUST match up with Role!)
DeclareEnum2(Authority,
  Client,  /// Client has authority over property changes
  Server); /// Server has authority over property changes

/// ReplicaChannel Change Authority Mode
DeclareEnum2(AuthorityMode,
  Dynamic, /// Authority is dynamic and may be modified after a replica is made valid
  Fixed);  /// Authority is fixed and cannot be modified after a replica is made valid

/// Replicator Plugin Message Types
DeclareEnum11(ReplicatorMessageType,
  ConnectConfirmation,     /// Connect confirmation
  CreateContextItems,      /// Creation context cache items
  ReplicaTypeItems,        /// Replica type cache items
  EmplaceContextItems,     /// Emplace context cache items
  Spawn,                   /// Spawn command
  Clone,                   /// Clone command
  Forget,                  /// Forget command
  Destroy,                 /// Destroy command
  Change,                  /// Replica channel change
  Interrupt,               /// Interrupt step command
  ReverseReplicaChannels); /// Reverse replica channel mappings

// Replica Stream Serialization Mode
DeclareEnum5(ReplicaStreamMode,
  Spawn,                   /// Spawn serialization mode
  Clone,                   /// Clone serialization mode
  Forget,                  /// Forget serialization mode
  Destroy,                 /// Destroy serialization mode
  ReverseReplicaChannels); /// Reverse replica channels serialization mode

/// ReplicaChannel Serialization Flags
namespace SerializationFlags
{
  typedef uint Type;
  enum Enum
  {
    None,

    OnSpawn        = (1 << 0), /// Serialize on replica spawn
    OnCloneEmplace = (1 << 1), /// Serialize on replica clone (if the replica was originally emplaced)
    OnCloneSpawn   = (1 << 2), /// Serialize on replica clone (if the replica was originally spawned)
    OnForget       = (1 << 3), /// Serialize on replica forget
    OnDestroy      = (1 << 4), /// Serialize on replica destroy
    OnChange       = (1 << 5), /// Serialize on replica channel change

    Default = OnSpawn | OnCloneEmplace | OnCloneSpawn | OnChange,
    All     = Default | OnForget       | OnDestroy,
  };
}

/// ReplicaChannel Change Detection Mode
DeclareEnum4(DetectionMode,
  Assume,     /// Assume something has changed
  Manual,     /// Detect changes manually using change flags
  Automatic,  /// Detect changes automatically using comparisons
  Manumatic); /// Detect changes manually using change flags and automatically using comparisons

/// ReplicaChannel Change Reliability Mode
DeclareEnum2(ReliabilityMode,
  Unreliable, /// Lost changes are not retransmitted
  Reliable);  /// Lost changes are retransmitted

/// ReplicaChannel/Property Change Serialization Mode
DeclareEnum2(SerializationMode,
  All,      /// Serialize all properties/primitive-components (Always used internally when there is only one)
  Changed); /// Serialize only properties/primitive-components that have changed (Uses an extra bit flag between them)

/// Routing Mode
DeclareEnum2(RouteMode,
  Exclude,  /// Route to all except targets
  Include); /// Route to none except targets

/// Replication Phase
DeclareEnum3(ReplicationPhase,
  Initialization,    /// Replica/Channel/Property initialization
  Uninitialization,  /// Replica/Channel/Property uninitialization
  Change);           /// Replica/Channel/Property change

/// Replica Property Convergence State
DeclareEnum3(ConvergenceState,
  None,     /// No convergence is being applied
  Active,   /// Active convergence is being applied
  Resting); /// Resting convergence is being applied

//---------------------------------------------------------------------------------//
//                                 Macros                                          //
//---------------------------------------------------------------------------------//

#if ZERO_ENABLE_ERROR

/// Test the specified condition on all replicas provided
#define AssertReplicas(replicas, condition, ...) \
do                                               \
{                                                \
  /* For all replicas */                         \
  forRange(Replica* replica, replicas.All())     \
  {                                              \
    /* Absent replica? */                        \
    if(!replica)                                 \
      continue; /* Skip */                       \
                                                 \
    /* Test provided condition */                \
    Assert(condition, __VA_ARGS__);              \
  }                                              \
} while(gConditionalFalseConstant)

#else

#define AssertReplicas(...) ((void)0)

#endif

/// Switch cases for non-boolean arithmetic types
/// Calls the specified function with the given parameters
/// Optionally returns or breaks afterwards
#define SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(ReturnOrEmpty, BreakOrNoOp, Fn, ...)              \
                                                                                              \
/* Char Type */                                                                               \
case BasicNativeType::Char:                                                                   \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Char>::Type>(__VA_ARGS__);        \
  BreakOrNoOp;                                                                                \
                                                                                              \
/* Fixed-Width Signed Integral Types */                                                       \
case BasicNativeType::Int8:                                                                   \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Int8>::Type>(__VA_ARGS__);        \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Int16:                                                                  \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Int16>::Type>(__VA_ARGS__);       \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Int32:                                                                  \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type>(__VA_ARGS__);       \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Int64:                                                                  \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type>(__VA_ARGS__);       \
  BreakOrNoOp;                                                                                \
                                                                                              \
/* Fixed-Width Unsigned Integral Types */                                                     \
case BasicNativeType::Uint8:                                                                  \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type>(__VA_ARGS__);       \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Uint16:                                                                 \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Uint16>::Type>(__VA_ARGS__);      \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Uint32:                                                                 \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Uint32>::Type>(__VA_ARGS__);      \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Uint64:                                                                 \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Uint64>::Type>(__VA_ARGS__);      \
  BreakOrNoOp;                                                                                \
                                                                                              \
/* Floating Point Types */                                                                    \
case BasicNativeType::Float:                                                                  \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Float>::Type>(__VA_ARGS__);       \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Double:                                                                 \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Double>::Type>(__VA_ARGS__);      \
  BreakOrNoOp;                                                                                \
                                                                                              \
/* Multi-Primitive Math Types (Excluding Bool Types) */                                       \
case BasicNativeType::IntVector2:                                                             \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type>(__VA_ARGS__);  \
  BreakOrNoOp;                                                                                \
case BasicNativeType::IntVector3:                                                             \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type>(__VA_ARGS__);  \
  BreakOrNoOp;                                                                                \
case BasicNativeType::IntVector4:                                                             \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type>(__VA_ARGS__);  \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Vector2:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type>(__VA_ARGS__);     \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Vector3:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type>(__VA_ARGS__);     \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Vector4:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type>(__VA_ARGS__);     \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Quaternion:                                                             \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type>(__VA_ARGS__);  \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Matrix2:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Matrix2>::Type>(__VA_ARGS__);     \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Matrix3:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type>(__VA_ARGS__);     \
  BreakOrNoOp;                                                                                \
case BasicNativeType::Matrix4:                                                                \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type>(__VA_ARGS__);     \
  BreakOrNoOp

#define SWITCH_CASES_NON_BOOL_ARITHMETIC_CALL(Fn, ...)                                SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(         , ((void)0), Fn, __VA_ARGS__)
#define SWITCH_CASES_NON_BOOL_ARITHMETIC_CALL_AND_RETURN(Fn, ...)                     SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(   return, ((void)0), Fn, __VA_ARGS__)
#define SWITCH_CASES_NON_BOOL_ARITHMETIC_CALL_AND_BREAK(Fn, ...)                      SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(         ,     break, Fn, __VA_ARGS__)
#define SWITCH_CASES_NON_BOOL_ARITHMETIC_CALL_STORE_RESULT_AND_BREAK(Result, Fn, ...) SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(Result = ,     break, Fn, __VA_ARGS__)

/// Switch cases for arithmetic types
/// Calls the specified function with the given parameters
/// Optionally returns or breaks afterwards
#define SWITCH_CASES_ARITHMETIC_DO(ReturnOrEmpty, BreakOrNoOp, Fn, ...)                       \
SWITCH_CASES_NON_BOOL_ARITHMETIC_DO(ReturnOrEmpty, BreakOrNoOp, Fn, __VA_ARGS__);             \
                                                                                              \
/* Bool Type */                                                                               \
case BasicNativeType::Bool:                                                                   \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type>(__VA_ARGS__);        \
  BreakOrNoOp;                                                                                \
                                                                                              \
/* Multi-Primitive Math Types (Only Bool Types) */                                            \
case BasicNativeType::BoolVector2:                                                            \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type>(__VA_ARGS__); \
  BreakOrNoOp;                                                                                \
case BasicNativeType::BoolVector3:                                                            \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type>(__VA_ARGS__); \
  BreakOrNoOp;                                                                                \
case BasicNativeType::BoolVector4:                                                            \
  ReturnOrEmpty Fn<BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type>(__VA_ARGS__); \
  BreakOrNoOp

#define SWITCH_CASES_ARITHMETIC_CALL(Fn, ...)                                SWITCH_CASES_ARITHMETIC_DO(         , ((void)0), Fn, __VA_ARGS__)
#define SWITCH_CASES_ARITHMETIC_CALL_AND_RETURN(Fn, ...)                     SWITCH_CASES_ARITHMETIC_DO(   return, ((void)0), Fn, __VA_ARGS__)
#define SWITCH_CASES_ARITHMETIC_CALL_AND_BREAK(Fn, ...)                      SWITCH_CASES_ARITHMETIC_DO(         ,     break, Fn, __VA_ARGS__)
#define SWITCH_CASES_ARITHMETIC_CALL_STORE_RESULT_AND_BREAK(Result, Fn, ...) SWITCH_CASES_ARITHMETIC_DO(Result = ,     break, Fn, __VA_ARGS__)

//---------------------------------------------------------------------------------//
//                            Helper Functions                                     //
//---------------------------------------------------------------------------------//

// Specialization policy for how Variant key pairs are sorted
template<typename DataType>
struct PairSortPolicy<Variant, DataType>
{
  /// Typedefs
  typedef typename Pair<Variant, DataType> value_type;
  typedef Variant  key_type;
  typedef DataType data_type;

  /// Sorts according to key (Variant type then hash)
  bool operator()(const value_type& lhs, const key_type& rhs) const
  {
    return lhs.first.mNativeType == rhs.mNativeType
        && lhs.first.Hash() < rhs.Hash();
  }
  bool operator()(const value_type& lhs, const value_type& rhs) const
  {
    return lhs.first.mNativeType == rhs.first.mNativeType
        && lhs.first.Hash() < rhs.first.Hash();
  }

  /// Equates according to key (Variant value)
  bool Equal(const value_type& lhs, const key_type& rhs) const
  {
    return lhs.first == rhs;
  }
  bool Equal(const value_type& lhs, const value_type& rhs) const
  {
    return lhs.first == rhs.first;
  }
};

/// (Only defined for arithmetic types)
template <typename T, TF_ENABLE_IF(IsBasicNativeTypeArithmetic<T>::Value)>
inline Bits SerializeKnownBasicVariantArithmetic(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  // Primitive member info
  typedef typename BasicNativeTypePrimitiveMembers<T>::Type PrimitiveType;
  static const size_t PrimitiveCount = BasicNativeTypePrimitiveMembers<T>::Count;

  // Get starting bits count
  Bits startBits = bitStream.GetBitsSerialized(direction);

  // For each primitive member
  for(size_t i = 0; i < PrimitiveCount; ++i)
  {
    // Get the primitive member to be serialized
    PrimitiveType& valuePrimitiveMember = value.GetPrimitiveMemberOrError<T>(i);

    // Serialize primitive member
    if(!bitStream.Serialize(direction, valuePrimitiveMember)) // Unable?
      return 0;
  }

  // Success
  return bitStream.GetBitsSerialized(direction) - startBits;
}

/// Serializes a non-empty basic Variant (stored value type has a constant native type ID)
/// Will not serialize the stored type ID, the deserializer must default construct the variant as the expected type before deserializing
/// Returns the number of bits serialized if successful, else 0
inline Bits SerializeKnownBasicVariant(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  Bits result = 0;

  // Switch on native type
  switch(value.GetNativeTypeId())
  {
  // Other Types
  default:
    {
      Error("Not a non-empty basic variant");
      return 0;
    }

  // Arithmetic Types
  SWITCH_CASES_ARITHMETIC_CALL_STORE_RESULT_AND_BREAK(result, SerializeKnownBasicVariantArithmetic, direction, bitStream, value);

  // String Type
  case BasicNativeType::String:
    {
      // Serialize string
      result = bitStream.Serialize(direction, value.GetOrError<String>());
    }
    break;
  }

  // Serialization failed?
  if(!result)
  {
    Error("Serialization failed");
    return 0;
  }

  return result;
}

/// Serializes a basic Variant (stored value type has a constant native type ID)
/// Will serialize the stored type ID, the deserializer does not need to know what type to expect and should pass in an empty variant when deserializing
/// Returns the number of bits serialized if successful, else 0
inline Bits SerializeUnknownBasicVariant(SerializeDirection::Enum direction, BitStream& bitStream, Variant& value)
{
  // Get starting bits count
  Bits startBits = bitStream.GetBitsSerialized(direction);

  // Writing?
  if(direction == SerializeDirection::Write)
  {
    // Get variant's native type ID
    NativeTypeId nativeTypeId = value.GetNativeTypeId();

    // Variant has a runtime native type ID? (Variant is storing a non-basic native type?)
    if(IsRuntimeNativeTypeId(nativeTypeId))
    {
      Error("Not a basic variant");
      return 0;
    }

    // Write variant's constant native type ID
    if(!bitStream.SerializeQuantized(SerializeDirection::Write, nativeTypeId, cConstantNativeTypeIdMin, cConstantNativeTypeIdMax)) // Unable?
    {
      Error("Serialization failed");
      return 0;
    }
  }
  // Reading?
  else
  {
    // Read variant's constant native type ID
    NativeTypeId nativeTypeId = 0;
    if(!bitStream.SerializeQuantized(SerializeDirection::Read, nativeTypeId, cConstantNativeTypeIdMin, cConstantNativeTypeIdMax)) // Unable?
    {
      Error("Serialization failed");
      return 0;
    }

    // Get native type identified by constant native type ID (if any)
    if(NativeType* nativeType = GetNativeTypeByConstantId(nativeTypeId))
    {
      // Default construct variant as the specified type
      value.DefaultConstruct(nativeType);
    }
  }

  // Variant is not empty?
  if(value.IsNotEmpty())
  {
    // Serialize known variant
    if(!SerializeKnownBasicVariant(direction, bitStream, value)) // Unable?
    {
      Assert(false);
      return 0;
    }
  }

  // Success
  return bitStream.GetBitsSerialized(direction) - startBits;
}

/// Data property getter (propertyData is just a pointer to the property)
/// Returns the current property value
template <typename T>
Variant GetDataValue(const Variant& propertyData)
{
  // Get property data pointer
  T* dataPointer = propertyData.GetOrError<T*>();

  // Get property value
  return Variant(*dataPointer);
}

/// Data property setter (propertyData is just a pointer to the property)
/// Sets the current property value
template <typename T>
void SetDataValue(const Variant& value, Variant& propertyData)
{
  // Get property data pointer
  T* dataPointer = propertyData.GetOrError<T*>();

  // Set property value
  *dataPointer = value.GetOrError<T>();
}

} // namespace Zero
