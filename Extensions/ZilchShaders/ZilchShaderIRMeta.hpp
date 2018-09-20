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

DeclareEnum4(FragmentType, Vertex, Pixel, Geometry, None);

//-------------------------------------------------------------------ShaderFieldKey
// A typed string for looking up shader fields. This is a specific type to help avoid ambiguities on
// HashMap<String, ShaderField*> where it's hard to know what the string's value is. This type implies
// that the hashmap is not based upon the field's name, but rather it's key (name:string).
struct ShaderFieldKey
{
  ShaderFieldKey() {};
  //ShaderFieldKey(ShaderField* shaderField);
  ShaderFieldKey(StringParam fieldName, StringParam fieldType);

  void Set(StringParam fieldName, StringParam fieldType);

  size_t Hash() const;
  bool operator==(const ShaderFieldKey& other) const;
  operator String() const;

  String mKey;
};

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

  // Any user data that cant simply be represented by a pointer
  // Data can be written to the buffer and will be properly destructed
  // when this object is destroyed (must be read in the order it's written)
  mutable Zilch::DestructibleBuffer mComplexUserData;
};

}//namespace Zero
