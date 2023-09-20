// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class ShaderIRAttributeParameter
{
public:
  ShaderIRAttributeParameter();
  ShaderIRAttributeParameter(Raverie::AttributeParameter& param, Raverie::SyntaxNode* node);

  String GetName() const;
  void SetName(StringParam name);

  Raverie::ConstantType::Enum GetType() const;

  String GetStringValue() const;
  void SetStringValue(StringParam stringValue);

  int GetIntValue() const;
  void SetIntValue(int intValue);

  float GetFloatValue() const;
  void SetFloatValue(float floatValue);

  Raverie::Type* GetTypeValue() const;
  void SetTypeValue(Raverie::Type* typeValue);

  Raverie::CodeLocation* GetLocation();
  void SetLocationNode(Raverie::SyntaxNode* node);

  // Return the internal raverie attribute parameter. Mostly exposed for ease of
  // binding.
  Raverie::AttributeParameter& GetRaverieAttributeParameter();

private:
  Raverie::AttributeParameter mParameter;
  Raverie::SyntaxNode* mNode;
};

class ShaderIRAttribute
{
public:
  ShaderIRAttribute();
  ShaderIRAttribute(StringParam attributeName, Raverie::SyntaxNode* locationNode);

  ShaderIRAttributeParameter* FindFirstParameter(StringParam name);
  Raverie::CodeLocation* GetLocation();

  String mAttributeName;
  Array<ShaderIRAttributeParameter> mParameters;
  Raverie::SyntaxNode* mNode;

  // Was this attribute created from another (e.g. [Input] implies
  // [AppBuiltInInput]). Some errors are only valid if the attribute was
  // explicitly declared.
  bool mImplicitAttribute;
};

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

  ShaderIRAttribute* AddAttribute(StringParam attributeName, Raverie::AttributeNode* node);
  NamedRange FindAttributes(StringParam attributeName);
  ShaderIRAttribute* FindFirstAttribute(StringParam attributeName);
  Range All();
  size_t Size();

  ShaderIRAttribute* GetAtIndex(int index);
  ShaderIRAttribute* operator[](int index);

private:
  Array<ShaderIRAttribute> mAttributes;
};

} // namespace Raverie
