// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ShaderFieldKey::ShaderFieldKey(StringParam fieldName, StringParam fieldType)
{
  Set(fieldName, fieldType);
}

void ShaderFieldKey::Set(StringParam fieldName, StringParam fieldType)
{
  mKey = BuildString(fieldName, ":", fieldType);
}

size_t ShaderFieldKey::Hash() const
{
  return mKey.Hash();
}

bool ShaderFieldKey::operator==(const ShaderFieldKey& other) const
{
  return mKey == other.mKey;
}

ShaderFieldKey::operator String() const
{
  return mKey;
}

bool ShaderIRMeta::ContainsAttribute(StringParam attributeName)
{
  return !mAttributes.FindAttributes(attributeName).Empty();
}

ShaderIRFieldMeta::ShaderIRFieldMeta()
{
  mZilchType = nullptr;
  mOwner = nullptr;
  mZilchProperty = nullptr;
}

ShaderFieldKey ShaderIRFieldMeta::MakeFieldKey() const
{
  return ShaderFieldKey(mZilchName, mZilchType->ToString());
}

ShaderFieldKey ShaderIRFieldMeta::MakeFieldKey(ShaderIRAttribute* attribute) const
{
  return ShaderFieldKey(GetFieldAttributeName(attribute), mZilchType->ToString());
}

String ShaderIRFieldMeta::GetFieldAttributeName(ShaderIRAttribute* attribute) const
{
  String nameOverrideParam = SpirVNameSettings::mNameOverrideParam;
  // If the field contains a name override attribute then use that to make the
  // field key instead.
  ShaderIRAttributeParameter* param = attribute->FindFirstParameter(nameOverrideParam);
  if (param != nullptr)
    return param->GetStringValue();

  // Check if there's an un-named parameter and count this as 'name'.
  // @JoshD: Cleanup later!
  if (attribute->mParameters.Size() == 1 && attribute->mParameters[0].GetName().Empty())
    return attribute->mParameters[0].GetStringValue();
  return mZilchName;
}

ShaderIRFieldMeta* ShaderIRFieldMeta::Clone(ZilchShaderIRLibrary* owningLibrary) const
{
  ShaderIRFieldMeta* clone = new ShaderIRFieldMeta();
  owningLibrary->mOwnedFieldMeta.PushBack(clone);

  clone->mZilchName = mZilchName;
  clone->mZilchType = mZilchType;
  clone->mOwner = mOwner;
  clone->mZilchProperty = mZilchProperty;
  clone->mAttributes = mAttributes;

  return clone;
}

ShaderIRFunctionMeta::ShaderIRFunctionMeta()
{
  mZilchFunction = nullptr;
}

ShaderIRTypeMeta::ShaderIRTypeMeta()
{
  mZilchType = nullptr;
  mFragmentType = FragmentType::None;
}

ShaderIRFieldMeta* ShaderIRTypeMeta::CreateField(ZilchShaderIRLibrary* library)
{
  ShaderIRFieldMeta* fieldMeta = new ShaderIRFieldMeta();
  fieldMeta->mOwner = this;
  mFields.PushBack(fieldMeta);
  library->mOwnedFieldMeta.PushBack(fieldMeta);
  return fieldMeta;
}

ShaderIRFieldMeta* ShaderIRTypeMeta::FindField(StringParam fieldName)
{
  for (size_t i = 0; i < mFields.Size(); ++i)
  {
    ShaderIRFieldMeta* fieldMeta = mFields[i];
    if (fieldMeta->mZilchName == fieldName)
      return fieldMeta;
  }
  return nullptr;
}

ShaderIRFunctionMeta* ShaderIRTypeMeta::CreateFunction(ZilchShaderIRLibrary* library)
{
  ShaderIRFunctionMeta* functionMeta = new ShaderIRFunctionMeta();
  mFunctions.PushBack(functionMeta);
  library->mOwnedFunctionMeta.PushBack(functionMeta);
  return functionMeta;
}

} // namespace Zero
