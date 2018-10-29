#include "Precompiled.hpp"

#include "ZilchShaderIRExtendedTypes.hpp"

namespace Zero
{

//-------------------------------------------------------------------ZilchShaderIRImageType
ZilchShaderIRImageType::ZilchShaderIRImageType()
{
  mIRType = nullptr;
}

ZilchShaderIRImageType::ZilchShaderIRImageType(ZilchShaderIRType* type)
{
  Load(type);
}

bool ZilchShaderIRImageType::Load(ZilchShaderIRType* type)
{
  if(type->mBaseType != ShaderIRTypeBaseType::Image)
  {
    mIRType = nullptr;
    return false;
  }

  mIRType = type;
  return true;
}

ZilchShaderIRType* ZilchShaderIRImageType::GetSampledType()
{
  return mIRType->mParameters[0]->As<ZilchShaderIRType>();
}

int ZilchShaderIRImageType::GetDim()
{
  return GetIntegerConstantParameterValue(1);
}

int ZilchShaderIRImageType::GetDepth()
{
  return GetIntegerConstantParameterValue(2);
}

int ZilchShaderIRImageType::GetArrayed()
{
  return GetIntegerConstantParameterValue(3);
}

int ZilchShaderIRImageType::GetMultiSampled()
{
  return GetIntegerConstantParameterValue(4);
}

int ZilchShaderIRImageType::GetSampled()
{
  return GetIntegerConstantParameterValue(5);
}

int ZilchShaderIRImageType::GetFormat()
{
  return GetIntegerConstantParameterValue(6);
}

bool ZilchShaderIRImageType::IsStorageImage()
{
  return GetSampled() == 2;
}

int ZilchShaderIRImageType::GetIntegerConstantParameterValue(int parameterIndex)
{
  IZilchShaderIR* parameter = mIRType->mParameters[parameterIndex];
  ZilchShaderIRConstantLiteral* constantLiteral = parameter->As<ZilchShaderIRConstantLiteral>();
  if(constantLiteral == nullptr)
    return -1;
  int value = constantLiteral->mValue.Get<int>();
  return value;
}

//-------------------------------------------------------------------ZilchShaderIRRuntimeArrayType
ZilchShaderIRRuntimeArrayType::ZilchShaderIRRuntimeArrayType()
{
  mIRType = nullptr;
}

bool ZilchShaderIRRuntimeArrayType::Load(ZilchShaderIRType* type)
{
  ShaderIRTypeMeta* typeMeta = type->mMeta;
  if(typeMeta == nullptr)
    return false;

  Zilch::BoundType* zilchType = typeMeta->mZilchType;
  if(zilchType == nullptr)
    return false;

  // Only visit runtime array types (the struct that wraps the actual spirv runtime array)
  if(zilchType->TemplateBaseName != SpirVNameSettings::mRuntimeArrayTypeName)
    return false;

  mIRType = type;
  return true;
}

ZilchShaderIRType* ZilchShaderIRRuntimeArrayType::GetSpirVRuntimeArrayType()
{
  if(mIRType == nullptr)
    return nullptr;

  return mIRType->mParameters[0]->As<ZilchShaderIRType>();
}

ZilchShaderIRType* ZilchShaderIRRuntimeArrayType::GetContainedType()
{
  ZilchShaderIRType* spirVRuntimeArrayType = GetSpirVRuntimeArrayType();
  if(spirVRuntimeArrayType == nullptr)
    return nullptr;

  return spirVRuntimeArrayType->mParameters[0]->As<ZilchShaderIRType>();
}

}//namespace Zero
