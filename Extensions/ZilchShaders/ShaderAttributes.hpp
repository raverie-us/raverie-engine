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
// @JoshD: Legacy
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
// @JoshD: Legacy
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

  // @JoshD: Rename to uppercase
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

//-------------------------------------------------------------------ShaderIRAttributeParameter
class ShaderIRAttributeParameter
{
public:
  ShaderIRAttributeParameter();
  ShaderIRAttributeParameter(Zilch::AttributeParameter& param, Zilch::SyntaxNode* node);

  String GetName() const;
  void SetName(StringParam name);

  Zilch::ConstantType::Enum GetType() const;

  String GetStringValue() const;
  void SetStringValue(StringParam stringValue);

  int GetIntValue() const;
  void SetIntValue(int intValue);

  float GetFloatValue() const;
  void SetFloatValue(float floatValue);

  Zilch::Type* GetTypeValue() const;
  void SetTypeValue(Zilch::Type* typeValue);

  Zilch::CodeLocation* GetLocation();
  void SetLocationNode(Zilch::SyntaxNode* node);

  // Return the internal zilch attribute parameter. Mostly exposed for ease of binding.
  Zilch::AttributeParameter& GetZilchAttributeParameter();

private:
  Zilch::AttributeParameter mParameter;
  Zilch::SyntaxNode* mNode;
};

//-------------------------------------------------------------------ShaderIRAttribute
class ShaderIRAttribute
{
public:
  ShaderIRAttribute();
  ShaderIRAttribute(StringParam attributeName, Zilch::SyntaxNode* locationNode);

  ShaderIRAttributeParameter* FindFirstParameter(StringParam name);
  Zilch::CodeLocation* GetLocation();

  String mAttributeName;
  Array<ShaderIRAttributeParameter> mParameters;
  Zilch::SyntaxNode* mNode;

  // Was this attribute created from another (e.g. [Input] implies [AppBuiltInInput]).
  // Some errors are only valid if the attribute was explicitly declared.
  bool mImplicitAttribute;
};

//-------------------------------------------------------------------ShaderIRAttributeList
class ShaderIRAttributeList
{
public:
  typedef Array<ShaderIRAttribute>::range Range;

  class NamedRange
  {
  public:
    NamedRange();
    NamedRange(StringParam attributeToFind, const Range& range);

    ShaderIRAttribute* Front();
    bool Empty() const;
    void PopFront();

  private:
    void SkipAttributes();

    String mAttributeToFind;
    Range mRange;
  };

  ShaderIRAttribute* AddAttribute(StringParam attributeName, Zilch::AttributeNode* node);
  NamedRange FindAttributes(StringParam attributeName);
  ShaderIRAttribute* FindFirstAttribute(StringParam attributeName);
  Range All();
  size_t Size();

  ShaderIRAttribute* GetAtIndex(int index);
  ShaderIRAttribute* operator[](int index);

private:
  Array<ShaderIRAttribute> mAttributes;
};

}//namespace Zero
