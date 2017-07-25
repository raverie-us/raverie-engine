///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------- Meta Serialization
//**************************************************************************************************
ZilchDefineType(MetaSerialization, builder, type)
{
}

//**************************************************************************************************
bool MetaSerialization::SerializePrimitiveProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer)
{
  Error("Type '%s' is not serializable on property '%s'", meta->ToString().c_str(), fieldName);
  return false;
}

//**************************************************************************************************
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
        object = ZilchAllocateUntyped(propertyType);

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

//**************************************************************************************************
void MetaSerialization::SerializeProperty(HandleParam instance, Property* property, Serializer& serializer)
{
  BoundType* propertyType = Type::GetBoundType(property->PropertyType);

  Any value;
  if(serializer.GetMode() == SerializerMode::Saving)
  {
    value = property->GetValue(instance);
  }
  else if(serializer.GetMode() == SerializerMode::Loading)
  {
    // If it's a reference type, attempt to get the already allocated object
    if (propertyType->CopyMode == TypeCopyMode::ReferenceType)
      value = property->GetValue(instance);
    else
      value.DefaultConstruct(propertyType);
  }

  // Checking to see if it's a serialization primitive or not is odd here. MetaSerialization
  // should be split up into MetaPrimitiveSerialization and MetaObjectSerialization
  // The String check is a hack until this change
  if (propertyType->CopyMode == TypeCopyMode::ReferenceType)
  {
    bool serialized = SerializeReferenceProperty(propertyType, property->Name.c_str(), value, serializer);

    // Set the object back in case we allocated it
    if(serialized)
      property->SetValue(instance, value);
  }
  else
  {
    bool serialized = SerializePrimitiveProperty(propertyType, property->Name.c_str(), value, serializer);

    // Check if it has a rename
    if (MetaPropertyRename* rename = property->Has<MetaPropertyRename>())
      serialized = SerializePrimitiveProperty(propertyType, rename->mOldName.c_str(), value, serializer);

    // If the value was not found use the default
    // value off of the property
    if (!serialized)
    {
      MetaSerializedProperty* metaDefault = property->Has<MetaSerializedProperty>();
      if (metaDefault)
      {
        // Even though we failed to read the property, someone set up a valid default so set the property!
        value = metaDefault->mDefault;
        serialized = true;
      }
    }

    // We only set the property if we properly read a value, or we had a valid default
    // If neither is true, we actually just leave the property as it was
    // (assuming it is already initialized to its own default)
    if (serialized)
      property->SetValue(instance, value);
  }
}

//**************************************************************************************************
void MetaSerialization::SerializeObject(AnyParam object, Serializer& serializer)
{
  Handle handle = object.ToHandle();
  ReturnIf(handle.IsNull(), , "Cannot serialize null object");

  if (Object* zeroObject = handle.Get<Object*>())
    zeroObject->Serialize(serializer);
  else
    MetaSerializeProperties(handle, serializer);
}

//**************************************************************************************************
void MetaSerialization::SetDefault(Type* type, Any& any)
{
  any = Any(type);
}

//**************************************************************************************************
bool MetaSerialization::ConvertFromString(StringParam input, Any& output)
{
  return false;
}

//**************************************************************************************************
void MetaSerialization::SerializeMembers(HandleParam object, Serializer& serializer)
{
  MetaSerializeProperties(object, serializer);
}

//--------------------------------------------------------------------- Meta Serialization Primitive
//**************************************************************************************************
ZilchDefineType(EnumMetaSerialization, builder, type)
{
}

//**************************************************************************************************
bool EnumMetaSerialization::SerializePrimitiveProperty(BoundType* meta, cstr fieldName, Any& value, Serializer& serializer)
{
  BoundType* type = Type::DebugOnlyDynamicCast<BoundType*>(meta);

  Integer& enumValue = value.Get<Integer&>();
  return serializer.EnumField(type->Name.c_str(), fieldName, (uint&)enumValue, type);
}

//**************************************************************************************************
void EnumMetaSerialization::SetDefault(Type* meta, Any& any)
{
  BoundType* type = Type::DebugOnlyDynamicCast<BoundType*>(meta);
  any = Any(type);
  Integer& value = any.Get<Integer&>();
  value = type->DefaultEnumValue;
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Integer>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<String>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Boolean>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Real>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Real2>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Real3>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Real4>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Mat3>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Mat4>, builder, type)
{
}

//**************************************************************************************************
ZilchDefineType(PrimitiveMetaSerialization<Quat>, builder, type)
{
}

//------------------------------------------------------------------------ Meta String Serialization
//**************************************************************************************************
ZilchDefineType(MetaStringSerialization, builder, type)
{
}

//**************************************************************************************************
bool MetaStringSerialization::SerializeReferenceProperty(BoundType* propertyType, cstr fieldName,
                                                         Any& value, Serializer& serializer)
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

//**************************************************************************************************
bool MetaStringSerialization::ConvertFromString(StringParam input, Any& output)
{
  String value;
  ToValue(input.c_str(), value);
  output = value;
  return true;
}

//----------------------------------------------------------------------------- Serialization Filter
//**************************************************************************************************
ZilchDefineType(SerializationFilter, builder, type)
{
}

//**************************************************************************************************
bool SerializeAny(cstr fieldName, Any& value, Serializer& serializer)
{
  if (serializer.GetMode() == SerializerMode::Saving)
  {
    BoundType* type = Type::GetBoundType(value.StoredType);
    ReturnIf(type == nullptr, false,
      "The Any that we attempted to serialize had no type");

    MetaSerialization* meta = type->Has<MetaSerialization>();
    ReturnIf(meta == nullptr, false,
      "The type '%s' being serialized does not have the MetaSerialization component",
      type->ToString().c_str());

    if(value.StoredType->IsValue())
      return meta->SerializePrimitiveProperty(type, fieldName, value, serializer);
    else
      return meta->SerializeReferenceProperty(type, fieldName, value, serializer);
  }
  else
  {
    ReturnIf(serializer.GetType() != SerializerType::Text, nullptr, "Can only detect type from text");

    DataTreeLoader* dataTreeLoader = (DataTreeLoader*)&serializer;
    DataNode* node = dataTreeLoader->GetNext();

    if (node == nullptr)
      return false;

    String typeName = node->mTypeName.c_str();
    BoundType* type = MetaDatabase::GetInstance()->FindType(typeName);

    ReturnIf(type == nullptr, false, "Unknown type '%s' while serializing Any", node->mTypeName.c_str());

    MetaSerialization* meta = type->Has<MetaSerialization>();
    ReturnIf(meta == nullptr, false,
      "The type %s being serialized does not have the MetaSerialization component",
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
  if(serializer.GetMode() == SerializerMode::Loading)
  {
    // Load the any value from stream
    if(!SerializeAny(fieldName, anyValue, serializer)) // Unable?
      return false;

    // Convert any value to variant value
    value = ConvertBasicAnyToVariant(anyValue);
    if(value.IsEmpty()) // Unable?
    {
      Error("The Variant being serialized (loading) was not a basic native type, this is currently unsupported.");
      return false;
    }
  }
  // Saving value to stream?
  else
  {
    // Convert variant value to any value
    anyValue = ConvertBasicVariantToAny(value);
    if(!anyValue.IsHoldingValue()) // Unable?
    {
      Error("The Variant being serialized (saving) was not a basic native type, this is currently unsupported.");
      return false;
    }

    // Save the any value to stream
    if(!SerializeAny(fieldName, anyValue, serializer)) // Unable?
      return false;
  }

  return true;
}

void SerializeProperty(HandleParam instance, Property* property, Serializer& serializer)
{
  BoundType* type = Type::GetBoundType(property->PropertyType);
  MetaSerialization* meta = type->HasInherited<MetaSerialization>();

  if(meta)
  {
    meta->SerializeProperty(instance, property, serializer);
  }
  else
  {
    MetaSerialization defaultSerialization;
    defaultSerialization.SerializeProperty(instance, property, serializer);
  }
}

void MetaSerializeComponents(HandleParam instance, Serializer& serializer)
{
  BoundType* type = instance.StoredType;
  ReturnIf(type == nullptr, , "Invalid instance given to MetaSerializeComponents");
  
  MetaComposition* composition = type->Has<MetaComposition>();

  // Not a composition type
  // Note: This should not be an error because we call it generically on any object
  if(composition == nullptr)
    return;

  // Walk all components serialize each object
  if(serializer.GetMode() == SerializerMode::Saving)
  {
    uint childCount = composition->GetComponentCount(instance);
    for(uint i = 0; i < childCount; ++i)
    {
      Any component = composition->GetComponentAt(instance, i);
      MetaSerialization* metaSerialization = component.StoredType->Has<MetaSerialization>();
      if (metaSerialization)
        metaSerialization->SerializeObject(component, serializer);
      else
        Error("Unable to serialize component at index %d", i);
    }
  }
  else
  {
    PolymorphicNode componentNode;
    while(serializer.GetPolymorphic(componentNode))
    {
      BoundType* componentType = MetaDatabase::GetInstance()->FindType(componentNode.TypeName);
      if(componentType)
      {
        Handle component = composition->GetComponent(instance, componentType);
        if(component.StoredType != nullptr)
        {
          MetaSerialization* metaSerialization = component.StoredType->Has<MetaSerialization>();
          if (metaSerialization)
            metaSerialization->SerializeObject(component, serializer);
          else
            Error("Unable to serialize component of type '%s'",
              componentType->ToString().c_str());
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
  forRange(Property* property, boundType->GetProperties())
  {
    if (property->Get == nullptr || property->Set == nullptr)
      continue;

    if(property->HasAttribute(Zilch::PropertyAttribute) == false &&
       property->HasAttribute(PropertyAttributes::cSerialized) == false)
      continue;

    SerializeProperty(instance, property, serializer);
  }
}

void MetaSerializeObject(HandleParam instance, Serializer& serializer)
{
  BoundType* boundType = instance.StoredType;
  if(serializer.GetMode() == SerializerMode::Saving)
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

}//namespace Zero
