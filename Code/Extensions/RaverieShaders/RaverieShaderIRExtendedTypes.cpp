// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "RaverieShaderIRExtendedTypes.hpp"

namespace Raverie
{

RaverieShaderIRImageType::RaverieShaderIRImageType()
{
  mIRType = nullptr;
}

RaverieShaderIRImageType::RaverieShaderIRImageType(RaverieShaderIRType* type)
{
  Load(type);
}

bool RaverieShaderIRImageType::Load(RaverieShaderIRType* type)
{
  if (type->mBaseType != ShaderIRTypeBaseType::Image)
  {
    mIRType = nullptr;
    return false;
  }

  mIRType = type;
  return true;
}

RaverieShaderIRType* RaverieShaderIRImageType::GetSampledType()
{
  return mIRType->mParameters[0]->As<RaverieShaderIRType>();
}

int RaverieShaderIRImageType::GetDim()
{
  return GetIntegerConstantParameterValue(1);
}

int RaverieShaderIRImageType::GetDepth()
{
  return GetIntegerConstantParameterValue(2);
}

int RaverieShaderIRImageType::GetArrayed()
{
  return GetIntegerConstantParameterValue(3);
}

int RaverieShaderIRImageType::GetMultiSampled()
{
  return GetIntegerConstantParameterValue(4);
}

int RaverieShaderIRImageType::GetSampled()
{
  return GetIntegerConstantParameterValue(5);
}

int RaverieShaderIRImageType::GetFormat()
{
  return GetIntegerConstantParameterValue(6);
}

bool RaverieShaderIRImageType::IsStorageImage()
{
  return GetSampled() == 2;
}

int RaverieShaderIRImageType::GetIntegerConstantParameterValue(int parameterIndex)
{
  IRaverieShaderIR* parameter = mIRType->mParameters[parameterIndex];
  RaverieShaderIRConstantLiteral* constantLiteral = parameter->As<RaverieShaderIRConstantLiteral>();
  if (constantLiteral == nullptr)
    return -1;
  int value = constantLiteral->mValue.Get<int>();
  return value;
}

RaverieShaderIRRuntimeArrayType::RaverieShaderIRRuntimeArrayType()
{
  mIRType = nullptr;
}

bool RaverieShaderIRRuntimeArrayType::Load(RaverieShaderIRType* type)
{
  ShaderIRTypeMeta* typeMeta = type->mMeta;
  if (typeMeta == nullptr)
    return false;

  Raverie::BoundType* raverieType = typeMeta->mRaverieType;
  if (raverieType == nullptr)
    return false;

  // Only visit runtime array types (the struct that wraps the actual spirv
  // runtime array)
  if (raverieType->TemplateBaseName != SpirVNameSettings::mRuntimeArrayTypeName)
    return false;

  mIRType = type;
  return true;
}

RaverieShaderIRType* RaverieShaderIRRuntimeArrayType::GetSpirVRuntimeArrayType()
{
  if (mIRType == nullptr)
    return nullptr;

  return mIRType->mParameters[0]->As<RaverieShaderIRType>();
}

RaverieShaderIRType* RaverieShaderIRRuntimeArrayType::GetContainedType()
{
  RaverieShaderIRType* spirVRuntimeArrayType = GetSpirVRuntimeArrayType();
  if (spirVRuntimeArrayType == nullptr)
    return nullptr;

  return spirVRuntimeArrayType->mParameters[0]->As<RaverieShaderIRType>();
}

} // namespace Raverie
