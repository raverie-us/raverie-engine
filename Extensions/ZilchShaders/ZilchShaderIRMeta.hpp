///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ZilchShaderIRLibrary;
class ShaderIRTypeMeta;

//-------------------------------------------------------------------ShaderIRMeta
/// Base meta type of all shader meta objects. Currently only contains attributes.
class ShaderIRMeta
{
public:

  bool ContainsAttribute(StringParam attributeName);
  ShaderIRAttributeList mAttributes;
};

//-------------------------------------------------------------------ShaderIRTypeMeta
class ShaderIRFieldMeta : public ShaderIRMeta
{
public:
  ShaderIRFieldMeta();

  // Make a key for this field. Used for hashing.
  ShaderFieldKey MakeFieldKey() const;
  // Make a key for this field via the given attribute (for the field name). Used for hashing.
  ShaderFieldKey MakeFieldKey(ShaderIRAttribute* attribute) const;

  // Given an attribute, return the field name for this variable. This returns
  // the name in the attribute if it exists, otherwise the regular variable name.
  String GetFieldAttributeName(ShaderIRAttribute* attribute) const;

  // Creates a clone of the current field meta. The given library will take ownership of the cloned field.
  ShaderIRFieldMeta* Clone(ZilchShaderIRLibrary* owningLibrary) const;

  String mZilchName;
  Zilch::BoundType* mZilchType;
  ShaderIRTypeMeta* mOwner;

  Zilch::Property* mZilchProperty;
  Zilch::Any mDefaultValueVariant;
};

//-------------------------------------------------------------------ShaderIRTypeMeta
class ShaderIRFunctionMeta : public ShaderIRMeta
{
public:
  ShaderIRFunctionMeta();

  String mZilchName;

  Zilch::Function* mZilchFunction;
};

//-------------------------------------------------------------------ShaderIRTypeMeta
class ShaderIRTypeMeta : public ShaderIRMeta
{
public:
  ShaderIRTypeMeta();

  ShaderIRFieldMeta* CreateField(ZilchShaderIRLibrary* library);
  ShaderIRFieldMeta* FindField(StringParam fieldName);

  ShaderIRFunctionMeta* CreateFunction(ZilchShaderIRLibrary* library);

  String mZilchName;
  Zilch::BoundType* mZilchType;

  Array<ShaderIRFieldMeta*> mFields;
  Array<ShaderIRFunctionMeta*> mFunctions;

  FragmentType::Enum mFragmentType;
};

}//namespace Zero
