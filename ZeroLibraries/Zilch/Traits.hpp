/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_TRAITS_HPP
#define ZILCH_TRAITS_HPP

namespace Zilch
{
  template <typename T>
  class ZeroSharedTemplate StaticDereference
  {
  public:
    typedef T Type;
  };

  template <typename T>
  class ZeroSharedTemplate StaticDereference<T*>
  {
  public:
    typedef T Type;
  };

  // Tells us whether a type is a primitive (built in type)
  template <typename T>
  class ZeroSharedTemplate IsPrimitive
  {
  public:
    static const bool Value = false;
    typedef void FalseType;
  };

  // Mark all the basic types that we know of as primtiive
  template <> class ZeroShared IsPrimitive<          bool     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<          float    > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<          double   > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<          char     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<signed    char     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<unsigned  char     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<signed    short    > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<unsigned  short    > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<signed    int      > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<unsigned  int      > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<signed    long     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<unsigned  long     > { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<signed    long long> { public: static const bool Value = true; };
  template <> class ZeroShared IsPrimitive<unsigned  long long> { public: static const bool Value = true; };
}

#endif
