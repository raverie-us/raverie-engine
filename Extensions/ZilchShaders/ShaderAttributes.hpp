///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------ShaderAttribute
// Attribute information parsed from zilch. Currently just contains the
// attribute name, will be extended later for parameters.
class ShaderAttribute
{
public:
  ShaderAttribute() {};
  ShaderAttribute(StringParam attributeName);

  Zilch::AttributeParameter* FindFirstParameter(StringParam name);

  String mAttributeName;
  Array<Zilch::AttributeParameter> mParameters;
};

//-------------------------------------------------------------------ShaderAttributeList
class ShaderAttributeList
{
public:

  class NamedRange
  {
  public:
    NamedRange();
    NamedRange(StringParam attributeToFind, Array<ShaderAttribute>::range range);

    ShaderAttribute* Front();
    bool Empty() const;
    void PopFront();

  private:
    void SkipAttributes();

    String mAttributeToFind;
    Array<ShaderAttribute>::range mRange;
  };

  typedef Array<ShaderAttribute>::range range;

  ShaderAttribute* AddAttribute(StringParam attributeName, Zilch::AttributeNode* node);
  NamedRange FindAttributes(StringParam attributeName);
  ShaderAttribute* FindFirstAttribute(StringParam attributeName);
  range All();
  size_t Size();

  ShaderAttribute* GetAtIndex(int index);
  Zilch::CodeLocation* GetLocation(ShaderAttribute* shaderAttribute);
  Zilch::CodeLocation* GetLocation(ShaderAttribute* shaderAttribute, Zilch::AttributeParameter* param);

private:
  Array<ShaderAttribute> mAttributes;
  Array<Zilch::AttributeNode*> mNodes;
};

}//namespace Zero
