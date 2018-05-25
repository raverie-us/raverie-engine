// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineType(ShaderInputs, builder, type)
{
  ZeroBindDocumented();
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, bool));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, int));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, IntVec2));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, IntVec3));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, IntVec4));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, float));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Vec2));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Vec3));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Vec4));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Mat3));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Mat4));
  ZilchBindOverloadedMethod(Add, ZilchInstanceOverload(void, String, String, Texture*));
  ZilchBindMethod(Remove);
  ZilchBindMethod(Clear);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, bool input)
{
  Add(fragmentName, inputName, ShaderInputType::Bool, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, int input)
{
  Add(fragmentName, inputName, ShaderInputType::Int, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, IntVec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec2, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, IntVec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec3, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, IntVec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec4, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, float input)
{
  Add(fragmentName, inputName, ShaderInputType::Float, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Vec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec2, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Vec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec3, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Vec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec4, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Mat3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat3, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Mat4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat4, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, Texture* input)
{
  Add(fragmentName, inputName, ShaderInputType::Texture, input);
}

//**************************************************************************************************
void ShaderInputs::Add(String fragmentName, String inputName, ShaderInputType::Enum type, AnyParam value)
{
  ZilchShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

  ShaderInput shaderInput = shaderGenerator->CreateShaderInput(fragmentName, inputName, type, value);
  if (shaderInput.mShaderInputType != ShaderInputType::Invalid)
    mShaderInputs.Insert(StringPair(fragmentName, inputName), shaderInput);
}

//**************************************************************************************************
void ShaderInputs::Remove(String fragmentName, String inputName)
{
  mShaderInputs.Erase(StringPair(fragmentName, inputName));
}

//**************************************************************************************************
void ShaderInputs::Clear()
{
  mShaderInputs.Clear();
}

DefineThreadSafeReferenceCountedHandle(GraphicsBlendSettings);
//**************************************************************************************************
ZilchDefineType(GraphicsBlendSettings, builder, type)
{
  ZeroBindDocumented();
  ZeroBindThreadSafeReferenceCountedHandle();
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  //ZilchBindMethod(SetBlendAlpha);
  //ZilchBindMethod(SetBlendAdditive);

  ZilchBindGetterSetterProperty(BlendMode);
  ZilchBindGetterSetterProperty(BlendEquation);
  ZilchBindGetterSetterProperty(SourceFactor);
  ZilchBindGetterSetterProperty(DestFactor);
  ZilchBindGetterSetterProperty(BlendEquationAlpha);
  ZilchBindGetterSetterProperty(SourceFactorAlpha);
  ZilchBindGetterSetterProperty(DestFactorAlpha);

  BlendSettings::Constructed = &GraphicsBlendSettings::ConstructedStatic;
  BlendSettings::Destructed = &GraphicsBlendSettings::DestructedStatic;
}

//**************************************************************************************************
void GraphicsBlendSettings::ConstructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->ConstructedInstance();
}

//**************************************************************************************************
void GraphicsBlendSettings::DestructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->DestructedInstance();
}

//**************************************************************************************************
void GraphicsBlendSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

//**************************************************************************************************
void GraphicsBlendSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

DefineThreadSafeReferenceCountedHandle(GraphicsDepthSettings);
//**************************************************************************************************
ZilchDefineType(GraphicsDepthSettings, builder, type)
{
  ZeroBindDocumented();
  ZeroBindThreadSafeReferenceCountedHandle();
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  //ZilchBindMethod(SetDepthRead);
  //ZilchBindMethod(SetDepthWrite);
  //ZilchBindMethod(SetStencilTestMode);
  //ZilchBindMethod(SetStencilIncrement);
  //ZilchBindMethod(SetStencilDecrement);

  ZilchBindGetterSetterProperty(DepthMode);
  ZilchBindGetterSetterProperty(DepthCompareFunc);
  ZilchBindGetterSetterProperty(StencilMode);
  ZilchBindGetterSetterProperty(StencilCompareFunc);
  ZilchBindGetterSetterProperty(StencilFailOp);
  ZilchBindGetterSetterProperty(DepthFailOp);
  ZilchBindGetterSetterProperty(DepthPassOp);
  ZilchBindGetterSetterProperty(StencilCompareFuncBackFace);
  ZilchBindGetterSetterProperty(StencilFailOpBackFace);
  ZilchBindGetterSetterProperty(DepthFailOpBackFace);
  ZilchBindGetterSetterProperty(DepthPassOpBackFace);

  ZilchBindFieldProperty(mStencilReadMask);
  ZilchBindFieldProperty(mStencilWriteMask);
  ZilchBindFieldProperty(mStencilTestValue);
  ZilchBindFieldProperty(mStencilReadMaskBackFace);
  ZilchBindFieldProperty(mStencilWriteMaskBackFace);
  ZilchBindFieldProperty(mStencilTestValueBackFace);

  DepthSettings::Constructed = &GraphicsDepthSettings::ConstructedStatic;
  DepthSettings::Destructed = &GraphicsDepthSettings::DestructedStatic;
}

//**************************************************************************************************
void GraphicsDepthSettings::ConstructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->ConstructedInstance();
}

//**************************************************************************************************
void GraphicsDepthSettings::DestructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->DestructedInstance();
}

//**************************************************************************************************
void GraphicsDepthSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

//**************************************************************************************************
void GraphicsDepthSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

//**************************************************************************************************
ZilchDefineType(GraphicsRenderSettings, builder, type)
{
  ZeroBindDocumented();
  ZilchBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  ZilchBindSetter(ColorTarget);
  ZilchBindSetter(DepthTarget);
  ZilchBindGetter(MultiRenderTarget);

  ZilchBindGetterSetter(BlendSettings);
  ZilchBindGetterSetter(DepthSettings);
  ZilchBindGetterSetter(CullMode);
  ZilchBindGetterSetter(GlobalShaderInputs);
}

//**************************************************************************************************
GraphicsRenderSettings::GraphicsRenderSettings()
{
  ClearAll();
}

//**************************************************************************************************
void GraphicsRenderSettings::ClearAll()
{
  RenderSettings::ClearAll();
  mGlobalShaderInputs = nullptr;
}

//**************************************************************************************************
void GraphicsRenderSettings::SetColorTarget(RenderTarget* target)
{
  if (target == nullptr)
  {
    mColorTextures[0] = nullptr;
    mColorTargets[0] = nullptr;
  }
  else
  {
    mColorTextures[0] = target->mTexture;
    mColorTargets[0] = target->mTexture->mRenderData;
  }
}

//**************************************************************************************************
void GraphicsRenderSettings::SetDepthTarget(RenderTarget* target)
{
  if (target == nullptr)
  {
    mDepthTexture = nullptr;
    mDepthTarget = nullptr;
  }
  else
  {
    mDepthTexture = target->mTexture;
    mDepthTarget = target->mTexture->mRenderData;
  }
}

//**************************************************************************************************
void GraphicsRenderSettings::SetColorTargetMrt(RenderTarget* target, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  if (index > 0)
    mSingleColorTarget = false;

  if (target == nullptr)
  {
    mColorTextures[index] = nullptr;
    mColorTargets[index] = nullptr;
  }
  else
  {
    mColorTextures[index] = target->mTexture;
    mColorTargets[index] = target->mTexture->mRenderData;
  }
}

//**************************************************************************************************
GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettings()
{
  {
    DoNotifyException("Error", "Invalid index. Must be 0-7.")
    return nullptr;
  }
  return (GraphicsBlendSettings*)RenderSettings::GetBlendSettings();
}

//**************************************************************************************************
void GraphicsRenderSettings::SetBlendSettings(GraphicsBlendSettings* blendSettings)
{
  RenderSettings::SetBlendSettings(blendSettings);
}

//**************************************************************************************************
GraphicsDepthSettings* GraphicsRenderSettings::GetDepthSettings()
{
  return (GraphicsDepthSettings*)RenderSettings::GetDepthSettings();
}

//**************************************************************************************************
void GraphicsRenderSettings::SetDepthSettings(GraphicsDepthSettings* depthSettings)
{
  RenderSettings::SetDepthSettings(depthSettings);
}

//**************************************************************************************************
GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettingsMrt(uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7."), nullptr;

  return (GraphicsBlendSettings*)&mBlendSettings[index];
}

//**************************************************************************************************
void GraphicsRenderSettings::SetBlendSettingsMrt(GraphicsBlendSettings* blendSettings, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  mBlendSettings[index] = *blendSettings;
}

//**************************************************************************************************
ShaderInputs* GraphicsRenderSettings::GetGlobalShaderInputs()
{
  return mGlobalShaderInputs;
}

//**************************************************************************************************
void GraphicsRenderSettings::SetGlobalShaderInputs(ShaderInputs* shaderInputs)
{
  mGlobalShaderInputs = shaderInputs;
}

//**************************************************************************************************
MultiRenderTarget* GraphicsRenderSettings::GetMultiRenderTarget()
{
  MultiRenderTarget* multiRenderTarget = new MultiRenderTarget(this);
  multiRenderTarget->mReferenceCount.mCount--;
  return multiRenderTarget;
}

//**************************************************************************************************
ZilchDefineType(ColorTargetMrt, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Set);
}

//**************************************************************************************************
void ColorTargetMrt::Set(uint index, RenderTarget* colorTarget)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetColorTargetMrt(colorTarget, index);
}

//**************************************************************************************************
ZilchDefineType(BlendSettingsMrt, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Get);
  ZilchBindMethod(Set);
}

//**************************************************************************************************
void BlendSettingsMrt::Set(uint index, GraphicsBlendSettings* blendSettings)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetBlendSettingsMrt(blendSettings, index);
}

//**************************************************************************************************
GraphicsBlendSettings* BlendSettingsMrt::Get(uint index)
{
  if (mRenderSettings.IsNull())
  {
    DoNotifyException("Error", "Attempting to call member on null object.");
    return nullptr;
  }
  return mRenderSettings->GetBlendSettingsMrt(index);
}

//**************************************************************************************************
ZilchDefineType(MultiRenderTarget, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterAs(ColorTargetMrt, "ColorTarget");
  ZilchBindSetter(ColorTarget0);
  ZilchBindSetter(ColorTarget1);
  ZilchBindSetter(ColorTarget2);
  ZilchBindSetter(ColorTarget3);
  ZilchBindSetter(ColorTarget4);
  ZilchBindSetter(ColorTarget5);
  ZilchBindSetter(ColorTarget6);
  ZilchBindSetter(ColorTarget7);

  ZilchBindGetterAs(BlendSettingsMrt, "BlendSettings");
  ZilchBindGetterSetter(BlendSettings0);
  ZilchBindGetterSetter(BlendSettings1);
  ZilchBindGetterSetter(BlendSettings2);
  ZilchBindGetterSetter(BlendSettings3);
  ZilchBindGetterSetter(BlendSettings4);
  ZilchBindGetterSetter(BlendSettings5);
  ZilchBindGetterSetter(BlendSettings6);
  ZilchBindGetterSetter(BlendSettings7);
}

//**************************************************************************************************
MultiRenderTarget::MultiRenderTarget(HandleOf<GraphicsRenderSettings> renderSettings)
  : mRenderSettings(renderSettings)
  , mColorTargetMrt(renderSettings)
  , mBlendSettingsMrt(renderSettings)
{
}

} // namespace Zero
