///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// ugly template magic to have implicit character constructors and
// explicit integer constructor with compile time warnings to prevent
// implicit integer construction demoting to implicit character constructor
template <typename T>
struct IsChar
{
};

template <>
struct IsChar<char>
{
  typedef int Yes;
};

template <>
struct IsChar<unsigned char>
{
  typedef int Yes;
};

template <>
struct IsChar<signed char>
{
  typedef int Yes;
};

//------------------------------------------------------------------ Rune
class Rune;
typedef const Rune& RuneParam;

class Rune
{
public:
  static const size_t Invalid = 0xffffffff;

  Rune();
  explicit Rune(uint runeCode);

  template <typename T>
  Rune(T t, typename IsChar<T>::Yes = 0)
    : value((char)t)
  {
  };

  bool operator==(RuneParam rhs) const;
  bool operator!=(RuneParam rhs) const;
  bool operator>(RuneParam rhs) const;
  bool operator<(RuneParam rhs) const;
  bool operator>=(RuneParam rhs) const;
  bool operator<=(RuneParam rhs) const;

  bool operator==(uint rhs) const;
  bool operator!=(uint rhs) const;
  bool operator>(uint rhs) const;
  bool operator<(uint rhs) const;
  bool operator>=(uint rhs) const;
  bool operator<=(uint rhs) const;

  size_t Hash() const;

  uint value;
};

//------------------------------------------------------------------ Global Comparison Operators
inline bool operator==(uint left, RuneParam right)
{
  return left == right.value;
}

inline bool operator!=(uint left, RuneParam right)
{
  return left != right.value;
}

inline bool operator>(uint left, RuneParam right)
{
  return left > right.value;
}

inline bool operator<(uint left, RuneParam right)
{
  return left < right.value;
}

inline bool operator>=(uint left, RuneParam right)
{
  return left >= right.value;
}

inline bool operator<=(uint left, RuneParam right)
{
  return left <= right.value;
}

}// namespace Zero