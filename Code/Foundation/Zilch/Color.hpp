// MIT Licensed (see LICENSE.md).

#pragma once
#ifndef ZILCH_COLOR_HPP
#  define ZILCH_COLOR_HPP

namespace Zilch
{
class ColorClass
{
public:
  ZilchDeclareType(ColorClass, TypeCopyMode::ReferenceType);

  static Real4 FromBytes(Integer r, Integer g, Integer b);
  static Real4 FromBytes(Integer r, Integer g, Integer b, Integer a);
  static Real4 FromBytes(Integer4Param rgba);
  static Real4 FromInteger(Integer rgba);
  static Real4 FromInteger(Integer rgb, Integer a);
  static Real4 FromInteger(Integer rgb, Real a);
  static Real4 FromHsva(Real h, Real s, Real v);
  static Real4 FromHsva(Real h, Real s, Real v, Real a);
  static Real4 FromHsva(Real4Param hsva);
  static Real4 FromHexString(StringParam value);

  static Integer4 ToBytes(Real4Param rgba);
  static Integer4 ToBytes(Real r, Real g, Real b);
  static Integer4 ToBytes(Real r, Real g, Real b, Real a);
  static Integer ToInteger(Real4Param rgba);
  static Integer ToInteger(Real r, Real g, Real b);
  static Integer ToInteger(Real r, Real g, Real b, Real a);
  static Real4 ToHsva(Real4Param rgba);
  static Real4 ToHsva(Real r, Real g, Real b);
  static Real4 ToHsva(Real r, Real g, Real b, Real a);
  static String ToHexString(Real4Param rgba);
  static String ToHexString(Real r, Real g, Real b);
  static String ToHexString(Real r, Real g, Real b, Real a);
};

class ColorsClass
{
public:
  ZilchDeclareType(ColorsClass, TypeCopyMode::ReferenceType);

#  define DefineColor(name, r, g, b, a) static const Real4 name;
#  include "Math/ColorDefinitions.hpp"
#  undef DefineColor
};
} // namespace Zilch
#endif
