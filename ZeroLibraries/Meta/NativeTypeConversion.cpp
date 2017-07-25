///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

NativeType* ZilchTypeToBasicNativeType(Zilch::Type* zilchType)
{
  // Empty type?
  if(zilchType == nullptr)
    return nullptr;

  // Boolean type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type);

  // Integer type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type);
  // DoubleInteger type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type);

  // Byte type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type);

  // Real type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type);
  // DoubleReal type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type);

  // Boolean2 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type);
  // Boolean3 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type);
  // Boolean4 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type);

  // Integer2 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type);
  // Integer3 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type);
  // Integer4 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type);

  // Real2 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type);
  // Real3 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type);
  // Real4 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type);

  // Quaternion type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type);

  // Real3x3 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type);
  // Real4x4 type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type);

  // String type?
  else if(zilchType == ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::String>::Type))
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::String>::Type);

  // Other type?
  else
    return nullptr;
}

Zilch::Type* BasicNativeTypeToZilchType(NativeType* nativeType)
{
  // Get native type ID
  NativeTypeId nativeTypeId = nativeType ? nativeType->mTypeId : cInvalidNativeTypeId;

  // Get zilch type via native type ID
  return BasicNativeTypeToZilchType(nativeTypeId);
}
Zilch::Type* BasicNativeTypeToZilchType(NativeTypeId nativeTypeId)
{
  // Switch on native type ID
  switch(nativeTypeId)
  {
  // Other Types
  default:
    return nullptr;

  // Bool Type
  case BasicNativeType::Bool:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type); // Boolean type

  // Char Type
  case BasicNativeType::Char:
    return nullptr; // (Not bound to Zilch)

  // Fixed-Width Signed Integral Types
  case BasicNativeType::Int8:
    return nullptr; // (Not bound to Zilch)
  case BasicNativeType::Int16:
    return nullptr; // (Not bound to Zilch)
  case BasicNativeType::Int32:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type); // Integer type
  case BasicNativeType::Int64:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type); // DoubleInteger type

  // Fixed-Width Unsigned Integral Types
  case BasicNativeType::Uint8:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type); // Byte type
  case BasicNativeType::Uint16:
    return nullptr; // (Not bound to Zilch)
  case BasicNativeType::Uint32:
    return nullptr; // (Not bound to Zilch)
  case BasicNativeType::Uint64:
    return nullptr; // (Not bound to Zilch)

  // Floating Point Types
  case BasicNativeType::Float:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type); // Real type
  case BasicNativeType::Double:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type); // DoubleReal type

  // Multi-Primitive Math Types
  case BasicNativeType::BoolVector2:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type); // Boolean2 type
  case BasicNativeType::BoolVector3:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type); // Boolean3 type
  case BasicNativeType::BoolVector4:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type); // Boolean4 type
  case BasicNativeType::IntVector2:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type); // Integer2 type
  case BasicNativeType::IntVector3:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type); // Integer3 type
  case BasicNativeType::IntVector4:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type); // Integer4 type
  case BasicNativeType::Vector2:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type); // Real2 type
  case BasicNativeType::Vector3:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type); // Real3 type
  case BasicNativeType::Vector4:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type); // Real4 type
  case BasicNativeType::Quaternion:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type); // Quaternion type
  case BasicNativeType::Matrix3:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type); // Real3x3 type
  case BasicNativeType::Matrix4:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type); // Real4x4 type

  // String Type
  case BasicNativeType::String:
    return ZilchTypeId(BasicNativeTypeFromEnum<BasicNativeType::String>::Type); // String type
  }
}

Variant ConvertBasicAnyToVariant(const Any& anyValue)
{
  // Any is empty?
  if(!anyValue.IsHoldingValue())
    return Variant();

  // Get basic native type from the any's stored type
  NativeType* nativeType = ZilchTypeToBasicNativeType(anyValue.StoredType);
  if(!nativeType) // Unable? (The any's stored type is not a basic native type?)
    return Variant();

  // Get any's stored value data
  const void* anyData = anyValue.GetData();
  Assert(anyData);

  // Copy any's stored value to new variant
  Variant result(nativeType, anyData);
  return result;
}
Any ConvertBasicVariantToAny(const Variant& variantValue)
{
  // Variant is empty?
  if(variantValue.IsEmpty())
    return Any();

  // Get zilch type from the variant's stored type
  Type* zilchType = BasicNativeTypeToZilchType(variantValue.GetNativeType());
  if(!zilchType) // Unable? (The variant's stored type is not a basic native type?)
    return Any();

  // Get variant's stored value data
  const void* variantData = variantValue.GetData();
  Assert(variantData);

  // Copy variant's stored value to new any
  Any result((const byte*)variantData, zilchType);
  return result;
}

} // namespace Zero
