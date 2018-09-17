///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ShaderAttribute
ShaderAttribute::ShaderAttribute(StringParam attributeName)
{
  mAttributeName = attributeName;
}

Zilch::AttributeParameter* ShaderAttribute::FindFirstParameter(StringParam name)
{
  for(size_t i = 0; i < mParameters.Size(); ++i)
  {
    if(mParameters[i].Name == name)
      return &mParameters[i];
  }
  return nullptr;
}

//-------------------------------------------------------------------ShaderAttributeList::NamedRange
ShaderAttributeList::NamedRange::NamedRange()
{
}

ShaderAttributeList::NamedRange::NamedRange(StringParam attributeToFind, Array<ShaderAttribute>::range range)
{
  mRange = range;
  mAttributeToFind = attributeToFind;
  SkipAttributes();
}

ShaderAttribute* ShaderAttributeList::NamedRange::Front()
{
  return &mRange.Front();
}

bool ShaderAttributeList::NamedRange::Empty() const
{
  return mRange.Empty();
}

void ShaderAttributeList::NamedRange::PopFront()
{
  mRange.PopFront();
  SkipAttributes();
}

void ShaderAttributeList::NamedRange::SkipAttributes()
{
  while(!mRange.Empty())
  {
    if(mRange.Front().mAttributeName == mAttributeToFind)
      break;
    mRange.PopFront();
  }
}

//-------------------------------------------------------------------ShaderAttributeList
ShaderAttribute* ShaderAttributeList::AddAttribute(StringParam attributeName, Zilch::AttributeNode* node)
{
  mAttributes.PushBack(attributeName);
  mNodes.PushBack(node);
  return &mAttributes.Back();
}

ShaderAttributeList::NamedRange ShaderAttributeList::FindAttributes(StringParam attributeName)
{
  return NamedRange(attributeName, mAttributes.All());
}

ShaderAttribute* ShaderAttributeList::FindFirstAttribute(StringParam attributeName)
{
  NamedRange range = FindAttributes(attributeName);
  if(!range.Empty())
    return range.Front();
  return nullptr;
}

ShaderAttributeList::range ShaderAttributeList::All()
{
  return mAttributes.All();
}

size_t ShaderAttributeList::Size()
{
  return mAttributes.Size();
}

ShaderAttribute* ShaderAttributeList::GetAtIndex(int index)
{
  return &mAttributes[index];
}

Zilch::CodeLocation* ShaderAttributeList::GetLocation(ShaderAttribute* shaderAttribute)
{
  int index = shaderAttribute - mAttributes.Data();
  if(index < 0 || index >= (int)mAttributes.Size())
    return nullptr;

  return &mNodes[index]->Location;
}

Zilch::CodeLocation* ShaderAttributeList::GetLocation(ShaderAttribute* shaderAttribute, Zilch::AttributeParameter* param)
{
  int attributeIndex = shaderAttribute - mAttributes.Data();
  if(attributeIndex < 0 || attributeIndex >= (int)mAttributes.Size())
    return nullptr;

  int paramIndex = param - shaderAttribute->mParameters.Data();
  if(paramIndex < 0 || paramIndex >= (int)shaderAttribute->mParameters.Size())
    return nullptr;


  Zilch::FunctionCallNode* callNode = mNodes[attributeIndex]->AttributeCall;
  return &(callNode->Arguments[paramIndex]->Location);
}

//-------------------------------------------------------------------ShaderIRAttributeParameter
ShaderIRAttributeParameter::ShaderIRAttributeParameter()
{
  mNode = nullptr;
}

ShaderIRAttributeParameter::ShaderIRAttributeParameter(Zilch::AttributeParameter& param, Zilch::SyntaxNode* node)
{
  mParameter = param;
  mNode = node;
}

String ShaderIRAttributeParameter::GetName() const
{
  return mParameter.Name;
}

void ShaderIRAttributeParameter::SetName(StringParam name)
{
  mParameter.Name = name;
}

Zilch::ConstantType::Enum ShaderIRAttributeParameter::GetType() const
{
  return mParameter.Type;
}

String ShaderIRAttributeParameter::GetStringValue() const
{
  return mParameter.StringValue;
}

void ShaderIRAttributeParameter::SetStringValue(StringParam stringValue)
{
  mParameter.StringValue = stringValue;
  mParameter.Type = Zilch::ConstantType::String;
}

int ShaderIRAttributeParameter::GetIntValue() const
{
  return (int)mParameter.IntegerValue;
}

void ShaderIRAttributeParameter::SetIntValue(int intValue)
{
  mParameter.IntegerValue = intValue;
  mParameter.Type = Zilch::ConstantType::Integer;
}

float ShaderIRAttributeParameter::GetFloatValue() const
{
  return (float)mParameter.RealValue;
}

void ShaderIRAttributeParameter::SetFloatValue(float floatValue)
{
  mParameter.RealValue = floatValue;
  mParameter.Type = Zilch::ConstantType::Real;
}

Zilch::Type* ShaderIRAttributeParameter::GetTypeValue() const
{
  return mParameter.TypeValue;
}

void ShaderIRAttributeParameter::SetTypeValue(Zilch::Type* typeValue)
{
  mParameter.TypeValue = typeValue;
  mParameter.Type = Zilch::ConstantType::Type;
}

Zilch::CodeLocation* ShaderIRAttributeParameter::GetLocation()
{
  if(mNode == nullptr)
    return nullptr;
  return &mNode->Location;
}

void ShaderIRAttributeParameter::SetLocationNode(Zilch::SyntaxNode* node)
{
  mNode = node;
}

Zilch::AttributeParameter& ShaderIRAttributeParameter::GetZilchAttributeParameter()
{
  return mParameter;
}

//-------------------------------------------------------------------ShaderIRAttribute
ShaderIRAttribute::ShaderIRAttribute()
{
  mNode = nullptr;
  mImplicitAttribute = false;
}

ShaderIRAttribute::ShaderIRAttribute(StringParam attributeName, Zilch::SyntaxNode* locationNode)
{
  mNode = locationNode;
  mAttributeName = attributeName;
  mImplicitAttribute = false;
}

ShaderIRAttributeParameter* ShaderIRAttribute::FindFirstParameter(StringParam name)
{
  for(size_t i = 0; i < mParameters.Size(); ++i)
  {
    if(mParameters[i].GetName() == name)
      return &mParameters[i];
  }
  return nullptr;
}

Zilch::CodeLocation* ShaderIRAttribute::GetLocation()
{
  if(mNode == nullptr)
    return nullptr;
  return &mNode->Location;
}

//-------------------------------------------------------------------ShaderIRAttributeList
ShaderIRAttributeList::NamedRange::NamedRange()
{

}

ShaderIRAttributeList::NamedRange::NamedRange(StringParam attributeToFind, const Range& range)
{
  mRange = range;
  mAttributeToFind = attributeToFind;
  SkipAttributes();
}

ShaderIRAttribute* ShaderIRAttributeList::NamedRange::Front()
{
  return &mRange.Front();
}

bool ShaderIRAttributeList::NamedRange::Empty() const
{
  return mRange.Empty();
}

void ShaderIRAttributeList::NamedRange::PopFront()
{
  mRange.PopFront();
  SkipAttributes();
}

void ShaderIRAttributeList::NamedRange::SkipAttributes()
{
  while(!mRange.Empty())
  {
    if(mRange.Front().mAttributeName == mAttributeToFind)
      break;
    mRange.PopFront();
  }
}

ShaderIRAttribute* ShaderIRAttributeList::AddAttribute(StringParam attributeName, Zilch::AttributeNode* node)
{
  ShaderIRAttribute* attribute = &mAttributes.PushBack();
  attribute->mAttributeName = attributeName;
  attribute->mNode = node;
  return attribute;
}

ShaderIRAttributeList::NamedRange ShaderIRAttributeList::FindAttributes(StringParam attributeName)
{
  return NamedRange(attributeName, mAttributes.All());
}

ShaderIRAttribute* ShaderIRAttributeList::FindFirstAttribute(StringParam attributeName)
{
  NamedRange range = FindAttributes(attributeName);
  if(!range.Empty())
    return range.Front();
  return nullptr;
}

ShaderIRAttributeList::Range ShaderIRAttributeList::All()
{
  return mAttributes.All();
}

size_t ShaderIRAttributeList::Size()
{
  return mAttributes.Size();
}

ShaderIRAttribute* ShaderIRAttributeList::GetAtIndex(int index)
{
  return &mAttributes[index];
}

ShaderIRAttribute* ShaderIRAttributeList::operator[](int index)
{
  return GetAtIndex(index);
}

}//namespace Zero

