
///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Serialization
{

template<>
struct Policy<IntVec2>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, IntVec2& value)
  {
    return stream.ArrayField("Integer2", fieldName, (byte*)&value, BasicArrayType::Integer, 2, sizeof(int));
  }
};

template<>
struct Policy<IntVec3>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, IntVec3& value)
  {
    return stream.ArrayField("Integer3", fieldName, (byte*)&value, BasicArrayType::Integer, 3, sizeof(int));
  }
};

template<>
struct Policy<IntVec4>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, IntVec4& value)
  {
    return stream.ArrayField("Integer4", fieldName, (byte*)&value, BasicArrayType::Integer, 4, sizeof(int));
  }
};

template<>
struct Policy<Vec2>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Vec2& value)
  {
    return stream.ArrayField("Real2", fieldName, (byte*)&value, BasicArrayType::Float, 2, sizeof(float));
  }
};

template<>
struct Policy<Vec3>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Vec3& value)
  {
    return stream.ArrayField("Real3", fieldName, (byte*)&value, BasicArrayType::Float, 3, sizeof(float));
  }
};

template<>
struct Policy<Vec4>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Vec4& value)
  {
    return stream.ArrayField("Real4", fieldName, (byte*)&value, BasicArrayType::Float, 4, sizeof(float));
  }
};

template<>
struct Policy<Quat>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Quat& value)
  {
    return stream.ArrayField("Quaternion", fieldName, (byte*)&value, BasicArrayType::Float, 4, sizeof(float));
  }
};

template<>
struct Policy<Mat3>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Mat3& value)
  {
    return stream.ArrayField("Real3x3", fieldName, (byte*)&value, BasicArrayType::Float, 9, sizeof(float));
  }
};

template<>
struct Policy<Mat4>
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, Mat4& value)
  {
    return stream.ArrayField("Real4x4", fieldName, (byte*)&value, BasicArrayType::Float, 16, sizeof(float));
  }
};

}//namespace Serialization

}//namespace Zero
