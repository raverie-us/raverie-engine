///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

// (See "BasicNativeTypes.inl" in Common for the complete list of basic native type declarations.)

// ----------------------------------------------------------------------------
// Available Basic Native Type Declaration Macros:
//
// DeclareBasicNativeType(T, Name)
//
// DeclareBasicNativeTypePrimitive(T, Name)
//
// DeclareBasicNativeTypeMultiPrimitive(T, Name, PrimitiveT, PrimitiveCount)
// ----------------------------------------------------------------------------

// Multi-Primitive Math Types
// ("Forward declared" by name in Common in order to be included as a named BasicNativeType::Enum value.
//  "Fully declared" with actual type information in Math in order to define the required type trait information via template specializations.)
DeclareBasicNativeTypeMultiPrimitive(Math::BoolVector2, BoolVector2, bool,   2) // aka Boolean2
DeclareBasicNativeTypeMultiPrimitive(Math::BoolVector3, BoolVector3, bool,   3) // aka Boolean3
DeclareBasicNativeTypeMultiPrimitive(Math::BoolVector4, BoolVector4, bool,   4) // aka Boolean4
DeclareBasicNativeTypeMultiPrimitive(Math::IntVector2,  IntVector2,  int,    2) // aka Integer2
DeclareBasicNativeTypeMultiPrimitive(Math::IntVector3,  IntVector3,  int,    3) // aka Integer3
DeclareBasicNativeTypeMultiPrimitive(Math::IntVector4,  IntVector4,  int,    4) // aka Integer4
DeclareBasicNativeTypeMultiPrimitive(Math::Vector2,     Vector2,     float,  2) // aka Real2
DeclareBasicNativeTypeMultiPrimitive(Math::Vector3,     Vector3,     float,  3) // aka Real3
DeclareBasicNativeTypeMultiPrimitive(Math::Vector4,     Vector4,     float,  4) // aka Real4
DeclareBasicNativeTypeMultiPrimitive(Math::Quaternion,  Quaternion,  float,  4) // aka Quaternion
DeclareBasicNativeTypeMultiPrimitive(Math::Matrix2,     Matrix2,     float,  4) // aka Real2x2
DeclareBasicNativeTypeMultiPrimitive(Math::Matrix3,     Matrix3,     float,  9) // aka Real3x3
DeclareBasicNativeTypeMultiPrimitive(Math::Matrix4,     Matrix4,     float, 16) // aka Real4x4

// ----------------------------------------------------------------------------------------------------------------------------------
// Note: If any 'Name' macro arguments change here, they must also be updated to match in the "BasicNativeTypes.inl" file in Common.
// ----------------------------------------------------------------------------------------------------------------------------------

// TODO: Move this to a more appropriate location!
/// Returns the native type object uniquely identified by the specified constant native type ID, else nullptr
inline NativeType* GetNativeTypeByConstantId(NativeTypeId nativeTypeId)
{
  // Switch on native type ID
  switch(nativeTypeId)
  {
  // Other Types
  default:
    return nullptr;

  // Bool Type
  case BasicNativeType::Bool:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Bool>::Type); // aka Boolean

  // Char Type
  case BasicNativeType::Char:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Char>::Type);

  // Fixed-Width Signed Integral Types
  case BasicNativeType::Int8:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int8>::Type);
  case BasicNativeType::Int16:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int16>::Type);
  case BasicNativeType::Int32:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int32>::Type); // aka Integer
  case BasicNativeType::Int64:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Int64>::Type); // aka DoubleInteger

  // Fixed-Width Unsigned Integral Types
  case BasicNativeType::Uint8:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint8>::Type); // aka Byte
  case BasicNativeType::Uint16:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint16>::Type);
  case BasicNativeType::Uint32:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint32>::Type);
  case BasicNativeType::Uint64:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Uint64>::Type);

  // Floating Point Types
  case BasicNativeType::Float:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Float>::Type); // aka Real
  case BasicNativeType::Double:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Double>::Type); // aka DoubleReal

  // Multi-Primitive Math Types
  case BasicNativeType::BoolVector2:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector2>::Type); // aka Boolean2
  case BasicNativeType::BoolVector3:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector3>::Type); // aka Boolean3
  case BasicNativeType::BoolVector4:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::BoolVector4>::Type); // aka Boolean4
  case BasicNativeType::IntVector2:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector2>::Type); // aka Integer2
  case BasicNativeType::IntVector3:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector3>::Type); // aka Integer3
  case BasicNativeType::IntVector4:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::IntVector4>::Type); // aka Integer4
  case BasicNativeType::Vector2:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector2>::Type); // aka Real2
  case BasicNativeType::Vector3:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector3>::Type); // aka Real3
  case BasicNativeType::Vector4:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Vector4>::Type); // aka Real4
  case BasicNativeType::Quaternion:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Quaternion>::Type); // aka Quaternion
  case BasicNativeType::Matrix2:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix2>::Type); // aka Real2x2
  case BasicNativeType::Matrix3:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix3>::Type); // aka Real3x3
  case BasicNativeType::Matrix4:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::Matrix4>::Type); // aka Real4x4

  // String Type
  case BasicNativeType::String:
    return NativeTypeOf(BasicNativeTypeFromEnum<BasicNativeType::String>::Type); // aka String
  }
}
