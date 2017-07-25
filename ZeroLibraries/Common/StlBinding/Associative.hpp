///////////////////////////////////////////////////////////////////////////////
///
/// \file Associative.hpp
/// Binder for STL Map container.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include "Containers/ContainerCommon.hpp"
#include "Memory/ZeroAllocator.hpp"

namespace Zero
{

template<typename keytype,typename type>
inline void DeleteOp(std::Pair<const keytype,type>& entry ){delete entry.second;}

///StdMap is a generic map class.
///Used for storing a large amount of dynamic objects.
///Optimized for speed not size. Done using hashing.
///Examples: ResourceMap, Creator Maps, etc.
///Does not provide in order transversal.
template<typename keyType, typename dataType>
class StdHashMap : 
  public std::tr1::unordered_map<keyType, dataType, 
  std::tr1::hash<keyType>, 
  std::equal_to<keyType>, 
  Zallocator< std::Pair<const keyType, dataType> >  >
{
public:
  typedef std::tr1::unordered_map<keyType, dataType, std::tr1::hash<keyType>, std::equal_to<keyType>, Zallocator< std::Pair<const keyType, dataType> > > base_type;
  typedef IteratorRange<base_type> range;
  typedef typename base_type::iterator iterator;
  typedef StdHashMap <keyType, dataType> this_type;

  range All()
  {
    return range(Begin(), End());
  }

  range Find(const keyType& key)
  {
    iterator entry = base_type::Find(key);
    return range(entry, End());
  }

  void Insert(const keyType& key, const dataType& value)
  {
    base_type::Insert(std::MakePair(key,value));
  }

  value_type FindValue(const keyType& key)
  {
    iterator it = base_type::Find(key);
    if(it == base_type::End())
      return it.second;
    else
      return value_type();
  }
};

template<typename type>
class StdSet : public std::set< type >
{

};

}//namespace Zero
