#include "Precompiled.hpp"

#include "ZilchShaderIRExtendedTypes.hpp"

namespace Zero
{

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

int ZilchShaderIRImageType::GetIntegerConstantParameterValue(int parameterIndex)
{
  IZilchShaderIR* parameter = mIRType->mParameters[parameterIndex];
  ZilchShaderIRConstantLiteral* constantLiteral = parameter->As<ZilchShaderIRConstantLiteral>();
  if(constantLiteral == nullptr)
    return -1;
  int value = constantLiteral->mValue.Get<int>();
  return value;
}

}//namespace Zero
