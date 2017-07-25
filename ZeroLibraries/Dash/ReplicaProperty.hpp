///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                              ReplicaProperty                                    //
//---------------------------------------------------------------------------------//

/// Replica Property
/// Manages the replication of a single property
class ReplicaProperty
{
public:
  /// Constructor
  ReplicaProperty(const String& name, ReplicaPropertyType* replicaPropertyType, const Variant& propertyData);

  /// Destructor
  REPLICA_PROPERTY_VIRTUAL ~ReplicaProperty();

  /// Comparison Operators (compares names)
  bool operator ==(const ReplicaProperty& rhs) const;
  bool operator !=(const ReplicaProperty& rhs) const;
  bool operator  <(const ReplicaProperty& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  //
  // Operations
  //

  /// Replica property name
  const String& GetName() const;

  /// Operating replica property type
  ReplicaPropertyType* GetReplicaPropertyType() const;

  /// Returns true if the replica property is valid (added to a valid replica channel), else false
  bool IsValid() const;

  /// Operating replicator (set if the replica property is valid)
  Replicator* GetReplicator() const;

  /// Operating replica (set if the replica property is valid)
  Replica* GetReplica() const;

  /// Operating replica channel
  void SetReplicaChannel(ReplicaChannel* replicaChannel);
  ReplicaChannel* GetReplicaChannel() const;

  /// Controls what convergence method, if any, to apply to this replica property every convergence interval
  void SetConvergenceState(ConvergenceState::Enum convergenceState);
  ConvergenceState::Enum GetConvergenceState() const;

  /// Returns true if this replica property is scheduled for change convergence, else false
  bool IsScheduled() const;

  /// Returns true if the current property value has changed (according to the configured delta threshold) since the last observation, else false
  bool HasChanged() const;
  /// Returns true if the current property value has changed at all since the last observation, else false
  bool HasChangedAtAll() const;

  /// Sets the current property value
  void SetValue(const Variant& value);
  /// Returns the current property value
  Variant GetValue() const;

  /// Internal property data passed to our getter and setter functions
  const Variant& GetPropertyData() const;

  /// Last observed property value
  void SetLastValue(MoveReference<Variant> value);
  const Variant& GetLastValue() const;

  /// Updates the last observed property value according to what has changed since the last observation
  void UpdateLastValue(bool forceAll);

  /// Timestamp indicating when this replica property was last changed, else cInvalidMessageTimestamp
  /// (Set immediately after a change is observed on any primitive member)
  void SetLastChangeTimestamp(TimeMs lastChangeTimestamp);
  TimeMs GetLastChangeTimestamp() const;

  /// Last received change value
  void SetLastReceivedChangeValue(const Variant& value);
  const Variant& GetLastReceivedChangeValue() const;

  /// Timestamp indicating when this replica property's last received change occurred, else cInvalidMessageTimestamp
  void SetLastReceivedChangeTimestamp(TimeMs lastReceivedChangeTimestamp);
  TimeMs GetLastReceivedChangeTimestamp() const;

  /// Frame ID when the last change was received on this replica property
  void SetLastReceivedChangeFrameId(uint64 lastReceivedChangeFrameId);
  uint64 GetLastReceivedChangeFrameId() const;

  /// Updates the received value curve at the specified time with the specified value
  void UpdateCurve(TimeMs timestamp, const Variant& value);

  /// Samples the received value curve at the specified time and returns the resulting value, else Variant()
  Variant SampleCurve(TimeMs timestamp);

  /// Returns the current sampled value from the received value curve, else Variant()
  Variant GetCurrentSampledValue();

  /// Returns the current sample time value used when sampling from the received value curve
  TimeMs GetCurrentSampleTime();

  /// Returns true if the property is resting (not actively changing)
  /// Considered at rest if the extrapolation limit duration has elapsed since the last value was received
  bool IsResting();

  /// Computes the interpolant [0, 1] to be applied when performing resting convergence
  float ComputeRestingInterpolant();

  /// Immediately sets the current property value to the appropriate received value
  void SnapNow();

  /// Gradually sets the current property value to the appropriate received value
  void ConvergeActiveNow();
  void ConvergeRestingNow();

  /// Reacts if the replica property has been legitimately changed, determined using comparisons, since this function was last called
  void ReactToChanges(TimeMs timestamp, ReplicationPhase::Enum replicationPhase, TransmissionDirection::Enum direction, bool generateNotification = true, bool setLastValue = true);

  //
  // Internal
  //

  /// Serializes the replica property
  /// Returns true if successful, else false
  bool Serialize(BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp) const;
  /// Deserializes the replica property
  /// Returns true if successful, else false
  bool Deserialize(const BitStream& bitStream, ReplicationPhase::Enum replicationPhase, TimeMs timestamp);

  /// Data
  String                 mName;                        /// Replica property name
  ReplicaPropertyType*   mReplicaPropertyType;         /// Operating replica property type
  ReplicaChannel*        mReplicaChannel;              /// Operating replica channel
  Link<ReplicaProperty>  mIndexListLink;               /// Replica property index list link (may be null)
  size_t*                mIndexListSize;               /// Replica property index list size (may be null)
  Variant                mPropertyData;                /// Property data interpreted by the user
  Variant                mLastValue;                   /// Last observed property value
  TimeMs                 mLastChangeTimestamp;         /// Timestamp indicating when this replica property was last changed (on any primitive member)
  Variant                mLastReceivedChangeValue;     /// Last received property change value
  TimeMs                 mLastReceivedChangeTimestamp; /// Last received property change timestamp
  uint64                 mLastReceivedChangeFrameId;   /// Last received property change frame ID
  Math::SplineCurve      mSplineCurve[4];              /// Received property change value curve (for each primitive member)
  Math::BakedCurve       mBakedCurve[4];               /// Received property change value curve baked out (for each primitive member)
  ConvergenceState::Enum mConvergenceState;            /// Convergence method currently being applied to this replica property
};

/// Typedefs
typedef UniquePointer<ReplicaProperty>                                        ReplicaPropertyPtr;
typedef ArraySet< ReplicaPropertyPtr, PointerSortPolicy<ReplicaPropertyPtr> > ReplicaPropertySet;
typedef InList<ReplicaProperty, &ReplicaProperty::mIndexListLink>             ReplicaPropertyList;

//---------------------------------------------------------------------------------//
//                             ReplicaPropertyIndex                                //
//---------------------------------------------------------------------------------//

/// Replica Property Index
/// Evenly distributes a collection of replica properties
class ReplicaPropertyIndex
{
public:
  /// Typedefs
  typedef ReplicaPropertyList     ListType;
  typedef Pair<size_t, ListType>  PairType;
  typedef UniquePointer<PairType> ValueType;
  typedef Array<ValueType>        ArrayType;

  /// Constructor
  ReplicaPropertyIndex();

  /// Destructor
  ~ReplicaPropertyIndex();

  /// Returns true if the index is empty (contains no replica properties), else false
  bool IsEmpty() const;

  /// Creates the specified number of lists
  /// (Not safe to call if the index is populated)
  void CreateLists(uint count);

  /// Returns the list at the specified index, else nullptr
  ReplicaPropertyList* GetList(size_t index);

  /// Returns the number of lists in the index
  size_t GetListCount() const;

  /// Inserts the replica property into the smallest internal list
  /// (Linear operation with respect to list count, checks every list's precomputed size and selects the smallest list to insert into)
  void Insert(ReplicaProperty* property);

  /// Removes the replica property from it's internal list
  /// (Constant operation, simply unlinks the intrusive list node and decrements the list's size value)
  void Remove(ReplicaProperty* property);

  /// Data
  ArrayType mPropertyLists; /// Replica property lists
  size_t    mPropertyCount; /// Replica property count
};

//---------------------------------------------------------------------------------//
//                             ReplicaPropertyType                                 //
//---------------------------------------------------------------------------------//

/// Replica Property Type
/// Configures the replication of a single property
class ReplicaPropertyType
{
public:
  /// Constructor
  ReplicaPropertyType(const String& name, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn);

  /// Destructor
  REPLICA_PROPERTY_TYPE_VIRTUAL ~ReplicaPropertyType();

  /// Comparison Operators (compares names)
  bool operator ==(const ReplicaPropertyType& rhs) const;
  bool operator !=(const ReplicaPropertyType& rhs) const;
  bool operator  <(const ReplicaPropertyType& rhs) const;
  bool operator ==(const String& rhs) const;
  bool operator !=(const String& rhs) const;
  bool operator  <(const String& rhs) const;

  //
  // Operations
  //

  /// Replica property type name
  const String& GetName() const;

  /// Property type's native type
  NativeType* GetNativeType() const;
  /// Property type's native type ID
  NativeTypeId GetNativeTypeId() const;

  /// Property value serializer function
  SerializeValueFn GetSerializeValueFn() const;
  /// Property value getter function
  GetValueFn GetGetValueFn() const;
  /// Property value setter function
  SetValueFn GetSetValueFn() const;

  /// Returns true if the replica property type is valid (registered with the replicator), else false
  bool IsValid() const;

  /// Makes the replica property type valid, called immediately after being registered with the replicator
  void MakeValid(Replicator* replicator);

  /// Operating replicator (set if the replica property type is valid)
  void SetReplicator(Replicator* replicator);
  Replicator* GetReplicator() const;

  /// Converges all scheduled replica properties of this type
  void ConvergeNow();
  void ConvergeNow(bool active, ReplicaPropertyIndex& replicaPropertyIndex, TimeMs timestamp, uint64 frameId);

  /// Schedules the unscheduled replica property for change convergence
  void ScheduleProperty(ReplicaProperty* property);
  /// Unschedules the replica property from change convergence
  void UnscheduleProperty(ReplicaProperty* property);

  //
  // Configuration
  //

  /// Resets all configuration settings
  void ResetConfig();

  /// Controls whether or not to use a delta threshold at which a replica property's primitive-components are considered changed during change detection
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetUseDeltaThreshold(bool useDeltaThreshold = false);
  bool GetUseDeltaThreshold() const;

  /// Controls the delta threshold at which a replica property's primitive-components are considered changed during change detection
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetDeltaThreshold(const Variant& deltaThreshold = Variant());
  const Variant& GetDeltaThreshold() const;

  /// Controls how replica properties are serialized
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetSerializationMode(SerializationMode::Enum serializationMode = SerializationMode::All);
  SerializationMode::Enum GetSerializationMode() const;

  /// Controls whether or not a floating-point replica property's primitive-components are converted to half floats during serialization
  /// (Only used with floating-point replica property primitive-component types)
  /// (Using half floats is mutually exclusive with using quantization)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetUseHalfFloats(bool useHalfFloats = false);
  bool GetUseHalfFloats() const;

  /// Controls whether or not a replica property's primitive-components are quantized (bit-packed to use only the bits necessary to represent all possible values) during serialization
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Quantization uses the specified delta threshold as a quantum interval value)
  /// (Quantization is mutually exclusive with using half floats)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetUseQuantization(bool useQuantization = false);
  bool GetUseQuantization() const;

  /// Controls the minimum, inclusive value at which a replica property's primitive-components may be quantized during serialization
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetQuantizationRangeMin(const Variant& quantizationRangeMin = Variant());
  const Variant& GetQuantizationRangeMin() const;

  /// Controls the maximum, inclusive value at which a replica property's primitive-components may be quantized during serialization
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetQuantizationRangeMax(const Variant& quantizationRangeMax = Variant());
  const Variant& GetQuantizationRangeMax() const;

  /// Controls whether or not to interpolate a replica property's received authoritative values before sampling them locally
  /// (Enable to improve changing value smoothness, at the expense of some small CPU and memory impact)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetUseInterpolation(bool useInterpolation = false);
  bool GetUseInterpolation() const;

  /// Controls the type of curve to use when interpolating a replica property's authoritative values to be sampled later locally
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetInterpolationCurve(Math::CurveType::Enum interpolationCurve = Math::CurveType::CatmulRom);
  Math::CurveType::Enum GetInterpolationCurve() const;

  /// Controls the time offset from now to sample a replica property's interpolated authoritative values
  /// (This is effectively how "forward" or "backward" in time we are sampling interpolated authority values)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetSampleTimeOffset(TimeMs sampleTimeOffset = TimeMs(100));
  TimeMs GetSampleTimeOffset() const;

  /// Controls the maximum amount of time to extrapolate beyond a replica property's last received authoritative value
  /// When sampling beyond this extrapolation limit, the sampled value will remain unchanged until the next authoritative value is received
  /// (Helps minimize the negative effects of missing changes or sparse change detection intervals when applied to non-deterministic property data)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetExtrapolationLimit(TimeMs extrapolationLimit = TimeMs(1000));
  TimeMs GetExtrapolationLimit() const;

  /// Controls whether or not to gradually converge a replica property's locally simulated values with received authoritative values
  /// (Enable to improve changing value smoothness, at the expense of some small CPU and memory impact)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetUseConvergence(bool useConvergence = false);
  bool GetUseConvergence() const;

  /// Controls whether or not replica properties should call Replicator::OnReplicaChannelPropertyConvergenceStateChange when its convergence state changes
  /// (Only used with arithmetic replica property primitive-component types)
  void SetNotifyOnConvergenceStateChange(bool notifyOnConvergenceStateChange = false);
  bool GetNotifyOnConvergenceStateChange() const;

  /// Controls the weight of an actively changing replica property's sampled authoritative values when performing active convergence, applied every convergence interval
  /// (Setting this to 0 will effectively never converge, and setting this to 1 will effectively always snap)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetActiveConvergenceWeight(float activeConvergenceWeight = float(0.1));
  float GetActiveConvergenceWeight() const;

  /// Controls the elapsed duration in which a resting replica property will be fully converged with the last received authoritative value, applied every convergence interval
  /// (Setting this to 0 will effectively snap to the last received property value immediately on rest)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetRestingConvergenceDuration(TimeMs restingConvergenceDuration = TimeMs(50));
  TimeMs GetRestingConvergenceDuration() const;

  /// Controls the frame interval in which a replica property's locally simulated values are converged with sampled authoritative values
  /// (Increase to spread out convergence-related property setter calls which reduces CPU impact at the expense of convergence smoothness)
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetConvergenceInterval(uint convergenceInterval = 1);
  uint GetConvergenceInterval() const;

  /// Controls the threshold at which to snap a replica property's locally simulated values to sampled authoritative values instead of gradually converging
  /// (Only used with arithmetic replica property primitive-component types)
  /// (Cannot be modified after the replica property type has been made valid)
  void SetSnapThreshold(const Variant& snapThreshold = Variant());
  const Variant& GetSnapThreshold() const;

  /// Data
  String                  mName;                           /// Replica property type name
  NativeType*             mNativeType;                     /// Property type's native type
  SerializeValueFn        mSerializeValueFn;               /// Property value serializer function
  GetValueFn              mGetValueFn;                     /// Property value getter function
  SetValueFn              mSetValueFn;                     /// Property value setter function
  Replicator*             mReplicator;                     /// Operating replicator
  ReplicaPropertyIndex    mActivePropertyIndex;            /// Active replica properties index
  ReplicaPropertyIndex    mRestingPropertyIndex;           /// Resting replica properties index
  bool                    mUseDeltaThreshold;              /// Use delta threshold?
  Variant                 mDeltaThreshold;                 /// Delta threshold
  SerializationMode::Enum mSerializationMode;              /// Serialization mode
  bool                    mUseHalfFloats;                  /// Use half floats?
  bool                    mUseQuantization;                /// Use quantization?
  Variant                 mQuantizationRangeMin;           /// Quantization range minimum
  Variant                 mQuantizationRangeMax;           /// Quantization range maximum
  bool                    mUseInterpolation;               /// Use interpolation?
  Math::CurveType::Enum   mInterpolationCurve;             /// Interpolation curve type
  TimeMs                  mSampleTimeOffset;               /// Sample time offset from now
  TimeMs                  mExtrapolationLimit;             /// Extrapolation time limit
  bool                    mUseConvergence;                 /// Use convergence?
  bool                    mNotifyOnConvergenceStateChange; /// Notify on convergence state change?
  float                   mActiveConvergenceWeight;        /// Active convergence weight applied every convergence interval
  TimeMs                  mRestingConvergenceDuration;     /// Resting convergence duration handled every convergence interval
  uint                    mConvergenceInterval;            /// Convergence interval
  Variant                 mSnapThreshold;                  /// Snap-instead-of-converge threshold
};

/// Typedefs
typedef UniquePointer<ReplicaPropertyType>                                            ReplicaPropertyTypePtr;
typedef ArraySet< ReplicaPropertyTypePtr, PointerSortPolicy<ReplicaPropertyTypePtr> > ReplicaPropertyTypeSet;

} // namespace Zero
