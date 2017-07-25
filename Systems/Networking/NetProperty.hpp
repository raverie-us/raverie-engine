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
//                                 NetProperty                                     //
//---------------------------------------------------------------------------------//

/// Network Property.
/// Manages the replication of a single property on the network.
class NetProperty : public SafeId32Object, public ReplicaProperty
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetProperty(const String& name, NetPropertyType* netPropertyType, const Variant& propertyData);

  /// Destructor.
  ~NetProperty();

  //
  // Operations
  //

  /// Net property name.
  const String& GetName() const;

  /// Operating net property type.
  NetPropertyType* GetNetPropertyType() const;

  /// Operating net channel.
  NetChannel* GetNetChannel() const;

  /// Timestamp indicating when this net property was last changed, else 0.
  float GetLastChangeTimestamp() const;

  /// Elapsed time passed since this net property was last changed, else 0.
  float GetLastChangeTimePassed() const;
};

//---------------------------------------------------------------------------------//
//                               NetPropertyType                                   //
//---------------------------------------------------------------------------------//

/// Network Property Type.
/// Configures the replication of a single property on the network.
class NetPropertyType : public SafeId32Object, public ReplicaPropertyType
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPropertyType(const String& name, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn);

  /// Destructor.
  ~NetPropertyType();

  //
  // Operations
  //

  /// Net property type name.
  const String& GetName() const;

  //
  // Configuration
  //

  /// Resets all configuration settings.
  /// (Cannot be modified at game runtime)
  void ResetConfig();

  /// Sets all configuration settings according to the specified NetPropertyConfig resource.
  /// (Cannot be modified at game runtime)
  void SetConfig(NetPropertyConfig* netPropertyConfig);
};

//---------------------------------------------------------------------------------//
//                              NetPropertyConfig                                  //
//---------------------------------------------------------------------------------//

// Variant Configuration Helper Macros
#define DeclareVariantGetSetForArithmeticTypes(property, defaultFloat, defaultInt) \
static constexpr float DefaultFloat##property = defaultFloat;                      \
static constexpr int   DefaultInt##property   = defaultInt;                        \
DeclareVariantGetSetForType(property, Integer,       int);                         \
DeclareVariantGetSetForType(property, DoubleInteger, s64);                         \
DeclareVariantGetSetForType(property, Integer2,      Integer2);                    \
DeclareVariantGetSetForType(property, Integer3,      Integer3);                    \
DeclareVariantGetSetForType(property, Integer4,      Integer4);                    \
DeclareVariantGetSetForType(property, Real,          float);                       \
DeclareVariantGetSetForType(property, DoubleReal,    double);                      \
DeclareVariantGetSetForType(property, Real2,         Real2);                       \
DeclareVariantGetSetForType(property, Real3,         Real3);                       \
DeclareVariantGetSetForType(property, Real4,         Real4);                       \
DeclareVariantGetSetForType(property, Quaternion,    Quaternion)

#define DeclareVariantGetSetForType(property, typeName, type) \
void Set##property##typeName(type value);                     \
type Get##property##typeName() const

/// Network Property Configuration.
/// Defines a configuration for the replication of a single property on the network.
class NetPropertyConfig : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  NetPropertyConfig();

  //
  // Data Resource Interface
  //

  /// Serializes the net property configuration resource.
  void Serialize(Serializer& stream) override;

  //
  // Operations
  //

  /// Net property configuration name.
  const String& GetName() const;

  /// Translates our variant properties into the target network property type (where possible, else defaults).
  void TranslateVariantProperties();

  //
  // Configuration
  //

  /// Controls the target network property type.
  void SetBasicNetType(BasicNetType::Enum basicNetType);
  BasicNetType::Enum GetBasicNetType() const;

  /// Controls whether or not to use a delta threshold at which a net property's primitive-components are considered changed during change detection.
  void SetUseDeltaThreshold(bool useDeltaThreshold);
  bool GetUseDeltaThreshold() const;

  /// Controls the delta threshold at which a net property's primitive-components are considered changed during change detection.
  DeclareVariantGetSetForArithmeticTypes(DeltaThreshold, float(1), int(1));

  /// Controls how net properties are serialized.
  void SetSerializationMode(SerializationMode::Enum serializationMode);
  SerializationMode::Enum GetSerializationMode() const;

  /// Controls whether or not a floating-point net property's primitive-components are converted to half floats during serialization.
  /// (Using half floats is mutually exclusive with using quantization)
  void SetUseHalfFloats(bool useHalfFloats);
  bool GetUseHalfFloats() const;

  /// Controls whether or not a net property's primitive-components are quantized (bit-packed to use only the bits necessary to represent all possible values) during serialization.
  /// (Quantization uses the specified delta threshold as a quantum interval value)
  /// (Quantization is mutually exclusive with using half floats)
  void SetUseQuantization(bool useQuantization);
  bool GetUseQuantization() const;

  /// Controls the minimum, inclusive value at which a net property's primitive-components may be quantized during serialization.
  DeclareVariantGetSetForArithmeticTypes(QuantizationRangeMin, float(-1), int(-1));

  /// Controls the maximum, inclusive value at which a net property's primitive-components may be quantized during serialization.
  DeclareVariantGetSetForArithmeticTypes(QuantizationRangeMax, float(+1), int(+1));

  /// Controls whether or not to interpolate a net property's received authoritative values before sampling them locally.
  /// (Enable to improve changing value smoothness, at the expense of some small CPU and memory impact)
  void SetUseInterpolation(bool useInterpolation);
  bool GetUseInterpolation() const;

  /// Controls the type of curve to use when interpolating a net property's authoritative values to be sampled later locally.
  void SetInterpolationCurve(Math::CurveType::Enum interpolationCurve);
  Math::CurveType::Enum GetInterpolationCurve() const;

  /// Controls the time offset from now to sample a net property's interpolated authoritative values.
  /// (This is effectively how "forward" or "backward" in time we are sampling interpolated authority values)
  void SetSampleTimeOffset(float sampleTimeOffset);
  float GetSampleTimeOffset() const;

  /// Controls the maximum amount of time to extrapolate beyond a net property's last received authoritative value.
  /// When sampling beyond this extrapolation limit, the sampled value will remain unchanged until the next authoritative value is received.
  /// (Helps minimize the negative effects of missing changes or sparse change detection intervals when applied to non-deterministic property data)
  void SetExtrapolationLimit(float extrapolationLimit);
  float GetExtrapolationLimit() const;

  /// Controls whether or not to gradually converge a net property's locally simulated values with received authoritative values.
  /// (Enable to improve changing value smoothness, at the expense of some small CPU and memory impact)
  void SetUseConvergence(bool useConvergence);
  bool GetUseConvergence() const;

  /// Controls whether or not net properties should dispatch NetChannelPropertyConvergenceStateChange when its convergence state changes.
  void SetEventOnConvergenceStateChange(bool eventOnConvergenceStateChange);
  bool GetEventOnConvergenceStateChange() const;

  /// Controls the weight of an actively changing net property's sampled authoritative values when performing active convergence, applied every convergence interval.
  /// (Setting this to 0 will effectively never converge, and setting this to 1 will effectively always snap)
  void SetActiveConvergenceWeight(float activeConvergenceWeight);
  float GetActiveConvergenceWeight() const;

  /// Controls the elapsed duration in which a resting net property will be fully converged with the last received authoritative value, applied every convergence interval.
  /// (Setting this to 0 will effectively snap to the last received property value immediately on rest)
  void SetRestingConvergenceDuration(float restingConvergenceDuration);
  float GetRestingConvergenceDuration() const;

  /// Controls the frame interval in which a net property's locally simulated values are converged with sampled authoritative values.
  /// (Increase to spread out convergence-related property setter calls which reduces CPU impact at the expense of convergence smoothness)
  void SetConvergenceInterval(uint convergenceInterval);
  uint GetConvergenceInterval() const;

  /// Controls the threshold at which to snap a net property's locally simulated values to sampled authoritative values instead of gradually converging.
  DeclareVariantGetSetForArithmeticTypes(SnapThreshold, float(10), int(10));

  // Data
  BasicNetType::Enum      mBasicNetType;                  ///< Target basic property type.
  bool                    mUseDeltaThreshold;             ///< Use delta threshold?
  Variant                 mDeltaThreshold;                ///< Delta threshold.
  SerializationMode::Enum mSerializationMode;             ///< Serialization mode.
  bool                    mUseHalfFloats;                 ///< Use half floats?
  bool                    mUseQuantization;               ///< Use quantization?
  Variant                 mQuantizationRangeMin;          ///< Quantization range minimum.
  Variant                 mQuantizationRangeMax;          ///< Quantization range maximum.
  bool                    mUseInterpolation;              ///< Use interpolation?
  Math::CurveType::Enum   mInterpolationCurve;            ///< Interpolation curve type.
  float                   mSampleTimeOffset;              ///< Sample time offset from now.
  float                   mExtrapolationLimit;            ///< Extrapolation time limit.
  bool                    mUseConvergence;                ///< Use convergence?
  bool                    mEventOnConvergenceStateChange; ///< Event on convergence state change?
  float                   mActiveConvergenceWeight;       ///< Active convergence weight applied every convergence interval.
  float                   mRestingConvergenceDuration;    ///< Resting convergence duration handled every convergence interval.
  uint                    mConvergenceInterval;           ///< Convergence interval.
  Variant                 mSnapThreshold;                 ///< Snap-instead-of-converge threshold.
};

// Variant Configuration Helper Macros
#undef DeclareVariantGetSetForArithmeticTypes
#undef DeclareVariantGetSetForType

//---------------------------------------------------------------------------------//
//                           NetPropertyConfigManager                              //
//---------------------------------------------------------------------------------//

/// Manages all NetPropertyConfig resources.
class NetPropertyConfigManager : public ResourceManager
{
public:
  /// Initialize ResourceManager.
  DeclareResourceManager(NetPropertyConfigManager, NetPropertyConfig);

  /// Constructor.
  NetPropertyConfigManager(BoundType* resourceType);
};

//---------------------------------------------------------------------------------//
//                               NetPropertyInfo                                   //
//---------------------------------------------------------------------------------//

/// Network Property Info.
/// Configures a network property.
class NetPropertyInfo : public SafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructors.
  NetPropertyInfo();
  NetPropertyInfo(BoundType* componentType, StringParam propertyName);

  /// Comparison Operators (compares component type and property names).
  bool operator ==(const NetPropertyInfo& rhs) const;
  bool operator !=(const NetPropertyInfo& rhs) const;
  bool operator ==(const Pair<BoundType*, String>& rhs) const;
  bool operator !=(const Pair<BoundType*, String>& rhs) const;

  //
  // Serialization Interface
  //

  /// Serializes the net property info.
  void Serialize(Serializer& stream);

  /// Initializes the net property info.
  void SetDefaults();

  //
  // Property Interface
  //

  /// Component's type name (the component type which defines the property being configured).
  void SetComponentName(StringParam componentName);
  String GetComponentName();

  /// Property's variable name (the property being configured).
  String GetPropertyName();

  /// Network channel configuration resource controlling how this property is grouped and replicated.
  void SetNetChannelConfig(NetChannelConfig* netChannelConfig);
  NetChannelConfig* GetNetChannelConfig();

  /// Network property configuration resource controlling how this property is replicated.
  void SetNetPropertyConfig(NetPropertyConfig* netPropertyConfig);
  NetPropertyConfig* GetNetPropertyConfig();

  //
  // Data
  //

  /// Component meta type.
  BoundType*                  mComponentType;
  /// Property variable name.
  String                      mPropertyName;
  /// Network channel configuration resource.
  HandleOf<NetChannelConfig>  mNetChannelConfig;
  /// Network property configuration resource.
  HandleOf<NetPropertyConfig> mNetPropertyConfig;
};

/// Typedefs.
typedef Array<NetPropertyInfo> NetPropertyInfoArray;

#define DeclarePropertyFilterForType(typeName)                                        \
class PropertyFilter##typeName : public MetaPropertyFilter                            \
{                                                                                     \
public:                                                                               \
  ZilchDeclareType(TypeCopyMode::ReferenceType);                                      \
  bool Filter(Property* prop, HandleParam instance) override;                         \
}

// Variant Configuration Property Filters
DeclarePropertyFilterForType(Other);
DeclarePropertyFilterForType(Boolean);
DeclarePropertyFilterForType(Integer);
DeclarePropertyFilterForType(DoubleInteger);
DeclarePropertyFilterForType(Integer2);
DeclarePropertyFilterForType(Integer3);
DeclarePropertyFilterForType(Integer4);
DeclarePropertyFilterForType(Real);
DeclarePropertyFilterForType(DoubleReal);
DeclarePropertyFilterForType(Real2);
DeclarePropertyFilterForType(Real3);
DeclarePropertyFilterForType(Real4);
DeclarePropertyFilterForType(Quaternion);
DeclarePropertyFilterForType(String);

class PropertyFilterMultiPrimitiveTypes : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Filter(Property* prop, HandleParam instance) override;
};

class PropertyFilterFloatingPointTypes : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Filter(Property* prop, HandleParam instance) override;
};

class PropertyFilterArithmeticTypes : public MetaPropertyFilter
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  bool Filter(Property* prop, HandleParam instance) override;
};

} // namespace Zero
