// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zilch
{
ZilchDefineType(ColorClass, builder, type)
{
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::FromBytes,
                      ZilchStaticOverload(Real4, Integer, Integer, Integer),
                      "FromBytes",
                      "r, g, b")
      ->Description = ZilchDocumentString("Converts RGB Bytes [0-255] into an RGBA Real4 (alpha defaults to 1.0). "
                                          "Integer is used in place of Byte for convenience.");
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::FromBytes,
                      ZilchStaticOverload(Real4, Integer, Integer, Integer, Integer),
                      "FromBytes",
                      "r, g, b, a")
      ->Description = ZilchDocumentString("Converts RGBA Bytes [0-255] into an RGBA Real4. "
                                          "Integer is used in place of Byte for convenience.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::FromBytes, ZilchStaticOverload(Real4, Integer4Param), "FromBytes", "rgba")
      ->Description = ZilchDocumentString("Converts RGBA Bytes [0-255] into an RGBA Real4. "
                                          "Integer is used in place of Byte for convenience.");

  ZilchFullBindMethod(
      builder, type, &ColorClass::FromInteger, ZilchStaticOverload(Real4, Integer), "FromInteger", "rgba")
      ->Description = ZilchDocumentString("Converts an RGBA packed Integer into an RGBA Real4. Endianness is "
                                          "handled so that 0x00FF00FF always means full green and full alpha.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::FromInteger, ZilchStaticOverload(Real4, Integer, Integer), "FromInteger", "rgb, a")
      ->Description = ZilchDocumentString("Converts an RGB packed Integer and an alpha Byte [0-255] into an RGBA "
                                          "Real4. Endianness is handled so that 0x00FF00FF always means full green "
                                          "and full alpha.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::FromInteger, ZilchStaticOverload(Real4, Integer, Real), "FromInteger", "rgb, a")
      ->Description = ZilchDocumentString("Converts an RGB packed Integer and an alpha Real "
                                          "into an RGBA Real4. Endianness is handled so that "
                                          "0x00FF00FF always means full green and full alpha.");

  ZilchFullBindMethod(
      builder, type, &ColorClass::FromHsva, ZilchStaticOverload(Real4, Real, Real, Real), "FromHsva", "h, s, v")
      ->Description = ZilchDocumentString("Converts HSV Reals into an RGBA Real4 (alpha defaults to 1.0). The hue "
                                          "parameter will wrap if is beyond [0-1]. The saturation and value "
                                          "parameters may go beyond [0-1] to represent HDR values.");
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::FromHsva,
                      ZilchStaticOverload(Real4, Real, Real, Real, Real),
                      "FromHsva",
                      "h, s, v, a")
      ->Description = ZilchDocumentString("Converts HSVA Reals into an RGBA Real4. The hue parameter will wrap if "
                                          "is beyond [0-1]. The saturation and value parameters may go beyond "
                                          "[0-1] to represent HDR values.");
  ZilchFullBindMethod(builder, type, &ColorClass::FromHsva, ZilchStaticOverload(Real4, Real4Param), "FromHsva", "hsva")
      ->Description = ZilchDocumentString("Converts an HSVA Real4 into an RGBA Real4. The hue parameter will wrap "
                                          "if is beyond [0-1]. The saturation and value parameters may go beyond "
                                          "[0-1] to represent HDR values.");

  ZilchFullBindMethod(builder, type, &ColorClass::FromHexString, ZilchNoOverload, "FromHexString", "value")
      ->Description = ZilchDocumentString("Converts a hex String into an RGBA Real4. Must be a 3, 4, 6, or 8 digit "
                                          "RGB[A] representation with an optional preceding '#' or '0x' (case "
                                          "insensitive). E.g. #f00, #F00F, ff0000, 0x00FF00FF.");

  ZilchFullBindMethod(builder, type, &ColorClass::ToBytes, ZilchStaticOverload(Integer4, Real4Param), "ToBytes", "rgba")
      ->Description = ZilchDocumentString("Converts an RGBA Real4 into an RGBA Integer4 [0-255]. Integer is used "
                                          "in place of Byte for convenience.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::ToBytes, ZilchStaticOverload(Integer4, Real, Real, Real), "ToBytes", "r, g, b")
      ->Description = ZilchDocumentString("Converts RGB Reals into an RGBA Integer4 [0-255] (alpha defaults to "
                                          "255). Integer is used in place of Byte for convenience.");
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::ToBytes,
                      ZilchStaticOverload(Integer4, Real, Real, Real, Real),
                      "ToBytes",
                      "r, g, b, a")
      ->Description = ZilchDocumentString("Converts RGBA Reals into an RGBA Integer4 [0-255]. "
                                          "Integer is used in place of Byte for convenience.");

  ZilchFullBindMethod(
      builder, type, &ColorClass::ToInteger, ZilchStaticOverload(Integer, Real4Param), "ToInteger", "rgba")
      ->Description = ZilchDocumentString("Converts an RGBA Real4 into an RGBA packed Integer. Endianness is "
                                          "handled so that 0x00FF00FF always means full green and full alpha.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::ToInteger, ZilchStaticOverload(Integer, Real, Real, Real), "ToInteger", "r, g, b")
      ->Description = ZilchDocumentString("Converts RGB Reals into an RGBA packed Integer (alpha defaults to 255). "
                                          "Endianness is handled so that 0x00FF00FF always means full green and "
                                          "full alpha.");
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::ToInteger,
                      ZilchStaticOverload(Integer, Real, Real, Real, Real),
                      "ToInteger",
                      "r, g, b, a")
      ->Description = ZilchDocumentString("Converts RGBA Reals into an RGBA packed Integer. Endianness is handled "
                                          "so that 0x00FF00FF always means full green and full alpha.");

  ZilchFullBindMethod(builder, type, &ColorClass::ToHsva, ZilchStaticOverload(Real4, Real4Param), "ToHsva", "rgba")
      ->Description = ZilchDocumentString("Converts an RGBA Real4 into an HSVA Real4.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::ToHsva, ZilchStaticOverload(Real4, Real, Real, Real), "ToHsva", "r, g, b")
      ->Description = ZilchDocumentString("Converts RGB Reals into an HSVA Real4 (alpha defaults to 1.0).");
  ZilchFullBindMethod(
      builder, type, &ColorClass::ToHsva, ZilchStaticOverload(Real4, Real, Real, Real, Real), "ToHsva", "r, g, b, a")
      ->Description = ZilchDocumentString("Converts RGBA Reals into an HSVA Real4.");

  ZilchFullBindMethod(
      builder, type, &ColorClass::ToHexString, ZilchStaticOverload(String, Real4Param), "ToHexString", "rgba")
      ->Description = ZilchDocumentString("Converts an RGBA Real4 into the 8 digit hex format RRGGBBAA.");
  ZilchFullBindMethod(
      builder, type, &ColorClass::ToHexString, ZilchStaticOverload(String, Real, Real, Real), "ToHexString", "r, g, b")
      ->Description = ZilchDocumentString("Converts RGB Reals into the 8 digit hex format RRGGBBAA (alpha defaults "
                                          "to 1.0 so the end will always be FF).");
  ZilchFullBindMethod(builder,
                      type,
                      &ColorClass::ToHexString,
                      ZilchStaticOverload(String, Real, Real, Real, Real),
                      "ToHexString",
                      "r, g, b, a")
      ->Description = ZilchDocumentString("Converts RGBA Reals into the 8 digit hex format RRGGBBAA.");
}

// We need to ensure that we actually use an RGBA format where R is the highest
// byte
namespace CC
{
const uint MaxByte = 0xff;

const uint RedOffset = 24;
const uint RedMask = MaxByte << RedOffset;

const uint GreenOffset = 16;
const uint GreenMask = MaxByte << GreenOffset;

const uint BlueOffset = 8;
const uint BlueMask = MaxByte << BlueOffset;

const uint AlphaOffset = 0;
const uint AlphaMask = MaxByte << AlphaOffset;

const float InvFactor = 1.0f / 255.0f;
} // namespace CC

#undef ByteColorRGBA
#define ByteColorRGBA(r, g, b, a)                                                                                      \
  ((ByteColor)((((a)&CC::MaxByte) << CC::AlphaOffset) | (((r)&CC::MaxByte) << CC::RedOffset) |                         \
               (((g)&CC::MaxByte) << CC::GreenOffset) | ((b)&CC::MaxByte) << CC::BlueOffset))

#undef FloatColorRGBA
#define FloatColorRGBA(r, g, b, a)                                                                                     \
  Math::Vec4(float(r) * CC::InvFactor, float(g) * CC::InvFactor, float(b) * CC::InvFactor, float(a) * CC::InvFactor)

inline Math::Vec4 ToFloatColor(ByteColor color)
{
  return Math::Vec4(float(((color)&CC::RedMask) >> CC::RedOffset) * CC::InvFactor,
                    float(((color)&CC::GreenMask) >> CC::GreenOffset) * CC::InvFactor,
                    float(((color)&CC::BlueMask) >> CC::BlueOffset) * CC::InvFactor,
                    float(((color)&CC::AlphaMask) >> CC::AlphaOffset) * CC::InvFactor);
}

inline ByteColor ToByteColor(Math::Vec4 color)
{
  return ByteColorRGBA(byte(Math::Round(color.x * 255.f)),
                       byte(Math::Round(color.y * 255.f)),
                       byte(Math::Round(color.z * 255.f)),
                       byte(Math::Round(color.w * 255.f)));
}

inline ByteColor ColorWithAlpha(ByteColor color, float alpha)
{
  Math::Vec4 floatColor = ToFloatColor(color);
  floatColor.w = alpha;
  return ToByteColor(floatColor);
}

inline ByteColor ColorWithAlphaByte(ByteColor color, int alpha)
{
  ByteColor alphaRemoved = (CC::RedMask | CC::GreenMask | CC::BlueMask) & color;
  return alphaRemoved | (((alpha)&0xff) << CC::AlphaOffset);
}

Real4 ColorClass::FromBytes(Integer r, Integer g, Integer b)
{
  return FromBytes(r, g, b, 255);
}

Real4 ColorClass::FromBytes(Integer r, Integer g, Integer b, Integer a)
{
  r = Math::Clamp(r, 0, 255);
  g = Math::Clamp(g, 0, 255);
  b = Math::Clamp(b, 0, 255);
  a = Math::Clamp(a, 0, 255);
  return FloatColorRGBA(r, g, b, a);
}

Real4 ColorClass::FromBytes(Integer4Param rgba)
{
  return FromBytes(rgba.x, rgba.y, rgba.z, rgba.w);
}

Real4 ColorClass::FromInteger(Integer rgba)
{
  return ToFloatColor(rgba);
}

Real4 ColorClass::FromInteger(Integer rgb, Integer a)
{
  return ToFloatColor(ColorWithAlphaByte(rgb, Math::Clamp(a, 0, 255)));
}

Real4 ColorClass::FromInteger(Integer rgb, Real a)
{
  return ToFloatColor(ColorWithAlpha(rgb, a));
}

Real4 ColorClass::FromHsva(Real h, Real s, Real v)
{
  return FromHsva(h, s, v, 1.0f);
}

Real4 ColorClass::FromHsva(Real h, Real s, Real v, Real a)
{
  h = Math::FMod(h, 1.0f);
  if (h < 0)
    h += 1.0f;

  return HSVToFloatColor(h, s, v, a);
}

Real4 ColorClass::FromHsva(Real4Param hsva)
{
  return FromHsva(hsva.x, hsva.y, hsva.z, hsva.w);
}

int DetermineHexValue(char r)
{
  if (r >= '0' && r <= '9')
    return r - '0';
  else if (r >= 'a' && r <= 'f')
    return r - 'a' + 10;
  else if (r >= 'A' && r <= 'F')
    return r - 'A' + 10;

  ExecutableState::CallingState->ThrowException("Invalid character in hex color string.");
  return 0;
}

Real4 ColorClass::FromHexString(StringParam value)
{
  cstr begin = value.Data();
  cstr end = value.Data() + value.SizeInBytes();

  const char* sizeError = "The hex string must be a 3, 4, 6, or 8 digit RGB[A] representation with "
                          "an optional preceding '#' or '0x' (case insensitive). E.g. #f00, #F00F, "
                          "ff0000, 0x00FF00FF.";
  if (value.SizeInBytes() < 2)
  {
    ExecutableState::CallingState->ThrowException(sizeError);
    return Real4::cZero;
  }

  bool is0xRepresentation = false;

  // Skip the leading 0x, 0X, or # characters if they exist
  if (begin[0] == '0' && (begin[1] == 'x' || begin[1] == 'X'))
  {
    begin += 2;
    is0xRepresentation = true;
  }
  else if (begin[0] == '#')
  {
    begin += 1;
  }

  size_t length = end - begin;

  if (is0xRepresentation && length != 8)
  {
    ExecutableState::CallingState->ThrowException("A string that starts with '0x' must be the 8 digit RGBA "
                                                  "representation. E.g. 0xFF000000.");
    return Real4::cZero;
  }

  if (length != 3 && length != 4 && length != 6 && length != 8)
  {
    ExecutableState::CallingState->ThrowException(sizeError);
    return Real4::cZero;
  }

  if (length == 3 || length == 4)
  {
    int r = DetermineHexValue(begin[0]);
    int g = DetermineHexValue(begin[1]);
    int b = DetermineHexValue(begin[2]);
    int a = 0xF;

    if (length == 4)
      a = DetermineHexValue(begin[3]);

    const float inverseFactor = 1.0f / 15.0f;

    return Real4(r * inverseFactor, g * inverseFactor, b * inverseFactor, a * inverseFactor);
  }

  // Otherwise, this is a 6 or 8 byte representation
  // For 8 byte representations there is no offset
  int offset = 0;

  // For 6 byte representations we need to offset the integer value by 2 places
  u32 result = 0;
  if (length == 6)
  {
    // Make sure we use full alpha
    result = 0xFF;
    offset = 2;
  }

  for (int i = offset; begin != end; --end, ++i)
  {
    // Process the string in reverse
    char r = end[-1];
    u32 value = DetermineHexValue(r);
    result += value << i * 4;
  }

  return ToFloatColor(result);
}

Integer4 ColorClass::ToBytes(Real4Param rgba)
{
  Real4 rgbaClamped = Math::Clamp(rgba, Real4(0.0f), Real4(1.0f));

  return Integer4(int(Math::Round(rgbaClamped.x * 255.0f)),
                  int(Math::Round(rgbaClamped.y * 255.0f)),
                  int(Math::Round(rgbaClamped.z * 255.0f)),
                  int(Math::Round(rgbaClamped.w * 255.0f)));
}

Integer4 ColorClass::ToBytes(Real r, Real g, Real b)
{
  return ToBytes(Real4(r, g, b, 1.0f));
}

Integer4 ColorClass::ToBytes(Real r, Real g, Real b, Real a)
{
  return ToBytes(Real4(r, g, b, a));
}

Integer ColorClass::ToInteger(Real4Param rgba)
{
  return ToByteColor(Math::Clamp(rgba, Real4(0.0f), Real4(1.0f)));
}

Integer ColorClass::ToInteger(Real r, Real g, Real b)
{
  return ToInteger(Real4(r, g, b, 1.0f));
}

Integer ColorClass::ToInteger(Real r, Real g, Real b, Real a)
{
  return ToInteger(Real4(r, g, b, a));
}

Real4 ColorClass::ToHsva(Real4Param rgba)
{
  return FloatColorToHSV(rgba);
}

Real4 ColorClass::ToHsva(Real r, Real g, Real b)
{
  return ToHsva(Real4(r, g, b, 1.0f));
}

Real4 ColorClass::ToHsva(Real r, Real g, Real b, Real a)
{
  return ToHsva(Real4(r, g, b, a));
}

String ColorClass::ToHexString(Real4Param rgba)
{
  ByteColor color = ToByteColor(Math::Clamp(rgba, Real4(0.0f), Real4(1.0f)));
  return String::Format("%08X", color);
}

String ColorClass::ToHexString(Real r, Real g, Real b)
{
  return ToHexString(Real4(r, g, b, 1.0f));
}

String ColorClass::ToHexString(Real r, Real g, Real b, Real a)
{
  return ToHexString(Real4(r, g, b, a));
}

ZilchDefineType(ColorsClass, builder, type)
{
  // Bind all the color definitions as getters
  const char* description = "RGBA color of Real4(%g, %g, %g, %g) or hex 0x%02X%02X%02X%02X";
#define DefineColor(name, r, g, b, a)                                                                                  \
  {                                                                                                                    \
    Zilch::Property* prop = ZilchBindFieldGetter(name);                                                                \
    prop->ComplexUserData.WriteObject(name, 0);                                                                        \
    prop->Description = ZilchDocumentString(String::Format(                                                            \
        description, r * CC::InvFactor, g * CC::InvFactor, b * CC::InvFactor, a * CC::InvFactor, r, g, b, a));         \
  }
#include "Math/ColorDefinitions.hpp"
#undef DefineColor
}

#define DefineColor(name, r, g, b, a) const Real4 ColorsClass::name = FloatColorRGBA(r, g, b, a);
#include "Math/ColorDefinitions.hpp"
#undef DefineColor
} // namespace Zilch
