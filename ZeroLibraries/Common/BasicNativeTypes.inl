///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

// (See "NativeType.hpp" for more information about basic native type declarations.)

// ----------------------------------------------------------------------------
// Available Basic Native Type Declaration Macros:
//
// ForwardDeclareBasicNativeType(Name)
//
// DeclareBasicNativeType(T, Name)
//
// DeclareBasicNativeTypePrimitive(T, Name)
//
// DeclareBasicNativeTypeMultiPrimitive(T, Name, PrimitiveT, PrimitiveCount)
// ----------------------------------------------------------------------------

// Bool Type
DeclareBasicNativeTypePrimitive(bool, Bool) // aka Boolean

// Char Type
// (C++ mandates 'char' is a distinct type. It is not a typedef for 'signed char' nor 'unsigned char'.)
DeclareBasicNativeTypePrimitive(char, Char)

// Fixed-Width Signed Integral Types
DeclareBasicNativeTypePrimitive(int8,  Int8)
DeclareBasicNativeTypePrimitive(int16, Int16)
DeclareBasicNativeTypePrimitive(int32, Int32) // aka Integer
DeclareBasicNativeTypePrimitive(int64, Int64) // aka DoubleInteger

// Fixed-Width Unsigned Integral Types
DeclareBasicNativeTypePrimitive(uint8,  Uint8) // aka Byte
DeclareBasicNativeTypePrimitive(uint16, Uint16)
DeclareBasicNativeTypePrimitive(uint32, Uint32)
DeclareBasicNativeTypePrimitive(uint64, Uint64)

// Floating Point Types
DeclareBasicNativeTypePrimitive(float,  Float)  // aka Real
DeclareBasicNativeTypePrimitive(double, Double) // aka DoubleReal

// Multi-Primitive Math Types
// ("Forward declared" by name in Common in order to be included as a named BasicNativeType::Enum value.
//  "Fully declared" with actual type information in Math in order to define the required type trait information via template specializations.)
ForwardDeclareBasicNativeType(BoolVector2) // aka Boolean2
ForwardDeclareBasicNativeType(BoolVector3) // aka Boolean3
ForwardDeclareBasicNativeType(BoolVector4) // aka Boolean4
ForwardDeclareBasicNativeType(IntVector2)  // aka Integer2
ForwardDeclareBasicNativeType(IntVector3)  // aka Integer3
ForwardDeclareBasicNativeType(IntVector4)  // aka Integer4
ForwardDeclareBasicNativeType(Vector2)     // aka Real2
ForwardDeclareBasicNativeType(Vector3)     // aka Real3
ForwardDeclareBasicNativeType(Vector4)     // aka Real4
ForwardDeclareBasicNativeType(Quaternion)  // aka Quaternion
ForwardDeclareBasicNativeType(Matrix2)     // aka Real2x2
ForwardDeclareBasicNativeType(Matrix3)     // aka Real3x3
ForwardDeclareBasicNativeType(Matrix4)     // aka Real4x4

// String Type
// (Must always be the last declaration in this file!)
DeclareBasicNativeType(String, String) // aka String

// ----------------------------------------------------------------------------------------------------------------------------------
// Note: String is assumed to be the last declaration in this file, we rely on this to calculate enum size in BasicNativeType::Size.
// ----------------------------------------------------------------------------------------------------------------------------------
