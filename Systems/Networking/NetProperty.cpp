///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

/// Translates the specified variant into the target type (if possible) and returns the result, else Variant().
Variant TranslateVariant(const Variant& originalValue, BasicNetType::Enum targetBasicNetType)
{
  // Get original and target native types
  NativeType* originalNativeType = originalValue.GetNativeType();
  NativeType* targetNativeType   = GetNativeTypeByConstantId((NativeTypeId)BasicNetworkToNativeTypeEnum(targetBasicNetType));

  // Either the original or target native type is not a basic type?
  if(originalNativeType == nullptr
  || targetNativeType   == nullptr)
  {
    // (We can only convert between basic native types)
    return Variant();
  }

  // Convert original value into string
  String originalValueString = originalValue.ToString();

  // Convert string value into target type
  Variant resultValue;
  resultValue.ToValue(originalValueString, targetNativeType);

  // Success
  return resultValue;
}

//---------------------------------------------------------------------------------//
//                                 NetProperty                                     //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetProperty, builder, type)
{
  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Bind operations
  ZilchBindGetterProperty(Name);
  ZilchBindGetterProperty(NetPropertyType);
  ZilchBindGetterProperty(NetChannel);
  ZilchBindGetterProperty(LastChangeTimestamp);
  ZilchBindGetterProperty(LastChangeTimePassed);
}

NetProperty::NetProperty(const String& name, NetPropertyType* netPropertyType, const Variant& propertyData)
  : ReplicaProperty(name, netPropertyType, propertyData)
{
}

NetProperty::~NetProperty()
{
}

//
// Operations
//

const String& NetProperty::GetName() const
{
  return ReplicaProperty::GetName();
}

NetPropertyType* NetProperty::GetNetPropertyType() const
{
  return static_cast<NetPropertyType*>(ReplicaProperty::GetReplicaPropertyType());
}

NetChannel* NetProperty::GetNetChannel() const
{
  return static_cast<NetChannel*>(ReplicaProperty::GetReplicaChannel());
}

float NetProperty::GetLastChangeTimestamp() const
{
  // Get last change timestamp
  TimeMs timestamp = ReplicaProperty::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  return TimeMsToFloatSeconds(timestamp);
}

float NetProperty::GetLastChangeTimePassed() const
{
  // Get replicator
  Replicator* replicator = ReplicaProperty::GetReplicator();
  if(!replicator) // Unable?
    return 0;

  // Get current time
  TimeMs now = replicator->GetPeer()->GetLocalTime();

  // Get last change timestamp
  TimeMs timestamp = ReplicaProperty::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  // Compute time passed since last change (duration between now and last change timestamp)
  TimeMs timePassed = (now - timestamp);
  return TimeMsToFloatSeconds(timePassed);
}

//---------------------------------------------------------------------------------//
//                               NetPropertyType                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPropertyType, builder, type)
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
}

NetPropertyType::NetPropertyType(const String& name, NativeType* nativeType, SerializeValueFn serializeValueFn, GetValueFn getValueFn, SetValueFn setValueFn)
  : ReplicaPropertyType(name, nativeType, serializeValueFn, getValueFn, setValueFn)
{
  ResetConfig();
}

NetPropertyType::~NetPropertyType()
{
}

//
// Operations
//

const String& NetPropertyType::GetName() const
{
  return ReplicaPropertyType::GetName();
}

//
// Configuration
//

void NetPropertyType::ResetConfig()
{
  // Not valid yet?
  if(!IsValid())
  {
    // Set non-runtime config options
    SetDeltaThreshold();
    SetUseDeltaThreshold();
    SetSerializationMode();
    SetUseHalfFloats();
    SetUseQuantization();
    SetQuantizationRangeMin();
    SetQuantizationRangeMax();
    SetUseInterpolation();
    SetInterpolationCurve();
    SetSampleTimeOffset();
    SetExtrapolationLimit();
    SetUseConvergence();
    SetActiveConvergenceWeight();
    SetRestingConvergenceDuration();
    SetConvergenceInterval();
    SetSnapThreshold();
  }

  // Set runtime config options
  SetNotifyOnConvergenceStateChange();
}

void NetPropertyType::SetConfig(NetPropertyConfig* netPropertyConfig)
{
  // Get config's target network property type
  BasicNetType::Enum configBasicNetType = netPropertyConfig->GetBasicNetType();

  // Non-arithmetic type?
  if(configBasicNetType == BasicNetType::Other
  || configBasicNetType == BasicNetType::Boolean
  || configBasicNetType == BasicNetType::String)
    return; // Unable to translate

  // Get our network property type
  BasicNetType::Enum ourBasicNetType = BasicNativeToNetworkTypeEnum((BasicNativeType::Enum)ReplicaPropertyType::GetNativeTypeId());

  // Non-arithmetic type?
  if(ourBasicNetType == BasicNetType::Other
  || ourBasicNetType == BasicNetType::Boolean
  || ourBasicNetType == BasicNetType::String)
    return; // Unable to translate

  // Not valid yet?
  if(!IsValid())
  {
    // Get config variant property types
    Variant deltaThreshold       = netPropertyConfig->mDeltaThreshold;
    Variant quantizationRangeMin = netPropertyConfig->mQuantizationRangeMin;
    Variant quantizationRangeMax = netPropertyConfig->mQuantizationRangeMax;
    Variant snapThreshold        = netPropertyConfig->mSnapThreshold;

    // Need to translate config variant property types?
    if(ourBasicNetType != configBasicNetType)
    {
      deltaThreshold       = TranslateVariant(deltaThreshold,       ourBasicNetType);
      quantizationRangeMin = TranslateVariant(quantizationRangeMin, ourBasicNetType);
      quantizationRangeMax = TranslateVariant(quantizationRangeMax, ourBasicNetType);
      snapThreshold        = TranslateVariant(snapThreshold,        ourBasicNetType);
    }

    // Set non-runtime config options
    SetDeltaThreshold(deltaThreshold);
    SetUseDeltaThreshold(netPropertyConfig->mUseDeltaThreshold);
    SetSerializationMode(netPropertyConfig->mSerializationMode);
    SetUseHalfFloats(netPropertyConfig->mUseHalfFloats);
    SetUseQuantization(netPropertyConfig->mUseQuantization);
    SetQuantizationRangeMin(quantizationRangeMin);
    SetQuantizationRangeMax(quantizationRangeMax);
    SetUseInterpolation(netPropertyConfig->mUseInterpolation);
    SetInterpolationCurve(netPropertyConfig->mInterpolationCurve);
    SetSampleTimeOffset(FloatSecondsToTimeMs(netPropertyConfig->mSampleTimeOffset));
    SetExtrapolationLimit(FloatSecondsToTimeMs(netPropertyConfig->mExtrapolationLimit));
    SetUseConvergence(netPropertyConfig->mUseConvergence);
    SetActiveConvergenceWeight(netPropertyConfig->mActiveConvergenceWeight);
    SetRestingConvergenceDuration(FloatSecondsToTimeMs(netPropertyConfig->mRestingConvergenceDuration));
    SetConvergenceInterval(netPropertyConfig->mConvergenceInterval);
    SetSnapThreshold(snapThreshold);
  }

  // Set runtime config options
  SetNotifyOnConvergenceStateChange(netPropertyConfig->mEventOnConvergenceStateChange);
}

//---------------------------------------------------------------------------------//
//                              NetPropertyConfig                                  //
//---------------------------------------------------------------------------------//

// Variant Configuration Helper Macros
#define DefineVariantGetSetForArithmeticTypes(property)                                                                                                                     \
DefineVariantGetSetForType(property, Integer,       int,        int(DefaultInt##property));                                                                                 \
DefineVariantGetSetForType(property, DoubleInteger, s64,        s64(DefaultInt##property));                                                                                 \
DefineVariantGetSetForType(property, Integer2,      Integer2,   Integer2(DefaultInt##property, DefaultInt##property));                                                      \
DefineVariantGetSetForType(property, Integer3,      Integer3,   Integer3(DefaultInt##property, DefaultInt##property, DefaultInt##property));                                \
DefineVariantGetSetForType(property, Integer4,      Integer4,   Integer4(DefaultInt##property, DefaultInt##property, DefaultInt##property, DefaultInt##property));          \
DefineVariantGetSetForType(property, Real,          float,      float(DefaultFloat##property));                                                                             \
DefineVariantGetSetForType(property, DoubleReal,    double,     double(DefaultFloat##property));                                                                            \
DefineVariantGetSetForType(property, Real2,         Real2,      Real2(DefaultFloat##property, DefaultFloat##property));                                                     \
DefineVariantGetSetForType(property, Real3,         Real3,      Real3(DefaultFloat##property, DefaultFloat##property, DefaultFloat##property));                             \
DefineVariantGetSetForType(property, Real4,         Real4,      Real4(DefaultFloat##property, DefaultFloat##property, DefaultFloat##property, DefaultFloat##property));     \
DefineVariantGetSetForType(property, Quaternion,    Quaternion, Quaternion(DefaultFloat##property, DefaultFloat##property, DefaultFloat##property, DefaultFloat##property))

#define DefineVariantGetSetForType(property, typeName, type, defaultValue)         \
void NetPropertyConfig::Set##property##typeName(type value)                        \
{                                                                                  \
  m##property = value;                                                             \
}                                                                                  \
type NetPropertyConfig::Get##property##typeName() const                            \
{                                                                                  \
  return m##property.GetOrDefault<type>(defaultValue);                             \
}

#define BindVariantGetSetForArithmeticTypes(property) \
BindVariantGetSetForType(property, Integer);          \
BindVariantGetSetForType(property, DoubleInteger);    \
BindVariantGetSetForType(property, Integer2);         \
BindVariantGetSetForType(property, Integer3);         \
BindVariantGetSetForType(property, Integer4);         \
BindVariantGetSetForType(property, Real);             \
BindVariantGetSetForType(property, DoubleReal);       \
BindVariantGetSetForType(property, Real2);            \
BindVariantGetSetForType(property, Real3);            \
BindVariantGetSetForType(property, Real4);            \
BindVariantGetSetForType(property, Quaternion)

#define BindVariantGetSetForType(property, typeName)                                 \
ZilchBindGetterSetterProperty(property##typeName)->Add(new PropertyFilter##typeName)

#define DefinePropertyFilterForType(typeName)                                             \
ZilchDefineType(PropertyFilter##typeName, builder, type)                                  \
{                                                                                         \
}                                                                                         \
bool PropertyFilter##typeName::Filter(Property* prop, HandleParam instance)               \
{                                                                                         \
  return (instance.Get<NetPropertyConfig*>()->mBasicNetType == BasicNetType::##typeName); \
}

// Variant Configuration Property Filters
DefinePropertyFilterForType(Other);
DefinePropertyFilterForType(Boolean);
DefinePropertyFilterForType(Integer);
DefinePropertyFilterForType(DoubleInteger);
DefinePropertyFilterForType(Integer2);
DefinePropertyFilterForType(Integer3);
DefinePropertyFilterForType(Integer4);
DefinePropertyFilterForType(Real);
DefinePropertyFilterForType(DoubleReal);
DefinePropertyFilterForType(Real2);
DefinePropertyFilterForType(Real3);
DefinePropertyFilterForType(Real4);
DefinePropertyFilterForType(Quaternion);
DefinePropertyFilterForType(String);

ZilchDefineType(PropertyFilterMultiPrimitiveTypes, builder, type)
{
}

bool PropertyFilterMultiPrimitiveTypes::Filter(Property* prop, HandleParam instance)
{
  switch(instance.Get<NetPropertyConfig*>()->mBasicNetType)
  {
  default:
  case BasicNetType::Other:
  case BasicNetType::Boolean:
  case BasicNetType::Integer:
  case BasicNetType::DoubleInteger:
  case BasicNetType::Real:
  case BasicNetType::DoubleReal:
  case BasicNetType::String:
    return false;

  case BasicNetType::Integer2:
  case BasicNetType::Integer3:
  case BasicNetType::Integer4:
  case BasicNetType::Real2:
  case BasicNetType::Real3:
  case BasicNetType::Real4:
  case BasicNetType::Quaternion:
    return true;
  }
}

ZilchDefineType(PropertyFilterFloatingPointTypes, builder, type)
{
}

bool PropertyFilterFloatingPointTypes::Filter(Property* prop, HandleParam instance)
{
  switch(instance.Get<NetPropertyConfig*>()->mBasicNetType)
  {
  default:
  case BasicNetType::Other:
  case BasicNetType::Boolean:
  case BasicNetType::Integer:
  case BasicNetType::DoubleInteger:
  case BasicNetType::Integer2:
  case BasicNetType::Integer3:
  case BasicNetType::Integer4:
  case BasicNetType::String:
    return false;

  case BasicNetType::Real:
  case BasicNetType::DoubleReal:
  case BasicNetType::Real2:
  case BasicNetType::Real3:
  case BasicNetType::Real4:
  case BasicNetType::Quaternion:
    return true;
  }
}

ZilchDefineType(PropertyFilterArithmeticTypes, builder, type)
{
}

bool PropertyFilterArithmeticTypes::Filter(Property* prop, HandleParam instance)
{
  switch(instance.Get<NetPropertyConfig*>()->mBasicNetType)
  {
  default:
  case BasicNetType::Other:
  case BasicNetType::Boolean:
  case BasicNetType::String:
    return false;

  case BasicNetType::Integer:
  case BasicNetType::DoubleInteger:
  case BasicNetType::Integer2:
  case BasicNetType::Integer3:
  case BasicNetType::Integer4:
  case BasicNetType::Real:
  case BasicNetType::DoubleReal:
  case BasicNetType::Real2:
  case BasicNetType::Real3:
  case BasicNetType::Real4:
  case BasicNetType::Quaternion:
    return true;
  }
}

ZilchDefineType(NetPropertyConfig, builder, type)
{
  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind data members
  ZilchBindGetterSetterProperty(BasicNetType)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(UseDeltaThreshold)->Add(new PropertyFilterArithmeticTypes);
  BindVariantGetSetForArithmeticTypes(DeltaThreshold);
  ZilchBindGetterSetterProperty(SerializationMode)->Add(new PropertyFilterMultiPrimitiveTypes);
  ZilchBindGetterSetterProperty(UseHalfFloats)->AddAttributeChainable(PropertyAttributes::cInvalidatesObject)->Add(new PropertyFilterFloatingPointTypes);
  ZilchBindGetterSetterProperty(UseQuantization)->AddAttributeChainable(PropertyAttributes::cInvalidatesObject)->Add(new PropertyFilterArithmeticTypes);
  BindVariantGetSetForArithmeticTypes(QuantizationRangeMin);
  BindVariantGetSetForArithmeticTypes(QuantizationRangeMax);
  ZilchBindGetterSetterProperty(UseInterpolation)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(InterpolationCurve)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(SampleTimeOffset)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(ExtrapolationLimit)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(UseConvergence)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(EventOnConvergenceStateChange)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(ActiveConvergenceWeight)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(RestingConvergenceDuration)->Add(new PropertyFilterArithmeticTypes);
  ZilchBindGetterSetterProperty(ConvergenceInterval)->Add(new PropertyFilterArithmeticTypes);
  BindVariantGetSetForArithmeticTypes(SnapThreshold);
}

NetPropertyConfig::NetPropertyConfig()
  : mBasicNetType(BasicNetType::Other),
    mUseDeltaThreshold(false),
    mDeltaThreshold(),
    mSerializationMode(SerializationMode::All),
    mUseHalfFloats(false),
    mUseQuantization(false),
    mQuantizationRangeMin(),
    mQuantizationRangeMax(),
    mUseInterpolation(false),
    mInterpolationCurve(Math::CurveType::Linear),
    mSampleTimeOffset(0),
    mExtrapolationLimit(0),
    mUseConvergence(false),
    mEventOnConvergenceStateChange(false),
    mActiveConvergenceWeight(0),
    mRestingConvergenceDuration(0),
    mConvergenceInterval(0),
    mSnapThreshold()
{
}

//
// Data Resource Interface
//

void NetPropertyConfig::Serialize(Serializer& stream)
{
  // Serialize data members
  SerializeEnumNameDefault(BasicNetType, mBasicNetType, BasicNetType::Real);
  SerializeNameDefault(mUseDeltaThreshold, false);
  SerializeNameDefault(mDeltaThreshold, Variant(DefaultFloatDeltaThreshold));
  SerializeEnumNameDefault(SerializationMode, mSerializationMode, SerializationMode::All);
  SerializeNameDefault(mUseHalfFloats, false);
  SerializeNameDefault(mUseQuantization, false);
  SerializeNameDefault(mQuantizationRangeMin, Variant(DefaultFloatQuantizationRangeMin));
  SerializeNameDefault(mQuantizationRangeMax, Variant(DefaultFloatQuantizationRangeMax));
  SerializeNameDefault(mUseInterpolation, false);
  SerializeEnumNameDefault(Math::CurveType, mInterpolationCurve, Math::CurveType::CatmulRom);
  SerializeNameDefault(mSampleTimeOffset, float(0.1));
  SerializeNameDefault(mExtrapolationLimit, float(1));
  SerializeNameDefault(mUseConvergence, false);
  SerializeNameDefault(mEventOnConvergenceStateChange, false);
  SerializeNameDefault(mActiveConvergenceWeight, float(0.1));
  SerializeNameDefault(mRestingConvergenceDuration, float(0.05));
  SerializeNameDefault(mConvergenceInterval, uint(1));
  SerializeNameDefault(mSnapThreshold, Variant(DefaultFloatSnapThreshold));

  // Loading?
  if(stream.GetMode() == SerializerMode::Loading)
  {
    // Translate read in variant properties, just in case their types don't match our target type
    TranslateVariantProperties();
  }
}

//
// Operations
//

const String& NetPropertyConfig::GetName() const
{
  return Name;
}

void NetPropertyConfig::TranslateVariantProperties()
{
  // Non-arithmetic type?
  if(mBasicNetType == BasicNetType::Other
  || mBasicNetType == BasicNetType::Boolean
  || mBasicNetType == BasicNetType::String)
    return; // Unable to translate

  // Translate variant properties
  mDeltaThreshold       = TranslateVariant(mDeltaThreshold,       mBasicNetType);
  mQuantizationRangeMin = TranslateVariant(mQuantizationRangeMin, mBasicNetType);
  mQuantizationRangeMax = TranslateVariant(mQuantizationRangeMax, mBasicNetType);
  mSnapThreshold        = TranslateVariant(mSnapThreshold,        mBasicNetType);
}

//
// Configuration
//

void NetPropertyConfig::SetBasicNetType(BasicNetType::Enum basicNetType)
{
  mBasicNetType = basicNetType;
  TranslateVariantProperties();
}
BasicNetType::Enum NetPropertyConfig::GetBasicNetType() const
{
  return mBasicNetType;
}

void NetPropertyConfig::SetUseDeltaThreshold(bool useDeltaThreshold)
{
  mUseDeltaThreshold = useDeltaThreshold;

  // Not using delta threshold?
  if(!mUseDeltaThreshold)
    SetUseQuantization(false); // Disable quantization
}
bool NetPropertyConfig::GetUseDeltaThreshold() const
{
  return mUseDeltaThreshold;
}

DefineVariantGetSetForArithmeticTypes(DeltaThreshold);

void NetPropertyConfig::SetSerializationMode(SerializationMode::Enum serializationMode)
{
  mSerializationMode = serializationMode;
}
SerializationMode::Enum NetPropertyConfig::GetSerializationMode() const
{
  return mSerializationMode;
}

void NetPropertyConfig::SetUseHalfFloats(bool useHalfFloats)
{
  mUseHalfFloats = useHalfFloats;

  // Using half floats?
  if(mUseHalfFloats)
    SetUseQuantization(false); // Disable quantization
}
bool NetPropertyConfig::GetUseHalfFloats() const
{
  return mUseHalfFloats;
}

void NetPropertyConfig::SetUseQuantization(bool useQuantization)
{
  mUseQuantization = useQuantization;

  // Using quantization?
  if(mUseQuantization)
  {
    SetUseDeltaThreshold(true); // Enable delta threshold
    SetUseHalfFloats(false);    // Disable half floats
  }
}
bool NetPropertyConfig::GetUseQuantization() const
{
  return mUseQuantization;
}

DefineVariantGetSetForArithmeticTypes(QuantizationRangeMin);

DefineVariantGetSetForArithmeticTypes(QuantizationRangeMax);

void NetPropertyConfig::SetUseInterpolation(bool useInterpolation)
{
  mUseInterpolation = useInterpolation;
}
bool NetPropertyConfig::GetUseInterpolation() const
{
  return mUseInterpolation;
}

void NetPropertyConfig::SetInterpolationCurve(Math::CurveType::Enum interpolationCurve)
{
  mInterpolationCurve = interpolationCurve;
}
Math::CurveType::Enum NetPropertyConfig::GetInterpolationCurve() const
{
  return mInterpolationCurve;
}

void NetPropertyConfig::SetSampleTimeOffset(float sampleTimeOffset)
{
  mSampleTimeOffset = sampleTimeOffset;
}
float NetPropertyConfig::GetSampleTimeOffset() const
{
  return mSampleTimeOffset;
}

void NetPropertyConfig::SetExtrapolationLimit(float extrapolationLimit)
{
  mExtrapolationLimit = extrapolationLimit;
}
float NetPropertyConfig::GetExtrapolationLimit() const
{
  return mExtrapolationLimit;
}

void NetPropertyConfig::SetUseConvergence(bool useConvergence)
{
  mUseConvergence = useConvergence;
}
bool NetPropertyConfig::GetUseConvergence() const
{
  return mUseConvergence;
}

void NetPropertyConfig::SetEventOnConvergenceStateChange(bool eventOnConvergenceStateChange)
{
  mEventOnConvergenceStateChange = eventOnConvergenceStateChange;
}
bool NetPropertyConfig::GetEventOnConvergenceStateChange() const
{
  return mEventOnConvergenceStateChange;
}

void NetPropertyConfig::SetActiveConvergenceWeight(float activeConvergenceWeight)
{
  mActiveConvergenceWeight = activeConvergenceWeight;
}
float NetPropertyConfig::GetActiveConvergenceWeight() const
{
  return mActiveConvergenceWeight;
}

void NetPropertyConfig::SetRestingConvergenceDuration(float restingConvergenceDuration)
{
  mRestingConvergenceDuration = restingConvergenceDuration;
}
float NetPropertyConfig::GetRestingConvergenceDuration() const
{
  return mRestingConvergenceDuration;
}

void NetPropertyConfig::SetConvergenceInterval(uint convergenceInterval)
{
  mConvergenceInterval = convergenceInterval;
}
uint NetPropertyConfig::GetConvergenceInterval() const
{
  return mConvergenceInterval;
}

DefineVariantGetSetForArithmeticTypes(SnapThreshold);

// Variant Configuration Helper Macros
#undef DefineVariantGetSetForArithmeticTypes
#undef DefineVariantGetSetForType
#undef BindVariantGetSetForArithmeticTypes
#undef BindVariantGetSetForType
#undef DefinePropertyFilterForType

//---------------------------------------------------------------------------------//
//                           NetPropertyConfigManager                              //
//---------------------------------------------------------------------------------//

ImplementResourceManager(NetPropertyConfigManager, NetPropertyConfig);
NetPropertyConfigManager::NetPropertyConfigManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("NetPropertyConfig", new TextDataFileLoader<NetPropertyConfigManager>());
  mCategory = "Networking";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.NetPropertyConfig.data"));
  DefaultResourceName = "Default";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

//---------------------------------------------------------------------------------//
//                               NetPropertyInfo                                   //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetPropertyInfo, builder, type)
{
  // Bind documentation
  ZeroBindDocumented();

  // // Bind default constructor
  // ZilchBindDefaultConstructor();

  // Bind property interface
  ZilchBindCustomGetterPropertyAs(GetComponentName, "Component");
  ZilchBindCustomGetterPropertyAs(GetPropertyName,  "Property");
  ZilchBindGetterSetterProperty(NetChannelConfig);
  ZilchBindGetterSetterProperty(NetPropertyConfig);
}

NetPropertyInfo::NetPropertyInfo()
  : mComponentType(nullptr),
    mPropertyName(),
    mNetChannelConfig(),
    mNetPropertyConfig()
{
}
NetPropertyInfo::NetPropertyInfo(BoundType* componentType, StringParam propertyName)
  : mComponentType(componentType),
    mPropertyName(propertyName),
    mNetChannelConfig(NetChannelConfigManager::GetInstance()->FindOrNull("Default")),
    mNetPropertyConfig(NetPropertyConfigManager::GetInstance()->FindOrNull("Default"))
{
}

bool NetPropertyInfo::operator ==(const NetPropertyInfo& rhs) const
{
  return mComponentType == rhs.mComponentType
      && mPropertyName  == rhs.mPropertyName;
}
bool NetPropertyInfo::operator !=(const NetPropertyInfo& rhs) const
{
  return !(*this == rhs);
}
bool NetPropertyInfo::operator ==(const Pair<BoundType*, String>& rhs) const
{
  return mComponentType == rhs.first
      && mPropertyName  == rhs.second;
}
bool NetPropertyInfo::operator !=(const Pair<BoundType*, String>& rhs) const
{
  return !(*this == rhs);
}

//
// Serialization Interface
//

void NetPropertyInfo::Serialize(Serializer& stream)
{
  // Serialize target component name
  String componentName = GetComponentName();
  stream.SerializeFieldDefault("ComponentName", componentName, String());
  if(stream.GetMode() == SerializerMode::Loading) // Is loading?
    SetComponentName(componentName); // Set component type from the name we loaded

  // Serialize target property name
  SerializeNameDefault(mPropertyName, String());

  // Serialize net channel and property config resources
  SerializeResourceNameDefault(mNetChannelConfig, NetChannelConfigManager, "Default");
  SerializeResourceNameDefault(mNetPropertyConfig, NetPropertyConfigManager, "Default");
}

void NetPropertyInfo::SetDefaults()
{
}

//
// Property Interface
//

void NetPropertyInfo::SetComponentName(StringParam componentName)
{
  mComponentType = MetaDatabase::GetInstance()->FindType(componentName);
}
String NetPropertyInfo::GetComponentName()
{
  return mComponentType ? mComponentType->Name : String();
}

String NetPropertyInfo::GetPropertyName()
{
  return mPropertyName;
}

void NetPropertyInfo::SetNetChannelConfig(NetChannelConfig* netChannelConfig)
{
  mNetChannelConfig = netChannelConfig;
}
NetChannelConfig* NetPropertyInfo::GetNetChannelConfig()
{
  return mNetChannelConfig;
}

void NetPropertyInfo::SetNetPropertyConfig(NetPropertyConfig* netPropertyConfig)
{
  mNetPropertyConfig = netPropertyConfig;
}
NetPropertyConfig* NetPropertyInfo::GetNetPropertyConfig()
{
  return mNetPropertyConfig;
}

} // namespace Zero
