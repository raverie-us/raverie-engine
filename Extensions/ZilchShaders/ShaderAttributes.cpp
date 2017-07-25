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

}//namespace Zero
