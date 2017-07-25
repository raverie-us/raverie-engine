///////////////////////////////////////////////////////////////////////////////
///
/// \file ByteColor.hpp
/// Declaration of the ByteColor used for debugging and basic color constants.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Math/Math.hpp"
#include "Utility/Typedefs.hpp"

typedef unsigned int ByteColor;

namespace Math
{
typedef Vector4 Vec4;
}

namespace CS
{

  //DirectX ARGB
  //const uint AlphaOffset = 24;
  //const uint AlphaMask = static_cast<uint>(0xff << AlphaOffset);

  //const uint RedOffset = 16;
  //const uint RedMask = static_cast<uint>(0xff << RedOffset);

  //const uint GreenOffset = 8;
  //const uint GreenMask = static_cast<uint>(0xff << GreenOffset);

  //const uint BlueOffset = 0;
  //const uint BlueMask = static_cast<uint>(0xff << BlueOffset);


  //OpenGl ABGR
  const uint MaxByte = 0xff;
  const uint AlphaOffset = 24;
  const uint AlphaMask = MaxByte << AlphaOffset;

  const uint BlueOffset = 16;
  const uint BlueMask = MaxByte << BlueOffset;

  const uint GreenOffset = 8;
  const uint GreenMask = MaxByte << GreenOffset;

  const uint RedOffset = 0;
  const uint RedMask = MaxByte << RedOffset;

  const float InvFactor = 1.0f / 255.0f;
}

#define ByteColorRGBA(r,g,b,a) \
  ((ByteColor)((((a)&CS::MaxByte)<<CS::AlphaOffset)|(((r)&CS::MaxByte)<<CS::RedOffset)|(((g)&CS::MaxByte)<<CS::GreenOffset)|((b)&CS::MaxByte)<<CS::BlueOffset))

#define FloatColorRGBA(r,g,b,a) \
  Math::Vec4(float(r) * CS::InvFactor, float(g) * CS::InvFactor, float(b) * CS::InvFactor, float(a) * CS::InvFactor)

inline Math::Vec4 ToFloatColor(ByteColor color)
{
  return Math::Vec4(
    float(((color)&CS::RedMask)>>CS::RedOffset)*CS::InvFactor,
    float(((color)&CS::GreenMask)>>CS::GreenOffset)*CS::InvFactor,
    float(((color)&CS::BlueMask)>>CS::BlueOffset)*CS::InvFactor,
    float(((color)&CS::AlphaMask)>>CS::AlphaOffset)*CS::InvFactor);
}

inline ByteColor ToByteColor(Math::Vec4 color)
{
  return ByteColorRGBA(byte(Math::Round(color.x * 255.f)),
    byte(Math::Round(color.y * 255.f)),
    byte(Math::Round(color.z * 255.f)),
    byte(Math::Round(color.w * 255.f)));
}

inline float GetHdrFromColor(Math::Vec4 color)
{
  float max = Math::Max(color.x, Math::Max(color.y, color.z));
  return Math::Max(max, 1.0f);
}

inline Math::Vec4 RemoveHdrFromColor(Math::Vec4 color)
{
  float hdr = GetHdrFromColor(color);
  color.x /= hdr;
  color.y /= hdr;
  color.z /= hdr;
  return color;
}

inline Math::Vec4 ToPastelColor(Math::Vec4 color)
{
  color *= 0.5f;
  color += Math::Vec4(0.5f, 0.5f, 0.5f, 0.0f);
  color.w = 1.0f;
  return color;
}

inline ByteColor ToPastelColor(ByteColor color)
{
  Math::Vec4 floatColor = ToFloatColor(color);
  floatColor = ToPastelColor(floatColor);
  return ToByteColor(floatColor);
}

inline ByteColor MultiplyByteColor(ByteColor color, float p)
{
  Math::Vec4 floatColor = ToFloatColor(color);
  floatColor*= p;
  return ToByteColor(floatColor);
}

inline ByteColor ColorWithAlpha(ByteColor color, float alpha)
{
  Math::Vec4 floatColor = ToFloatColor(color);
  floatColor.w = alpha;
  return ToByteColor(floatColor);
}

inline ByteColor ColorWithAlphaByte(ByteColor color, int alpha)
{
  ByteColor alphaRemoved = (CS::RedMask | CS::GreenMask | CS::BlueMask) & color;
  return alphaRemoved | (((alpha)&0xff) << CS::AlphaOffset);
}

inline void SetAlphaByte(ByteColor& color, uint alpha)
{
  ByteColor alphaRemoved = (CS::RedMask | CS::GreenMask | CS::BlueMask) & color;
  color = alphaRemoved | (((alpha)&0xff) << CS::AlphaOffset);
}

// Hue [0,1]
// Saturation [0,1]
// Value [0,1]
inline Math::Vec4 HSVToFloatColor(float H, float S, float V, float alpha = 1.0f)
{
  float chroma = S * V;
  float HPrime = H * 6.0f;
  float x = chroma * (1.0f - Math::Abs(Math::FMod(HPrime, 2.0f) - 1.0f));

  Math::Vec4 color;
  if(HPrime < 1)
    color = Math::Vec4(chroma, x, 0, 0);
  else if(HPrime < 2)
    color = Math::Vec4(x, chroma, 0, 0);
  else if(HPrime < 3)
    color = Math::Vec4(0, chroma, x, 0);
  else if(HPrime < 4)
    color = Math::Vec4(0, x, chroma, 0);
  else if(HPrime < 5)
    color = Math::Vec4(x, 0, chroma, 0);
  else if(HPrime <= 6)
    color = Math::Vec4(chroma, 0, x, 0);
  else
    color = Math::Vec4(0,0,0,0);

  float m = V - chroma;
  return color + Math::Vec4(m,m,m,alpha);
}

// Hue [0,1]
// Saturation [0,1]
// Value [0,1]
inline Math::Vec4 HSVToFloatColor(Math::Vec4Param hsv)
{
  return HSVToFloatColor(hsv.x, hsv.y, hsv.z, hsv.w);
}

inline Math::Vec4 FloatColorToHSV(float r, float g, float b, float alpha = 1.0f)
{
  // Calculate chroma
  float max = Math::Max(Math::Max(r, g), b);
  float min = Math::Min(Math::Min(r, g), b);
  float chroma = max - min;

  // Calculate Hue
  float hue;
  if(chroma == 0.0f)
  {
    hue = 0;
  }
  else
  {
    if(max == r)
      hue =(g - b) / chroma;
    else if(max == g)
      hue = ((b - r) / chroma) + 2.0f;
    else //max == b
      hue = ((r - g) / chroma) + 4.0f;
  }

  hue *= (60.0f / 360.0f);

  // Wrap the hue around
  if(hue < 0.0f)
    hue += 1.0f;

  // Calculate the value (or lightness)
  float value = max;

  // Calculate Saturation
  float saturation;
  if(chroma == 0.0f)
    saturation = 0.0f;
  else
    saturation = chroma / value;

  return Math::Vec4(hue, saturation, value, alpha);
}

inline Math::Vec4 FloatColorToHSV(Math::Vec4Param rgb)
{
  return FloatColorToHSV(rgb.x, rgb.y, rgb.z, rgb.w);
}

namespace EditorColor
{
  const ByteColor Red = ByteColorRGBA(128,  66,  66, 255);
  const ByteColor Green = ByteColorRGBA(56, 158,  56, 255);
  const ByteColor Blue = ByteColorRGBA(66,  66, 138, 255);
  const ByteColor Yellow = ByteColorRGBA(250, 204,   4, 255);

  const ByteColor NeutralGray= ByteColorRGBA(115, 115, 115, 255);

  const ByteColor Gray0 = ByteColorRGBA(128, 128, 128, 255);
  const ByteColor Gray1 = ByteColorRGBA(106, 106, 106, 255);
  const ByteColor Gray2 = ByteColorRGBA(92,  92,  92, 255);
  const ByteColor Gray3 = ByteColorRGBA(71,  71,  71, 255);

  const ByteColor TextBlack = ByteColorRGBA(0,   0,   0, 128);
}

namespace Color
{
#define DefineColor(name, r, g, b, a) const ByteColor name = ByteColorRGBA(r,g,b,a);
#include "ColorDefinitions.hpp"
#undef DefineColor
}
