///////////////////////////////////////////////////////////////////////////////
///
/// \file DefaultSerializer.hpp
/// Declaration of the Default Serializer
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///A Serializer does not serialize any values. So all values
///will be defaulted.
class DefaultSerializer : public Serializer
{
public:
  DefaultSerializer()
  {
    mMode = SerializerMode::Loading;
    mSerializerType = SerializerType::Text;
  }
  SerializerClass::Enum GetClass() override { return SerializerClass::DefaultSerializer; }

  bool Start(cstr typeName, cstr fieldName, StructType structType) override 
  {
    return true;
  }

  void End(cstr typeName, StructType structType) override {}

  bool EnumField(cstr enumTypeName, cstr fieldName, uint& enumValue, BoundType* type) override
  {
    enumValue = (uint)type->DefaultEnumValue;
    return false;
  }

  void ArraySize(uint& arraySize) override
  {
    arraySize = 0;
  }

  bool GetPolymorphic(PolymorphicNode& node) override
  {
    return false;
  }

  void EndPolymorphic() override
  {
  }

  bool ArrayField(cstr, cstr, byte *, ArrayType arrayType, uint, uint) override
  {
    return false;
  }

  bool StringField(cstr typeName, cstr fieldName, StringRange& stringRange) override
  {
    return false;
  }

#define FUNDAMENTAL(type) bool FundamentalField(cstr fieldName, type& value) override{return false;};
#include "FundamentalTypes.hpp"
#undef FUNDAMENTAL

};

}
