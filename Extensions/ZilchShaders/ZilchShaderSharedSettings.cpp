///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------NameSettings
String NameSettings::mVertexIntrinsicAttributeName = "VertexIntrinsic";
String NameSettings::mGeometryIntrinsicAttributeName = "GeometryIntrinsic";
String NameSettings::mPixelIntrinsicAttributeName = "PixelIntrinsic";
String NameSettings::mNonCopyableAttribute = "NonCopyable";
String NameSettings::mForcedStaticAttribute = "ForcedStatic";

NameSettings::NameSettings()
{
  mPerspectivePositionName = "ApiPerspectivePosition";

  mPixelAttributeName = "Pixel";
  mVertexAttributeName = "Vertex";
  mGeometryAttributeName = "Geometry";

  mStaticAttribute = "Static";
  mPreConstructorName = "PreConstructor";
  mConstructorName = "Constructor";
  mReferenceKeyword = "inout";
  mThisKeyword = "self";
  mMainAttribute = "Main";
  mNoMangleAttributeName = "NoMangle";
  mMainAttributeTag = "[Main]";

  mUniformTag = "[Uniform]";
  mStageInputTag = "[StageInput]";
  mStageOutputTag = "[StageOutput]";
  mSystemValueInputTag = "[SVInput]";
  mSystemValueOutputTag = "[SVOutput]";

  mInputAttributeName = "Input";
  mOutputAttributeName = "Output";
  mInputTag = "[Input]";
  mOutputTag = "[Output]";
  mFragmentInputAttributeName = "FragmentInput";
  mStageInputAttributeName = "StageInput";
  mStageOutputAttributeName = "StageOutput";
  mSystemValueInputAttributeName = "SVInput";
  mSystemValueOutputAttributeName = "SVOutput";
  mBuiltinInputAttributeName = "BuiltInInput";
  mPropertyInputAttributeName = "PropertyInput";
  mInputSubAttributes.PushBack(mFragmentInputAttributeName);
  mInputSubAttributes.PushBack(mStageInputAttributeName);
  mInputSubAttributes.PushBack(mBuiltinInputAttributeName);
  mInputSubAttributes.PushBack(mPropertyInputAttributeName);

  mGeometryShaderOutputTag = "[GsOut]";

  mUniformName = "Uniform";
  mGeometryShaderOutputAttribute = "GsOut";

  mMaxVerticesParamName = "maxVertices";

  mImplementAttribute = "Implements";
  mExtensionAttribute = "Extension";
  mIntrinsicAttribute = "Intrinsic";
  mLanguageParamName = "language";
  mTypeNameParamName = "typeName";
  mStorageQualifierParamName = "storageQualifier";
  mRefTypeParamName = "refType";
  mPropertyTypeParamName = "propertyType";
  mNonCopyableParamName = "nonCopyable";
  mForcedStaticParamName = "forcedStatic";

  mSharedInputAttributeName = "Shared";
  mSharedInputNameParamName = "name";

  mEmitVertexHelperFunctionName = "EmitVertexHelper";

  mPixelShaderInputPrefix = "psIn";
  mGeometryShaderInputPrefix = "gsIn";
  mAttributePrefix = "att";

  mAllowedClassAttributes.Insert(mVertexAttributeName);
  mAllowedClassAttributes.Insert(mGeometryAttributeName);
  mAllowedClassAttributes.Insert(mPixelAttributeName);
  mAllowedClassAttributes.Insert(mStaticAttribute);
  mAllowedClassAttributes.Insert(mIntrinsicAttribute);
  mAllowedClassAttributes.Insert(mNonCopyableAttribute);
  mAllowedClassAttributes.Insert(mForcedStaticAttribute);
  mAllowedClassAttributes.Insert(mImplementAttribute);

  mAllowedFunctionAttributes.Insert(mMainAttribute);
  mAllowedFunctionAttributes.Insert(mStaticAttribute);
  mAllowedFunctionAttributes.Insert(mExtensionAttribute);
  mAllowedFunctionAttributes.Insert(mImplementAttribute);
  mAllowedFunctionAttributes.Insert(mNoMangleAttributeName);
  mAllowedFunctionAttributes.Insert(mVertexIntrinsicAttributeName);
  mAllowedFunctionAttributes.Insert(mGeometryIntrinsicAttributeName);
  mAllowedFunctionAttributes.Insert(mPixelIntrinsicAttributeName);

  mAllowedFieldAttributes.Insert(mStaticAttribute);
  mAllowedFieldAttributes.Insert(mSharedInputAttributeName);
  mAllowedFieldAttributes.Insert(mNoMangleAttributeName);
  mAllowedFieldAttributes.Insert(mInputAttributeName);
  mAllowedFieldAttributes.Insert(mOutputAttributeName);
  mAllowedFieldAttributes.Insert(mUniformName);
  mAllowedFieldAttributes.Insert(mFragmentInputAttributeName);
  mAllowedFieldAttributes.Insert(mStageInputAttributeName);
  mAllowedFieldAttributes.Insert(mStageOutputAttributeName);
  mAllowedFieldAttributes.Insert(mSystemValueInputAttributeName);
  mAllowedFieldAttributes.Insert(mSystemValueOutputAttributeName);
  mAllowedFieldAttributes.Insert(mBuiltinInputAttributeName);
  mAllowedFieldAttributes.Insert(mPropertyInputAttributeName);
  mAllowedFieldAttributes.Insert(mGeometryShaderOutputAttribute);
}

//-------------------------------------------------------------------ShaderDefinitionSettings
void ShaderDefinitionSettings::SetMaxSimultaneousRenderTargets(size_t maxNumber)
{
  mRenderTargetNames.Resize(maxNumber);
}

void ShaderDefinitionSettings::SetRenderTargetName(StringParam varName, size_t targetIndex)
{
  // Make sure the user is not trying to set an invalid render target name
  if(targetIndex >= mRenderTargetNames.Size())
  {
    Error("Render target %d is invalid. There are only %d allowed simultaneous render targets. "
          "Use SetMaxSimultaneousRenderTargets to increase the limit.", targetIndex, mRenderTargetNames.Size());
    return;
  }

  // Make sure the user is not setting two render target indices to same name
  for(size_t i = 0; i < mRenderTargetNames.Size(); ++i)
  {
    if(i == targetIndex)
      continue;

    if(mRenderTargetNames[i] == varName)
    {
      Error("Render target name '%s' is already used in target(%d).", varName.c_str(), i);
    }
  }

  mRenderTargetNames[targetIndex] = varName;
}

void ShaderDefinitionSettings::AddBuiltIn(StringParam name, StringParam type)
{
  ShaderFieldKey fieldKey(name, type);
  ErrorIf(mBuiltIns.ContainsKey(fieldKey), "BuiltIn '%s' of type '%s' already exists", name.c_str(), type.c_str());
  
  ShaderField* field = new ShaderField();
  field->mZilchName = name;
  field->mZilchType = type;
  mBuiltIns.Insert(fieldKey, field);
}

void ShaderDefinitionSettings::AddVertexDefinition(StringParam varName, StringParam varType)
{
  ShaderFieldKey fieldKey(varName, varType);
  ErrorIf(mVertexDefinitions.ContainsKey(fieldKey), "Vertex definition '%s' of type '%s' already exists", varName.c_str(), varType.c_str());

  ShaderField* field = new ShaderField();
  field->mZilchName = varName;
  field->mZilchType = varType;
  mVertexDefinitions.Insert(fieldKey, field);
}

//-------------------------------------------------------------------LanguageSystemValue
LanguageSystemValue::LanguageSystemValue()
{
  mIoMode = 0;
  mMinVersion = 0;
  mMaxVersion = 9999;
}

void LanguageSystemValue::SetInput(StringParam type, StringParam name, StringParam attribute)
{
  mIoMode.SetFlag(SystemValueStates::Input);
  mInputType = type;
  mInputName = name;
}

void LanguageSystemValue::SetOutput(StringParam type, StringParam name, StringParam attribute)
{
  mIoMode.SetFlag(SystemValueStates::Output);
  mOutputType = type;
  mOutputName = name;
}

void LanguageSystemValue::SetInputAndOutput(StringParam type, StringParam name, StringParam attribute)
{
  SetInput(type, name, attribute);
  SetOutput(type, name, attribute);
}

bool LanguageSystemValue::GetForced()
{
  return mIoMode.IsSet(SystemValueStates::Forced);
}

void LanguageSystemValue::SetForced(bool state)
{
  return mIoMode.SetState(SystemValueStates::Forced, state);
}

bool LanguageSystemValue::IsInput() const
{
  return mIoMode.IsSet(SystemValueStates::Input);
}

bool LanguageSystemValue::IsOutput() const
{
  return mIoMode.IsSet(SystemValueStates::Output);
}

//-------------------------------------------------------------------ShaderSystemValue
LanguageSystemValue*  ShaderSystemValue::AddLanguageSystemValue(StringParam language)
{
  LanguageSystemValue& systemValue = mLanguages[language];
  return &systemValue;
}

//-------------------------------------------------------------------ShaderSystemValueStageSettings
ShaderSystemValue* ShaderSystemValueStageSettings::AddSystemValue(StringParam type, StringParam name)
{
  ShaderFieldKey fieldKey(name, type);
  ErrorIf(mSystemValues.ContainsKey(fieldKey), "BuiltIn '%s' of type '%s' already exists", name.c_str(), type.c_str());

  ShaderSystemValue& systemValue = mSystemValues[fieldKey];
  systemValue.mKey = fieldKey;
  systemValue.mName = name;
  systemValue.mType = type;
  return &systemValue;
}

bool ShaderSystemValueStageSettings::ContainsSystemValue(ShaderFieldKey& fieldKey)
{
  return mSystemValues.ContainsKey(fieldKey);
}

ShaderSystemValue* ShaderSystemValueStageSettings::FindSystemValue(ShaderFieldKey& fieldKey)
{
  return mSystemValues.FindPointer(fieldKey);
}

//-------------------------------------------------------------------ShaderSystemValueSettings
ShaderSystemValueSettings::ShaderSystemValueSettings()
{
  mShaderStages.Resize(ShaderStageType::Size);
}

ShaderSystemValue* ShaderSystemValueSettings::AddSystemValue(ShaderStageType::Enum fragmentStage, StringParam name, StringParam type)
{
  ShaderSystemValueStageSettings& stageMap = mShaderStages[fragmentStage];

  return stageMap.AddSystemValue(type, name);
}

bool ShaderSystemValueSettings::ContainsSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language, int languageVersion)
{
  LanguageSystemValue* shaderSystemValue = FindSystemValue(fragmentStage, fieldKey, language);
  if(shaderSystemValue != nullptr)
    return (shaderSystemValue->mMinVersion <= languageVersion && languageVersion <= shaderSystemValue->mMaxVersion);
  return false;
}

ShaderSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey)
{
  ShaderSystemValueStageSettings& stageMap = mShaderStages[fragmentStage];
  return stageMap.FindSystemValue(fieldKey);
}

LanguageSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language)
{
  ShaderSystemValue* shaderSystemValue = FindSystemValue(fragmentStage, fieldKey);
  if(shaderSystemValue != nullptr)
    return &shaderSystemValue->mLanguages[language];
  return nullptr;
}

LanguageSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& fieldKey, StringParam language, int languageVersion)
{
  LanguageSystemValue* systemValue = FindSystemValue(fragmentStage, fieldKey, language);
  if(systemValue != nullptr)
  {
    if(systemValue->mMinVersion <= languageVersion && languageVersion <= systemValue->mMaxVersion)
      return systemValue;
  }
  return nullptr;
}

LanguageSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderField* shaderField, BaseShaderTranslator* translator)
{
  int versionNumber = translator->GetLanguageVersionNumber();
  String language = translator->GetLanguageName();
  ShaderStageType::Enum fragmentStage = FragmentTypeToShaderStageType(shaderField->mOwner->mFragmentType);
  ShaderFieldKey fieldKey(shaderField);
  return FindSystemValue(fragmentStage, fieldKey, language, versionNumber);
}

LanguageSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderField* shaderField, BaseShaderTranslator* translator)
{
  int versionNumber = translator->GetLanguageVersionNumber();
  String language = translator->GetLanguageName();
  ShaderFieldKey fieldKey(shaderField);
  return FindSystemValue(fragmentStage, fieldKey, language, versionNumber);
}

LanguageSystemValue* ShaderSystemValueSettings::FindSystemValue(ShaderStageType::Enum fragmentStage, ShaderFieldKey& shaderFieldKey, BaseShaderTranslator* translator)
{
  int versionNumber = translator->GetLanguageVersionNumber();
  String language = translator->GetLanguageName();
  return FindSystemValue(fragmentStage, shaderFieldKey, language, versionNumber);
}


ShaderStageType::Enum ShaderSystemValueSettings::FragmentTypeToShaderStageType(FragmentType::Enum fragmentType)
{
  if(fragmentType == FragmentType::Vertex)
    return ShaderStageType::Vertex;
  else if(fragmentType == FragmentType::Geometry)
    return ShaderStageType::GeometryVertex;
  else if(fragmentType == FragmentType::Pixel)
    return ShaderStageType::Pixel;
  return ShaderStageType::GeometryPrimitive;
}

//-------------------------------------------------------------------ZilchShaderSettings
char ZilchShaderSettings::mDefaultConstructorKey;

void* ZilchShaderSettings::GetDefaultConstructorKey()
{
  return &mDefaultConstructorKey;
}

String GenerateMangledName(StringParam shaderTypeName, StringParam symbolName)
{
  return BuildString("_", shaderTypeName, symbolName);
}

String GenerateFieldUniformName(ShaderField* shaderField)
{
  return GenerateFieldUniformName(shaderField->mOwner->mZilchName, shaderField->mZilchName);
}

String GenerateFieldUniformName(StringParam zilchTypeName, StringParam fieldName)
{
  return BuildString(zilchTypeName, "_", fieldName);
}

}//namespace Zero
