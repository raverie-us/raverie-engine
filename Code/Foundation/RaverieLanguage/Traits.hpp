// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
template <typename T>
class StaticDereference
{
public:
  typedef T Type;
};

template <typename T>
class StaticDereference<T*>
{
public:
  typedef T Type;
};

// Tells us whether a type is a primitive (built in type)
template <typename T>
class IsPrimitive
{
public:
  static const bool Value = false;
  typedef void FalseType;
};

// Mark all the basic types that we know of as primtiive
template <>
class IsPrimitive<bool>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<float>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<double>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<char>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<signed char>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<unsigned char>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<signed short>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<unsigned short>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<signed int>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<unsigned int>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<signed long>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<unsigned long>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<signed long long>
{
public:
  static const bool Value = true;
};
template <>
class IsPrimitive<unsigned long long>
{
public:
  static const bool Value = true;
};
} // namespace Raverie
