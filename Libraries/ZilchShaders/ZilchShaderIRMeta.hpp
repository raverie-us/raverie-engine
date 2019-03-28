// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class ZilchShaderIRLibrary;
class ShaderIRTypeMeta;

DeclareEnum5(FragmentType, Vertex, Pixel, Geometry, Compute, None);

/// A typed string for looking up shader fields. This is a specific type to help
/// avoid ambiguities on HashMap<String, ShaderField*> where it's hard to know
/// what the string's value is. This type implies that the hashmap is not based
/// upon the field's name, but rather it's key (name:string).
struct ShaderFieldKey
{
  ShaderFieldKey(){};
  ShaderFieldKey(StringParam fieldName, StringParam fieldType);

  void Set(StringParam fieldName, StringParam fieldType);

  size_t Hash() const;
  bool operator==(const ShaderFieldKey& other) const;
  operator String() const;

  String mKey;
};

/// Base meta type of all shader meta objects. Currently only contains
/// attributes.
class ShaderIRMeta
{
public:
  bool ContainsAttribute(StringParam attributeName);
  ShaderIRAttributeList mAttributes;
};

/// Represents a field defined in zilch.
class ShaderIRFieldMeta : public ShaderIRMeta
{
public:
  ShaderIRFieldMeta();

  /// Make a key for this field. Used for hashing.
  ShaderFieldKey MakeFieldKey() const;
  /// Make a key for this field via the given attribute (for the field name).
  /// Used for hashing.
  ShaderFieldKey MakeFieldKey(ShaderIRAttribute* attribute) const;

  /// Given an attribute, return the field name for this variable. This returns
  /// the name in the attribute if it exists, otherwise the regular variable
  /// name.
  String GetFieldAttributeName(ShaderIRAttribute* attribute) const;

  /// Creates a clone of the current field meta. The given library will take
  /// ownership of the cloned field.
  ShaderIRFieldMeta* Clone(ZilchShaderIRLibrary* owningLibrary) const;

  /// The name of the field in zilch.
  String mZilchName;
  /// The type of the field in zilch.
  Zilch::BoundType* mZilchType;
  /// The type that owns this field.
  ShaderIRTypeMeta* mOwner;

  /// The property that this field comes from.
  /// Useful for further reflection (such as finding the field's location).
  Zilch::Property* mZilchProperty;
  /// The default value of this field after evaluating zilch.
  /// Used for reflection to extract the default value that a user sets.
  Zilch::Any mDefaultValueVariant;
};

/// Represents a function defined in zilch.
class ShaderIRFunctionMeta : public ShaderIRMeta
{
public:
  ShaderIRFunctionMeta();

  /// The name of the zilch function.
  String mZilchName;
  /// The zilch function this comes from. Useful for more complicated
  /// reflection.
  Zilch::Function* mZilchFunction;
};

/// Represents a type defined in zilch.
class ShaderIRTypeMeta : public ShaderIRMeta
{
public:
  ShaderIRTypeMeta();

  /// Internal function used by shader translation to create a field meta
  /// object.
  ShaderIRFieldMeta* CreateField(ZilchShaderIRLibrary* library);
  /// Find a field meta object by name.
  ShaderIRFieldMeta* FindField(StringParam fieldName);

  /// Internal function used by shader translation to create a function meta
  /// object.
  ShaderIRFunctionMeta* CreateFunction(ZilchShaderIRLibrary* library);

  /// The name of the zilch type.
  String mZilchName;
  /// The zilch type this comes from. Useful for more complicated reflection.
  Zilch::BoundType* mZilchType;

  /// All of the owned fields that this type owns.
  Array<ShaderIRFieldMeta*> mFields;
  /// All of the owned functions that this type owns.
  Array<ShaderIRFunctionMeta*> mFunctions;

  /// The kind of shader fragment type that this represents.
  FragmentType::Enum mFragmentType;

  /// Any user data that cant simply be represented by a pointer
  /// Data can be written to the buffer and will be properly destructed
  /// when this object is destroyed (must be read in the order it's written)
  mutable Zilch::DestructibleBuffer mComplexUserData;
};

} // namespace Zero
