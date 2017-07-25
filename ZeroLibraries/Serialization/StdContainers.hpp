///////////////////////////////////////////////////////////////////////////////
///
/// \file StdContainers.hpp
/// Declaration of serialization polices for STL types so they can be
/// serialized by the serialization system.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <utility>

#include "Memory/Graph.hpp"
#include "Memory/ZeroAllocator.hpp"
#include "StlBinding/String.hpp"
#include "Serialization\SerializationTraits.hpp"
#include "Serialization\Serialization.hpp"


namespace Zero
{

typedef std::string StlString;

namespace Serialization
{

template<>
struct Trait<StlString>
{
  enum { Type = StructureType::Value };
  static inline cstr TypeName(){ return "string"; }
};

template<>
struct Policy<StlString>
{
  static inline bool Serialize(Serializer& serializer, cstr fieldName, StlString& stringValue)
  {
    if(serializer.GetMode() == SerializerMode::Saving)
    {
      cstr byteStart = stringValue.c_str();
      cstr byteEnd = stringValue.c_str() + stringValue.size();
      StringRange strData(byteStart, byteEnd);
      return serializer.StringField("string", fieldName, strData);
    }
    else
    {
      StringRange strData;
      if(serializer.StringField("string", fieldName, strData))
      {
        stringValue = StlString(strData.Data(), strData.SizeInBytes());
        return true;
      }
      return false;
    }
  }
};

template<typename type>
struct StdSerializeFunctor
{
  StdSerializeFunctor(Serializer& s)
    : stream(s) {}
  void operator()(const type& constValue)
  {
    type& value = const_cast<type&>(constValue);
    stream.SerializeValue(value);
  }
  Serializer& stream;
};

template<typename first, typename second>
struct Policy< std::pair<first, second> >
{
  typedef std::pair<first, second>  containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, std::pair<first, second>& pair)
  {
    serializer.Start(Trait<containertype>::TypeName(), fieldName, StructureType::Object);
    serializer.SerializeField("key", pair.first);
    serializer.SerializeField("value", pair.second);
    serializer.End(Trait<containertype>::TypeName(), StructureType::Object);
    return true;
  }
};

template<typename first, typename second>
struct Policy< std::pair<const first, second>  >
{
  typedef std::pair<const first, second>  containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName,  std::pair<const first,second>& pair)
  {
    serializer.Start(Trait<containertype>::TypeName(), fieldName, StructureType::Object);
    serializer.SerializeField("key", const_cast<first&>(pair.first));
    serializer.SerializeField("value", pair.second);
    serializer.End(Trait<containertype>::TypeName(), StructureType::Object);
    return true;
  }
};

template<typename containerType>
void StdSaveSequence(Serializer& serializer, containerType& container)
{
  uint containerSize = container.size();
  serializer.ArraySize(containerSize);
  std::for_each(container.begin(), container.end(), StdSerializeFunctor<typename containerType::value_type>(serializer));
}

template<typename containerType>
void StdLoadSequence(Serializer& serializer, containerType& container)
{
  uint containerSize = 0;
  serializer.ArraySize(containerSize);
  container.resize(containerSize);
  std::for_each(container.begin(), container.end(), StdSerializeFunctor<typename containerType::value_type>(serializer));
}

template<typename containerType>
void StdInsertSequence(Serializer& serializer, containerType& container)
{
  container.clear();
  uint containerSize = 0;
  serializer.ArraySize(containerSize);
  for(uint i=0;i<containerSize;++i)
  {
    typename containerType::value_type tempValue;
    serializer.SerializeValue(tempValue);
    container.insert(tempValue);
  }
}

template<typename type>
struct Policy< std::vector<type>  >
{
  typedef std::vector<type>  containertype;
  static bool Serialize(Serializer& stream, cstr fieldName,  containertype& container)
  {
    bool started = stream.Start("Array", fieldName, StructureType::Array);
    if(started)
    {
      if(stream.GetMode() == SerializerMode::Saving)
      {
        StdSaveSequence(stream, container);
      }
      else
      {
        StdLoadSequence(stream, container);
      }
      stream.End("Array", StructureType::Array);
    }
    return started;
  }
};

template<typename keytype, typename valuetype>
struct Policy< std::map<keytype, valuetype>  >
{
  typedef std::map<keytype, valuetype>  containertype;
  static bool Serialize(Serializer& stream, cstr fieldName,  containertype& container)
  {
    bool started = stream.Start("Map", fieldName, StructureType::Array);
    if(started)
    {
      if(stream.GetMode() == Zero::SerializerMode::Saving)
      {
        StdSaveSequence(stream, container);
      }
      else
      {
        StdInsertSequence(stream, container);
      }
      stream.End("Map", StructureType::Array);
    }
    return started;
  }
};

template<typename type>
struct Policy< std::set<type> >
{
  typedef std::set<type> containertype;
  static bool Serialize(Serializer& stream, cstr fieldName,  containertype& container)
  {
    bool started = stream.Start("Array", fieldName, StructureType::Array);
    if(started)
    {
      if(stream.GetMode() == Zero::SerializerMode::Saving)
      {
        StdSaveSequence(stream, container);
      }
      else
      {
        StdInsertSequence(stream, container);
      }
      stream.End("Array", StructureType::Array);
    }
    return started;
  }
};

}//namespace Serialization
}//namespace Zero
