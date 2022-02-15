// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

struct PolymorphicNode;

// See SerializationAttributes::cSerializationPrimitive
#define ZeroBindSerializationPrimitive() type->AddAttribute(SerializationAttributes::cSerializationPrimitive)
#define ZeroBindSerializationPrimitiveExternal(type)                                                                   \
  ZilchTypeId(type)->AddAttribute(SerializationAttributes::cSerializationPrimitive)

// Meta Serialization
class MetaSerialization : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(MetaSerialization, TypeCopyMode::ReferenceType);

  virtual void SerializeProperty(HandleParam instance, Property* property, Serializer& serializer);

  virtual bool SerializePrimitiveProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer);
  virtual bool SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer);

  // The default serialize object assumes that the value inherits from Zero's
  // Object class
  virtual void SerializeObject(AnyParam object, Serializer& serializer);

  // Set the value of an any to a default value
  // This function MUST at least initialize the type of the any
  virtual void SetDefault(Type* type, Any& any);

  virtual void AddCustomAttributes(HandleParam object, TextSaver* saver)
  {
  }

  // If the type being serialized is a primitive serialization type,
  // then this must be overridden in a derived class.
  virtual String ConvertToString(AnyParam input);

  // Overload this if the value can be converted from a string and return true
  virtual bool ConvertFromString(StringParam input, Any& output);

  virtual void SerializeMembers(HandleParam object, Serializer& serializer);
};

// Serialization Primitive
class EnumMetaSerialization : public MetaSerialization
{
public:
  ZilchDeclareType(EnumMetaSerialization, TypeCopyMode::ReferenceType);

  EnumMetaSerialization(BoundType* enumType);

  bool SerializePrimitiveProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer) override;
  void SetDefault(Type* type, Any& any) override;
  String ConvertToString(AnyParam input) override;
  bool ConvertFromString(StringParam input, Any& output) override;

  BoundType* mEnumType;
};

// Serialization Primitive
template <typename T>
class PrimitiveMetaSerialization : public MetaSerialization
{
public:
  ZilchDeclareType(PrimitiveMetaSerialization, TypeCopyMode::ReferenceType);

  bool SerializePrimitiveProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer) override;
  String ConvertToString(AnyParam input) override;
  bool ConvertFromString(StringParam input, Any& output) override;
};

// String Serialization
class MetaStringSerialization : public MetaSerialization
{
public:
  ZilchDeclareType(MetaStringSerialization, TypeCopyMode::ReferenceType);

  bool SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer) override;
  String ConvertToString(AnyParam input) override;
  bool ConvertFromString(StringParam input, Any& output) override;
};

// Serialization Filter
class SerializationFilter : public ReferenceCountedEventObject
{
public:
  ZilchDeclareType(SerializationFilter, TypeCopyMode::ReferenceType);
  virtual bool ShouldSerialize(Object* object) = 0;
};

/// Serializes the given any. If it's saving, it has to be initialized
/// otherwise it will fail. If it's loading, the any does not need to
/// be initialized, but it must come from a source with type information (text).
bool SerializeAny(cstr fieldName, Any& value, Serializer& serializer);

/// Serializes the given variant. If it's saving, it has to be initialized
/// otherwise it will fail. If it's loading, the variant does not need to
/// be initialized, but it must come from a source with type information (text).
/// (NOTE: Currently, the given variant must be a basic native type.
/// This needs to be extended to support all stored types with serialization
/// capability.)
bool SerializeVariant(cstr fieldName, Variant& value, Serializer& serializer);

/// Serialize a Meta Property on an object
void SerializeProperty(HandleParam instance, Property* property, Serializer& serializer);

/// Serialize all components and properties on an object
void MetaSerializeObject(HandleParam instance, Serializer& serializer);

/// Serialize all components on an object
void MetaSerializeComponents(HandleParam instance, Serializer& serializer);

/// Serialize an object by using meta properties.
void MetaSerializeProperties(HandleParam instance, Serializer& serializer);

/// Utility for copying data between objects that are the same
/// Should not use if Serialize has adverse side effects
DataBlock SerializeObjectToDataBlock(Object* object);
void SerializeObjectFromDataBlock(DataBlock& block, Object* object);

namespace Serialization
{

template <>
struct Policy<Any>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Any& value)
  {
    return SerializeAny(fieldName, value, stream);
  }
};

template <>
struct Policy<Variant>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Variant& value)
  {
    return SerializeVariant(fieldName, value, stream);
  }
};

} // namespace Serialization

#define ZeroSerialize(DefaultValue)                                                                                    \
  AddAttributeChainable(PropertyAttributes::cSerialize)->Add(new MetaSerializedProperty(DefaultValue))

template <typename T>
bool PrimitiveMetaSerialization<T>::SerializePrimitiveProperty(BoundType* meta,
                                                               cstr fieldName,
                                                               Any& value,
                                                               Serializer& serializer)
{
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    if (value.IsNotNull())
    {
      T localValue = value.Get<T>();
      return Serialization::Policy<T>::Serialize(serializer, fieldName, localValue);
    }
    return true;
  }
  else
  {
    T localValue;
    if (Serialization::Policy<T>::Serialize(serializer, fieldName, localValue))
    {
      value = localValue;
      return true;
    }

    return false;
  }
}

template <typename T>
bool PrimitiveMetaSerialization<T>::ConvertFromString(StringParam input, Any& output)
{
  T value = T();
  ToValue(input.c_str(), value);
  CorrectNonFiniteValues(value);
  output = value;
  return true;
}

template <typename T>
String PrimitiveMetaSerialization<T>::ConvertToString(AnyParam input)
{
  return ToString(input.Get<T>());
}

} // namespace Zero
