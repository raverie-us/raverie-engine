///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ShaderTypeData
ShaderTypeData::ShaderTypeData()
{
  mSizeX = mSizeY = 0;
  mCount = 0;
  mType = ShaderVarType::Other;
}

//-------------------------------------------------------------------ShaderLocation
ShaderLocation::ShaderLocation()
{
  mUserData = nullptr;
}

//-------------------------------------------------------------------ShaderFieldKey
ShaderFieldKey::ShaderFieldKey(ShaderField* shaderField)
{
  Set(shaderField->mZilchName, shaderField->mZilchType);
}

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

//-------------------------------------------------------------------ShaderField
ShaderField::ShaderField()
{
  mIsStatic = false;
  mIsShared = false;
  mOwner = nullptr;
}

bool ShaderField::IsStatic() const
{
  return mIsStatic;
}
bool ShaderField::IsShared() const
{
  return mIsShared;
}

ShaderType* ShaderField::GetShaderType()
{
  return mOwner->mOwningLibrary->FindType(mZilchType);
}

bool ShaderField::ContainsAttribute(StringParam attributeName)
{
  return !mAttributes.FindAttributes(attributeName).Empty();
}

//-------------------------------------------------------------------ShaderFunction
ShaderFunction::ShaderFunction()
{
  mIsStatic = false;
  mOwner = nullptr;
}

bool ShaderFunction::IsStatic() const
{
  return mIsStatic;
}

bool ShaderFunction::ContainsAttribute(StringParam attributeName)
{
  return !mAttributes.FindAttributes(attributeName).Empty();
}

//-------------------------------------------------------------------ShaderType
ShaderType::ShaderType()
{
  mOwningLibrary = nullptr;
  mFragmentType = FragmentType::None;
  mInputType = nullptr;
  mOutputType = nullptr;
}

ShaderType::~ShaderType()
{
  for(size_t i = 0; i < mFieldList.Size(); ++i)
    delete mFieldList[i];
  for(size_t i = 0; i < mFunctionList.Size(); ++i)
    delete mFunctionList[i];
  for(size_t i = 0; i < mFinalShaderFunctions.Size(); ++i)
    delete mFinalShaderFunctions[i];
}

ShaderFunction* ShaderType::FindFunction(void* zilchFnKey)
{
  return mFunctionOverloadMap.FindValue(zilchFnKey, nullptr);
}

ShaderFunction* ShaderType::FindOrCreateFunction(StringParam zilchFunctionName, void* zilchFnKey)
{
  ErrorIf(zilchFnKey == nullptr, "Cannot register a null function");

  ShaderFunction* result = FindFunction(zilchFnKey);
  if(result != nullptr)
    return result;

  result = new ShaderFunction();
  result->mZilchName = zilchFunctionName;
  result->mOwner = this;
  mFunctionNameMultiMap[zilchFunctionName].PushBack(result);
  mFunctionOverloadMap[zilchFnKey] = result;
  mFunctionList.PushBack(result);
  return result;
}

ShaderFunction* ShaderType::CreateFinalShaderFunction(StringParam shaderFunctionName)
{
  ShaderFunction* shaderFunction = new ShaderFunction();
  mFinalShaderFunctions.PushBack(shaderFunction);
  shaderFunction->mOwner = this;
  shaderFunction->mShaderName = shaderFunctionName;
  return shaderFunction;
}

ShaderField* ShaderType::FindField(StringParam zilchFieldName)
{
  return mFieldMap.FindValue(zilchFieldName, nullptr);
}

ShaderField* ShaderType::FindOrCreateField(StringParam zilchFieldName)
{
  ShaderField* field = FindField(zilchFieldName);
  if(field != nullptr)
    return field;

  field = new ShaderField();
  field->mZilchName = zilchFieldName;
  field->mOwner = this;
  mFieldList.PushBack(field);
  mFieldMap[zilchFieldName] = field;
  return field;
}

void ShaderType::AddDependency(ShaderType* shaderType)
{
  mOwningLibrary->mTypeDependents[shaderType].Insert(this);
  mDependencyList.PushBack(shaderType);
}

bool ShaderType::ContainsAttribute(StringParam attributeName)
{
  return !mAttributes.FindAttributes(attributeName).Empty();
}

ShaderTypeData* ShaderType::GetTypeData()
{
  return &mTypeData;
}

bool ShaderType::GetIntrinsic() const
{
  return mFlags.IsSet(ShaderTypeFlags::Intrinsic);
}

void ShaderType::SetIntrinsic(bool state)
{
  mFlags.SetState(ShaderTypeFlags::Intrinsic | ShaderTypeFlags::HideStruct, state);
}

bool ShaderType::IsNonCopyable() const
{
  return mFlags.IsSet(ShaderTypeFlags::NonCopyable);
}

void ShaderType::SetNonCopyable(bool state)
{
  mFlags.SetState(ShaderTypeFlags::NonCopyable, state);
}

bool ShaderType::IsNative() const
{
  return mFlags.IsSet(ShaderTypeFlags::Native);
}

void ShaderType::SetNative(bool state)
{
  mFlags.SetState(ShaderTypeFlags::Native, state);
}

bool ShaderType::GetHasMain() const
{
  return mFlags.IsSet(ShaderTypeFlags::HasMain);
}

void ShaderType::SetHasMain(bool state)
{
  mFlags.SetState(ShaderTypeFlags::HasMain, state);
}

bool ShaderType::GetHideFunctions()
{
  return mFlags.IsSet(ShaderTypeFlags::HideFunctions);
}

bool ShaderType::GetHideStruct()
{
  return mFlags.IsSet(ShaderTypeFlags::HideStruct);
}

void ShaderType::SetHideStructAndFunctions(bool state)
{
  mFlags.SetState(ShaderTypeFlags::HideFunctions | ShaderTypeFlags::HideStruct, state);
}

}//namespace Zero
