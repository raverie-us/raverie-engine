// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
template <typename ToType, typename FromType>
class PointerCastHelper
{
public:
  static ToType DoCast(FromType from)
  {
    return from;
  }
};

template <typename ToType, typename FromType>
class PointerCastHelper<ToType*, FromType&>
{
public:
  static ToType* DoCast(FromType& from)
  {
    return &from;
  }
};

template <typename ToType, typename FromType>
class PointerCastHelper<ToType&, FromType*>
{
public:
  static ToType& DoCast(FromType* from)
  {
    return *from;
  }
};

template <typename ToType, typename FromType>
class PointerCastHelper<ToType, FromType*>
{
public:
  static ToType DoCast(FromType* from)
  {
    return *from;
  }
};

template <typename ToType, typename FromType>
class PointerCastHelper<ToType, FromType&>
{
public:
  static ToType DoCast(FromType& from)
  {
    return from;
  }
};

template <typename ToType, typename FromType>
ToType PointerCast(FromType from)
{
  return PointerCastHelper<ToType, FromType>::DoCast(from);
}

} // namespace Zero
