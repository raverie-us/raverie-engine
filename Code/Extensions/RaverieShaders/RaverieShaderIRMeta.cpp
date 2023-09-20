// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
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
  mRaverieType = nullptr;
  mOwner = nullptr;
  mRaverieProperty = nullptr;
}

ShaderFieldKey ShaderIRFieldMeta::MakeFieldKey() const
{
  return ShaderFieldKey(mRaverieName, mRaverieType->ToString());
}

ShaderFieldKey ShaderIRFieldMeta::MakeFieldKey(ShaderIRAttribute* attribute) const
{
  return ShaderFieldKey(GetFieldAttributeName(attribute), mRaverieType->ToString());
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
  return mRaverieName;
}

ShaderIRFieldMeta* ShaderIRFieldMeta::Clone(RaverieShaderIRLibrary* owningLibrary) const
{
  ShaderIRFieldMeta* clone = new ShaderIRFieldMeta();
  owningLibrary->mOwnedFieldMeta.PushBack(clone);

  clone->mRaverieName = mRaverieName;
  clone->mRaverieType = mRaverieType;
  clone->mOwner = mOwner;
  clone->mRaverieProperty = mRaverieProperty;
  clone->mAttributes = mAttributes;

  return clone;
}

ShaderIRFunctionMeta::ShaderIRFunctionMeta()
{
  mRaverieFunction = nullptr;
}

ShaderIRTypeMeta::ShaderIRTypeMeta()
{
  mRaverieType = nullptr;
  mFragmentType = FragmentType::None;
}

ShaderIRFieldMeta* ShaderIRTypeMeta::CreateField(RaverieShaderIRLibrary* library)
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
    if (fieldMeta->mRaverieName == fieldName)
      return fieldMeta;
  }
  return nullptr;
}

ShaderIRFunctionMeta* ShaderIRTypeMeta::CreateFunction(RaverieShaderIRLibrary* library)
{
  ShaderIRFunctionMeta* functionMeta = new ShaderIRFunctionMeta();
  mFunctions.PushBack(functionMeta);
  library->mOwnedFunctionMeta.PushBack(functionMeta);
  return functionMeta;
}

} // namespace Raverie
