// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

// Interpolation Policies
namespace Interpolation
{

template <typename type>
struct Lerp
{
  static inline type Interpolate(type& t0, type& t1, float t)
  {
    return type((1 - t) * t0 + t * t1);
  }
};

template <>
struct Lerp<Quat>
{
  static inline Quat Interpolate(Quat& t0, Quat& t1, float t)
  {
    return Quat::SlerpUnnormalized(t0, t1, t);
  }
};

template <>
struct Lerp<bool>
{
  static inline bool Interpolate(bool& t0, bool& t1, float t)
  {
    return t1;
  }
};

template <>
struct Lerp<String>
{
  static inline String Interpolate(String& t0, String& t1, float t)
  {
    if (t <= 0.0f)
      return t0;
    if (t >= 1.0f)
      return t1;
    // compute rune count is not the most efficient, might need optimization
    // later - Dane
    int t0Size = (int)t0.ComputeRuneCount();
    int t1Size = (int)t1.ComputeRuneCount();

    Rune startRune;
    Rune endRune;
    float runeFloatIndex;

    String* smallStr;
    String* bigStr;

    bool smallToLarge = t0Size <= t1Size;

    if (smallToLarge)
    {
      runeFloatIndex = t * t1Size;
      smallStr = &t0;
      bigStr = &t1;
    }
    else
    {
      runeFloatIndex = (1.0f - t) * t0Size;
      smallStr = &t1;
      bigStr = &t0;
    }

    int runeIndex = (int)runeFloatIndex;
    float leftoverT = runeFloatIndex - runeIndex;

    StringBuilder buffer;

    if (smallToLarge)
    {
      startRune = ' ';
      if (runeIndex < t0Size)
        // moving forward from the begin iterator is also not very fast, just
        // updating logic for UTF8
        startRune = *(t0.Begin() + runeIndex);
      endRune = *(t1.Begin() + runeIndex);
    }
    else
    {
      endRune = ' ';
      if (runeIndex < t1Size)
        endRune = *(t1.Begin() + runeIndex);
      startRune = *(t0.Begin() + runeIndex);
    }

    StringIterator it = bigStr->Begin();
    for (int i = 0; i < runeIndex; ++i)
    {
      buffer.Append(*it);
    }

    int lerpValue = (int)Math::Lerp((float)startRune.value, (float)endRune.value, leftoverT);
    buffer.Append(Rune(lerpValue));

    it = smallStr->Begin() + (runeIndex + 1);
    StringIterator smallEnd = smallStr->End();
    for (; it < smallEnd; ++it)
    {
      buffer.Append(*it);
    }

    return buffer.ToString();
  }
};

} // namespace Interpolation

// Ease Functions
namespace Ease
{
struct Linear
{
  static inline float In(float t)
  {
    return t;
  }

  static inline float Out(float t)
  {
    return t;
  }

  static inline float InOut(float t)
  {
    return t;
  }
};

struct Quad
{
  static inline float In(float t)
  {
    return t * t;
  }

  static inline float Out(float t)
  {
    return -(t * (t - 2));
  }

  static inline float InOut(float t)
  {
    if (t < 0.5f)
      return 2 * t * t;
    else
      return (-2 * t * t) + (4 * t) - 1;
  }
};

struct Sin
{
  static inline float In(float t)
  {
    return 1 - Math::Cos(t * Math::cPi / 2.0f);
  }

  static inline float Out(float t)
  {
    return Math::Sin(t * Math::cPi / 2.0f);
  }

  static inline float InOut(float t)
  {
    return -0.5f * (Math::Cos(t * Math::cPi) - 1);
  }
};

struct Elastic
{
  // Originally written by Robert Penner
  // http://robertpenner.com/easing/
  // Adapted by Doug Zwick

  static inline float In(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    float exponent = 10.0f;
    float period = 0.3f;

    float valueAtX = Math::Pow(2.0f, exponent * (t - 1.0f)) * Math::Cos(Math::cTwoPi * (t - 1.0f) / period);
    float verticalShift = -0.00048828125f;      // for default exponent and period
    float verticalScale = 1.0f - verticalShift; // 1.00048828125 for default exponent and period
    return (valueAtX - verticalShift) / verticalScale;
  }

  static inline float Out(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    float exponent = 10.0f;
    float period = 0.3f;
    float verticalScale = 1.00048828125f; // for default exponent and period
    return (1.0f - Math::Pow(2.0f, -exponent * t) * Math::Cos(Math::cTwoPi * t / period)) / verticalScale;
  }

  static inline float InOut(float t)
  {
    if (t < 0.5f)
    {
      t *= 2.0f;
      return In(t) / 2.0f;
    }
    else
    {
      t = 2.0f * t - 1.0f;
      return Out(t) / 2.0f + 0.5f;
    }
  }
};

struct Bounce
{
  // Originally written by Robert Penner
  // http://robertpenner.com/easing/
  // Adapted by Doug Zwick

  static inline float In(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    return 1.0f - Out(1.0f - t);
  }

  static inline float Out(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    if (t < 1.0f / 2.75f)
    {
      return 7.5625f * t * t;
    }
    if (t < 2.0f / 2.75f)
    {
      t -= 1.5f / 2.75f;
      return 7.5625f * t * t + 0.75f;
    }
    if (t < 2.5f / 2.75f)
    {
      t -= 2.25f / 2.75f;
      return 7.5625f * t * t + 0.9375f;
    }
    else
    {
      t -= 2.625f / 2.75f;
      return 7.5625f * t * t + 0.984375f;
    }
  }

  static inline float InOut(float t)
  {
    if (t < 0.5f)
    {
      t *= 2.0f;
      return In(t) / 2.0f;
    }
    else
    {
      t = 2.0f * t - 1.0f;
      return Out(t) / 2.0f + 0.5f;
    }
  }
};

struct Back
{
  // Originally written by Robert Penner
  // http://robertpenner.com/easing/
  // Adapted by Doug Zwick

  static inline float In(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    float scale = 1.70158; // original value defined by Penner
    return t * t * ((scale + 1.0f) * t - scale);
  }

  static inline float Out(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    float scale = 1.70158; // original value defined by Penner
    t -= 1.0f;
    return t * t * ((scale + 1.0f) * t + scale) + 1.0f;
  }

  static inline float InOut(float t)
  {
    if (t < 0.5f)
    {
      t *= 2.0f;
      return In(t) / 2.0f;
    }
    else
    {
      t = 2.0f * t - 1.0f;
      return Out(t) / 2.0f + 0.5f;
    }
  }
};

struct Warp
{
  static inline float In(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    if (t > 1.0f - MinDifferenceFromAsymptote)
      t = 1.0f - MinDifferenceFromAsymptote;

    return 1.0f - Math::Sqrt(1.0f / Math::Cos(Math::cPi * t / 2.0f));
  }

  static inline float Out(float t)
  {
    if (t <= 0.0f)
      return 0.0f;
    if (t >= 1.0f)
      return 1.0f;

    if (t < MinDifferenceFromAsymptote)
      t = MinDifferenceFromAsymptote;

    return Math::Sqrt(1.0f / Math::Cos(Math::cPi * (1.0f - t) / 2.0f));
  }

  static inline float InOut(float t)
  {
    if (t < 0.5f)
    {
      if (t > 0.5f - MinDifferenceFromAsymptote)
        t = 0.5f - MinDifferenceFromAsymptote;

      t *= 2.0f;
      return In(t) / 2.0f;
    }
    else
    {
      if (t < 0.5f + MinDifferenceFromAsymptote)
        t = 0.5f + MinDifferenceFromAsymptote;

      t = 2.0f * t - 1.0f;
      return Out(t) / 2.0f + 0.5f;
    }
  }

  static const float MinDifferenceFromAsymptote;
};

} // namespace Ease

} // namespace Zero
