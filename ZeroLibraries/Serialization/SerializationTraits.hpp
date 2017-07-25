///////////////////////////////////////////////////////////////////////////////
///
/// \file SerializationTraits.hpp
/// Type Traits for containers.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Serialization/Serialization.hpp"

namespace Zero
{

class Serializer;

namespace Serialization
{

template<typename type>
struct Trait
{
  enum {Type = StructureType::Object};
  static inline cstr TypeName(){ return "Object"; }
};

#define BasicSerializationType(type, typeName)        \
  template<> struct Trait<type>{                      \
  enum {Type = StructureType::Value };                \
  static inline cstr TypeName(){ return typeName;}};

BasicSerializationType(int,          "Integer");
BasicSerializationType(short,        "short");
BasicSerializationType(unsigned int, "uint");
BasicSerializationType(float,        "Real");
BasicSerializationType(double,       "double");
BasicSerializationType(s64,          "int64");
BasicSerializationType(u64,          "uint64");
BasicSerializationType(bool,         "bool");

#undef BasicSerializationType

template<typename type>
struct Policy
{
  static inline bool Serialize(Serializer& stream, cstr fieldName, type& value)
  {
    bool started = stream.Start(Trait<type>::TypeName(), fieldName, Trait<type>::Type);
    if(started)
    {
      value.Serialize(stream);
      stream.End(Trait<type>::TypeName(), Trait<type>::Type);
    }
    return started;
  }
};


// Serialization policy for pointers
template<typename type>
struct Policy<type*>
{
  // Serialize the connection
  static inline bool Serialize(Serializer& stream, cstr fieldName, type*& typePtr)
  {
    //Saving just serialize the object with policy
    if (stream.GetMode() == SerializerMode::Saving)
    {
      Policy<type>::Serialize(stream, fieldName, *typePtr);
    }
    else
    {
      //Loading create a instance of the object
      typePtr = new type();
      //and then serialize it with type's policy
      Policy<type>::Serialize(stream, fieldName, *typePtr);
    }
    return true;
  }
};

#define FUNDAMENTAL(type) template<> struct Policy<type> {                         \
  static inline bool Serialize(Serializer& stream, cstr fieldName, type& value){   \
  return stream.FundamentalField(fieldName, value); } };
#include "FundamentalTypes.hpp"
#undef FUNDAMENTAL

}//namespace Serialization

}//namespace Zero


#define DecalareTypeName(type, ctypename)                \
  namespace Serialization{                               \
   template<> struct Trait<type>{                       \
   enum { Type = StructureType::Object};                 \
   static inline cstr TypeName(){ return ctypename; }};}

