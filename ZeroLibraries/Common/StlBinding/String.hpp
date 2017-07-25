///////////////////////////////////////////////////////////////////////////////
///
/// \file String.hpp
/// Declaration of the Binder for STL String container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Memory/ZeroAllocator.hpp"
#include "Containers/Hashing.hpp"

namespace Zero
{

class StdString : public std::basic_string<char, std::char_traits<char>, Zallocator<char> >
{
public:
  typedef std::basic_string<char, std::char_traits<char>, Zallocator<char> > base_type;
  StdString(cstr cstring)
    :base_type(cstring)
  {

  }
  
  StdString(Zero::StringRange r)
    :base_type(r.mBegin,r.mEnd)
  {
  }

  StdString(cstr begin, size_t size)
    :base_type(begin, size)
  {
  }

  StdString(const base_type& right)
    :base_type(right)
  {
  }

  StdString(const StdString& right)
    :base_type(right)
  {
  }


  StdString()
  {}

  static StdString BuildString(const StdString& p0, cstr p1)
  {
    return StdString(p0+p1);
  }
};

//Define a HashPolicy so the string class can be used with HashMap
template<>
struct HashPolicy<StdString> : public ComparePolicy<StdString>
{
  inline size_t operator()(const StdString& str ) const
  {
    return HashString(str.c_str(), str.size());
  }
};

}

//Define a tr1::hash so the string class can be used with std::tr1::hash_map
namespace std{
template<>
class hash<Zero::StdString>
{
public:
  inline size_t operator()(const Zero::StdString& str ) const
  {
    return Zero::HashString(str.c_str(), str.size());
  }
};
}

