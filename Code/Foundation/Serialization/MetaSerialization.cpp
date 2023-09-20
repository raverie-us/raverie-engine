// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Meta Serialization
RaverieDefineType(MetaSerialization, builder, type)
{
}

void MetaSerialization::SerializeProperty(HandleParam instance, Property* property, Serializer& serializer)
{
  BoundType* propertyType = Type::GetBoundType(property->PropertyType);
  ReturnIf(propertyType == nullptr, , "A property that was attempting to be serialized was not a BoundType");

  SerializerMode::Enum mode = serializer.GetMode();

  Any value;
  if (mode == SerializerMode::Saving)
  {
    value = property->GetValue(instance);
  }
  else if (mode == SerializerMode::Loading)
  {
    // If it's a reference type, attempt to get the already allocated object
    if (propertyType->CopyMode == TypeCopyMode::ReferenceType)
      value = property->GetValue(instance);
    else
      value.DefaultConstruct(propertyType);
  }

  // Checking to see if it's a serialization primitive or not is odd here.
  // MetaSerialization should be split up into MetaPrimitiveSerialization and
  // MetaObjectSerialization The String check is a hack until this change
  if (propertyType->CopyMode == TypeCopyMode::ReferenceType)
  {
    bool serialized = SerializeReferenceProperty(propertyType, property->Name.c_str(), value, serializer);

    // Set the object back in case we allocated it
    if (serialized)
      property->SetValue(instance, value);
  }
  else
  {
    bool serialized = SerializePrimitiveProperty(propertyType, property->Name.c_str(), value, serializer);

    // If we failed to load the property, there are a few extra things we need
    // to do
    if (mode == SerializerMode::Loading)
    {
      // Check if it has a rename
      if (!serialized)
      {
        if (MetaPropertyRename* rename = property->Has<MetaPropertyRename>())
          serialized = SerializePrimitiveProperty(propertyType, rename->mOldName.c_str(), value, serializer);
      }

      // If the value was not found use the default
      // value off of the property
      if (!serialized)
      {
        MetaSerializedProperty* metaDefault = property->Has<MetaSerializedProperty>();
        if (metaDefault)
        {
          // Even though we failed to read the property, someone set up a valid
          // default so set the property!
          value = metaDefault->mDefault;
          serialized = true;
        }
      }

      // We only set the property if we properly read a value, or we had a valid
      // default If neither is true, we actually just leave the property as it
      // was (assuming it is already initialized to its own default)
      if (serialized)
        property->SetValue(instance, value);
    }
  }
}

bool MetaSerialization::SerializePrimitiveProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer)
{
  Error("Type '%s' is not serializable on property '%s'", meta->ToString().c_str(), fieldName);
  return false;
}

bool MetaSerialization::SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer)
{
  Handle object = value.ToHandle();

  if (serializer.Start(propertyType, fieldName, StructureType::Object))
  {
    // Allocate the object if it's null, and we need to load in data
    if (object.IsNull() && serializer.GetMode() == SerializerMode::Loading)
    {
      if (propertyType->CreatableInScript && propertyType->GetDefaultConstructor() != nullptr)
      {
        object = RaverieAllocateUntyped(propertyType);

        // Re-assign the newly allocated object
        value = object;
      }
    }

    SerializeMembers(object, serializer);
    serializer.End(propertyType, StructureType::Object);
    return true;
  }

  return false;
}

void MetaSerialization::SerializeObject(AnyParam object, Serializer& serializer)
{
  Handle handle = object.ToHandle();
  ReturnIf(handle.IsNull(), , "Cannot serialize null object");

  if (Object* raverieObject = handle.Get<Object*>())
    raverieObject->Serialize(serializer);
  else
    MetaSerializeProperties(handle, serializer);
}

void MetaSerialization::SetDefault(Type* type, Any& any)
{
  any = Any(type);
}

String MetaSerialization::ConvertToString(AnyParam input)
{
  Error("Convert to string is required for serialization primitives. If a new "
        "type was added, this must be implemented for that primitive type.");
  return String();
}

bool MetaSerialization::ConvertFromString(StringParam input, Any& output)
{
  return false;
}

void MetaSerialization::SerializeMembers(HandleParam object, Serializer& serializer)
{
  MetaSerializeProperties(object, serializer);
}

// Serialization Primitive
RaverieDefineType(EnumMetaSerialization, builder, type)
{
}

EnumMetaSerialization::EnumMetaSerialization(BoundType* enumType) : mEnumType(enumType)
{
}

bool EnumMetaSerialization::SerializePrimitiveProperty(BoundType* type, cstr fieldName, Any& value, Serializer& serializer)
{
  Integer& enumValue = value.Get<Integer&>();
  return serializer.EnumField(type->Name.c_str(), fieldName, (uint&)enumValue, type);
}

void EnumMetaSerialization::SetDefault(Type* meta, Any& any)
{
  BoundType* type = Type::DebugOnlyDynamicCast<BoundType*>(meta);
  any = Any(type);
  Integer& value = any.Get<Integer&>();
  value = type->DefaultEnumValue;
}

String EnumMetaSerialization::ConvertToString(AnyParam input)
{
  Integer& value = input.Get<Integer&>();
  return mEnumType->EnumValueToStrings.FindValue(value, Array<String>(1)).Front();
}

bool EnumMetaSerialization::ConvertFromString(StringParam input, Any& output)
{
  Integer* foundEnumValue = mEnumType->StringToEnumValue.FindPointer(input);

  if (foundEnumValue)
  {
    output = Any((byte*)foundEnumValue, mEnumType);
    return true;
  }

  return false;
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Integer>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Integer2>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Integer3>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Integer4>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<String>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Boolean>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Real>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Real2>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Real3>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Real4>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Mat2>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Mat3>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Mat4>, builder, type)
{
}

RaverieDefineTemplateType(PrimitiveMetaSerialization<Quat>, builder, type)
{
}

// String Serialization
RaverieDefineType(MetaStringSerialization, builder, type)
{
}

bool MetaStringSerialization::SerializeReferenceProperty(BoundType* propertyType, cstr fieldName, Any& value, Serializer& serializer)
{
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    if (value.IsNotNull())
    {
      String localValue = value.Get<String>();
      return Serialization::Policy<String>::Serialize(serializer, fieldName, localValue);
    }
    return true;
  }
  else
  {
    String localValue;
    if (Serialization::Policy<String>::Serialize(serializer, fieldName, localValue))
    {
      value = localValue;
      return true;
    }

    return false;
  }
}

String MetaStringSerialization::ConvertToString(AnyParam input)
{
  return input.Get<String>();
}

bool MetaStringSerialization::ConvertFromString(StringParam input, Any& output)
{
  String value;
  ToValue(input.c_str(), value);
  output = value;
  return true;
}

// Serialization Filter
RaverieDefineType(SerializationFilter, builder, type)
{
}

bool SerializeAny(cstr fieldName, Any& value, Serializer& serializer)
{
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    BoundType* type = Type::GetBoundType(value.StoredType);
    ReturnIf(type == nullptr, false, "The Any that we attempted to serialize had no type");

    MetaSerialization* meta = type->Has<MetaSerialization>();
    ReturnIf(meta == nullptr,
             false,
             "The type '%s' being serialized does not have the "
             "MetaSerialization component",
             type->ToString().c_str());

    if (value.StoredType->IsValue())
      return meta->SerializePrimitiveProperty(type, fieldName, value, serializer);
    else
      return meta->SerializeReferenceProperty(type, fieldName, value, serializer);
  }
  else
  {
    ReturnIf(serializer.GetType() != SerializerType::Text, false, "Can only detect type from text");

    DataTreeLoader* dataTreeLoader = (DataTreeLoader*)&serializer;
    DataNode* node = dataTreeLoader->GetNext();

    if (node == nullptr)
      return false;

    String typeName = node->mTypeName.c_str();
    BoundType* type = MetaDatabase::GetInstance()->FindType(typeName);

    ReturnIf(type == nullptr, false, "Unknown type '%s' while serializing Any", node->mTypeName.c_str());

    MetaSerialization* meta = type->Has<MetaSerialization>();
    ReturnIf(meta == nullptr,
             false,
             "The type %s being serialized does not have the MetaSerialization "
             "component",
             value.StoredType->ToString().c_str());

    // Set up the any since SerializeAny expects it to initialized to a type
    meta->SetDefault(type, value);

    // Serialize the any
    if (value.StoredType->IsValue())
      return meta->SerializePrimitiveProperty(type, fieldName, value, serializer);
    else
      return meta->SerializeReferenceProperty(type, fieldName, value, serializer);
  }
}

bool SerializeVariant(cstr fieldName, Variant& value, Serializer& serializer)
{
  Any anyValue;

  // Loading value from stream?
  if (serializer.GetMode() == SerializerMode::Loading)
  {
    // Load the any value from stream
    if (!SerializeAny(fieldName, anyValue, serializer)) // Unable?
      return false;

    // Convert any value to variant value
    value = ConvertBasicAnyToVariant(anyValue);
    if (value.IsEmpty()) // Unable?
    {
      Error("The Variant being serialized (loading) was not a basic native "
            "type, this is currently unsupported.");
      return false;
    }
  }
  // Saving value to stream?
  else
  {
    // Convert variant value to any value
    anyValue = ConvertBasicVariantToAny(value);
    if (!anyValue.IsHoldingValue()) // Unable?
    {
      Error("The Variant being serialized (saving) was not a basic native "
            "type, this is currently unsupported.");
      return false;
    }

    // Save the any value to stream
    if (!SerializeAny(fieldName, anyValue, serializer)) // Unable?
      return false;
  }

  return true;
}

void SerializeProperty(HandleParam instance, Property* property, Serializer& serializer)
{
  BoundType* type = Type::GetBoundType(property->PropertyType);

  // This may be a non BoundType (for example, a DelegateType)
  // Note that a Function marked as [Property] never goes through this code path
  // because we only loop through Properties including Fields and GetterSetters,
  // which does not include Function (methods).
  if (type != nullptr)
  {
    MetaSerialization* meta = type->HasInherited<MetaSerialization>();

    if (meta)
    {
      meta->SerializeProperty(instance, property, serializer);
    }
    else
    {
      MetaSerialization defaultSerialization;
      defaultSerialization.SerializeProperty(instance, property, serializer);
    }
  }
}

void MetaSerializeComponents(HandleParam instance, Serializer& serializer)
{
  BoundType* type = instance.StoredType;
  ReturnIf(type == nullptr, , "Invalid instance given to MetaSerializeComponents");

  MetaComposition* composition = type->Has<MetaComposition>();

  // Not a composition type
  // Note: This should not be an error because we call it generically on any
  // object
  if (composition == nullptr)
    return;

  // Walk all components serialize each object
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    uint childCount = composition->GetComponentCount(instance);
    for (uint i = 0; i < childCount; ++i)
    {
      Any component = composition->GetComponentAt(instance, i);
      MetaSerialization* metaSerialization = component.StoredType->HasInherited<MetaSerialization>();
      if (metaSerialization)
        metaSerialization->SerializeObject(component, serializer);
      else
        Error("Unable to serialize component at index %d", i);
    }
  }
  else
  {
    PolymorphicNode componentNode;
    while (serializer.GetPolymorphic(componentNode))
    {
      BoundType* componentType = MetaDatabase::GetInstance()->FindType(componentNode.TypeName);
      if (componentType)
      {
        Handle component = composition->GetComponent(instance, componentType);
        if (component.IsNull())
        {
          component = RaverieAllocateUntyped(componentType);
          composition->AddComponent(instance, component);
        }

        if (component.IsNotNull())
        {
          MetaSerialization* metaSerialization = component.StoredType->Has<MetaSerialization>();
          if (metaSerialization)
            metaSerialization->SerializeObject(component, serializer);
          else
            Error("Unable to serialize component of type '%s'", componentType->ToString().c_str());
        }
      }
      else
      {
        Error("Unable to meta serialize component type '%s'", String(componentNode.TypeName).c_str());
      }
      serializer.EndPolymorphic();
    }
  }
}

void MetaSerializeProperties(HandleParam instance, Serializer& serializer)
{
  BoundType* boundType = instance.StoredType;
  // Go through all properties
  forRange (Property* property, boundType->GetProperties())
  {
    if (property->Get == nullptr || property->Set == nullptr)
      continue;

    if (property->HasAttribute(PropertyAttributes::cProperty) == nullptr && property->HasAttribute(PropertyAttributes::cSerialize) == nullptr &&
        property->HasAttribute(PropertyAttributes::cDeprecatedSerialized) == nullptr)
      continue;

    SerializeProperty(instance, property, serializer);
  }
}

void MetaSerializeObject(HandleParam instance, Serializer& serializer)
{
  BoundType* boundType = instance.StoredType;
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    serializer.StartPolymorphic(boundType);
    MetaSerializeProperties(instance, serializer);
    MetaSerializeComponents(instance, serializer);
    serializer.EndPolymorphic();
  }
  else
  {
    MetaSerializeProperties(instance, serializer);
    MetaSerializeComponents(instance, serializer);
  }
}

DataBlock SerializeObjectToDataBlock(Object* object)
{
  BinaryBufferSaver saver;
  object->Serialize(saver);
  return saver.ExtractAsDataBlock();
}

void SerializeObjectFromDataBlock(DataBlock& block, Object* object)
{
  BinaryBufferLoader loader;
  loader.SetBlock(block);
  object->Serialize(loader);
}

} // namespace Raverie
