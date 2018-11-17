///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Mix in class used to implement serializers. Inherited serializes need to implement
/// InnerStart, InnertEnd and FundamentalType.
template<typename ConcreteType>
class SerializerBuilder : public Serializer
{
public:
  SerializerClass::Enum GetClass() override { return SerializerClass::SerializerBuilder; }
  using Serializer::Start;
  bool Start(cstr typeName, cstr fieldName, StructType structType) override
  {
    return Base()->InnerStart(typeName, fieldName, structType);
  }

  using Serializer::End;
  void End(cstr typeName, StructType structType) override
  {
    return Base()->InnerEnd(typeName, structType);
  }

  template<typename type>
  inline bool FundamentalFieldType(cstr fieldName, type& value)
  {
    if(Base()->InnerStart(Serialization::Trait<type>::TypeName(), fieldName, StructureType::Value))
    {
      Base()->FundamentalType(value);
      Base()->InnerEnd(Serialization::Trait<type>::TypeName(), StructureType::Value);
      return true;
    }
    return false;
  }

#define FUNDAMENTAL(type) bool FundamentalField(cstr fieldName, type& value) override { return FundamentalFieldType(fieldName, value); }
#include "FundamentalTypes.hpp"
#undef FUNDAMENTAL

  inline ConcreteType* Base(){return static_cast<ConcreteType*>(this);}
};

}
