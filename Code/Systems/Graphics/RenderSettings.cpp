// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ShaderInputs, builder, type)
{
  RaverieBindDocumented();
  RaverieBindDestructor();

  RaverieBindMethod(Create);

  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, bool));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, int));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, IntVec2));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, IntVec3));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, IntVec4));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, float));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Vec2));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Vec3));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Vec4));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Mat3));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Mat4));
  RaverieBindOverloadedMethod(Add, RaverieInstanceOverload(void, String, String, Texture*));
  RaverieBindMethod(Remove);
  RaverieBindMethod(Clear);
}

ShaderInputs::~ShaderInputs()
{
  ErrorIf(mGuardId != cGuardId, "Expected the guard id to be set");
  mGuardId = 0;
}

HandleOf<ShaderInputs> ShaderInputs::Create()
{
  return new ShaderInputs();
}

void ShaderInputs::Add(String fragmentName, String inputName, bool input)
{
  Add(fragmentName, inputName, ShaderInputType::Bool, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, int input)
{
  Add(fragmentName, inputName, ShaderInputType::Int, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec2, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, IntVec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::IntVec4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, float input)
{
  Add(fragmentName, inputName, ShaderInputType::Float, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec2 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec2, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Vec4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Vec4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Mat3 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat3, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Mat4 input)
{
  Add(fragmentName, inputName, ShaderInputType::Mat4, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, Texture* input)
{
  Add(fragmentName, inputName, ShaderInputType::Texture, input);
}

void ShaderInputs::Add(String fragmentName, String inputName, ShaderInputType::Enum type, AnyParam value)
{
  RaverieShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

  ShaderInput shaderInput = shaderGenerator->CreateShaderInput(fragmentName, inputName, type, value);
  if (shaderInput.mShaderInputType != ShaderInputType::Invalid)
    mShaderInputs.Insert(StringPair(fragmentName, inputName), shaderInput);
}

void ShaderInputs::Remove(String fragmentName, String inputName)
{
  mShaderInputs.Erase(StringPair(fragmentName, inputName));
}

void ShaderInputs::Clear()
{
  mShaderInputs.Clear();
}

DefineThreadSafeReferenceCountedHandle(GraphicsBlendSettings);
RaverieDefineType(GraphicsBlendSettings, builder, type)
{
  RaverieBindDocumented();
  RaverieBindThreadSafeReferenceCountedHandle();
  RaverieBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  // RaverieBindMethod(SetBlendAlpha);
  // RaverieBindMethod(SetBlendAdditive);

  RaverieBindGetterSetterProperty(BlendMode);
  RaverieBindGetterSetterProperty(BlendEquation);
  RaverieBindGetterSetterProperty(SourceFactor);
  RaverieBindGetterSetterProperty(DestFactor);
  RaverieBindGetterSetterProperty(BlendEquationAlpha);
  RaverieBindGetterSetterProperty(SourceFactorAlpha);
  RaverieBindGetterSetterProperty(DestFactorAlpha);

  BlendSettings::Constructed = &GraphicsBlendSettings::ConstructedStatic;
  BlendSettings::Destructed = &GraphicsBlendSettings::DestructedStatic;
}

void GraphicsBlendSettings::ConstructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->ConstructedInstance();
}

void GraphicsBlendSettings::DestructedStatic(BlendSettings* settings)
{
  ((GraphicsBlendSettings*)settings)->DestructedInstance();
}

void GraphicsBlendSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

void GraphicsBlendSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

DefineThreadSafeReferenceCountedHandle(GraphicsDepthSettings);
RaverieDefineType(GraphicsDepthSettings, builder, type)
{
  RaverieBindDocumented();
  RaverieBindThreadSafeReferenceCountedHandle();
  RaverieBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  // Will probably remove these functions, so don't want them bound
  // RaverieBindMethod(SetDepthRead);
  // RaverieBindMethod(SetDepthWrite);
  // RaverieBindMethod(SetStencilTestMode);
  // RaverieBindMethod(SetStencilIncrement);
  // RaverieBindMethod(SetStencilDecrement);

  RaverieBindGetterSetterProperty(DepthMode);
  RaverieBindGetterSetterProperty(DepthCompareFunc);
  RaverieBindGetterSetterProperty(StencilMode);
  RaverieBindGetterSetterProperty(StencilCompareFunc);
  RaverieBindGetterSetterProperty(StencilFailOp);
  RaverieBindGetterSetterProperty(DepthFailOp);
  RaverieBindGetterSetterProperty(DepthPassOp);
  RaverieBindGetterSetterProperty(StencilCompareFuncBackFace);
  RaverieBindGetterSetterProperty(StencilFailOpBackFace);
  RaverieBindGetterSetterProperty(DepthFailOpBackFace);
  RaverieBindGetterSetterProperty(DepthPassOpBackFace);

  RaverieBindFieldProperty(mStencilReadMask);
  RaverieBindFieldProperty(mStencilWriteMask);
  RaverieBindFieldProperty(mStencilTestValue);
  RaverieBindFieldProperty(mStencilReadMaskBackFace);
  RaverieBindFieldProperty(mStencilWriteMaskBackFace);
  RaverieBindFieldProperty(mStencilTestValueBackFace);

  DepthSettings::Constructed = &GraphicsDepthSettings::ConstructedStatic;
  DepthSettings::Destructed = &GraphicsDepthSettings::DestructedStatic;
}

void GraphicsDepthSettings::ConstructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->ConstructedInstance();
}

void GraphicsDepthSettings::DestructedStatic(DepthSettings* settings)
{
  ((GraphicsDepthSettings*)settings)->DestructedInstance();
}

void GraphicsDepthSettings::ConstructedInstance()
{
  ConstructThreadSafeReferenceCountedHandle();
}

void GraphicsDepthSettings::DestructedInstance()
{
  DestructThreadSafeReferenceCountedHandle();
}

RaverieDefineType(GraphicsRenderSettings, builder, type)
{
  RaverieBindDocumented();
  RaverieBindDefaultCopyDestructor();
  type->CreatableInScript = true;

  RaverieBindSetter(ColorTarget);
  RaverieBindSetter(DepthTarget);
  RaverieBindGetter(MultiRenderTarget);

  RaverieBindGetterSetter(BlendSettings);
  RaverieBindGetterSetter(DepthSettings);
  RaverieBindGetterSetter(CullMode);
  RaverieBindGetterSetter(GlobalShaderInputs);
}

GraphicsRenderSettings::GraphicsRenderSettings()
{
  ClearAll();
}

void GraphicsRenderSettings::ClearAll()
{
  RenderSettings::ClearAll();
  mGlobalShaderInputs = nullptr;
}

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

GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettings()
{
  return (GraphicsBlendSettings*)RenderSettings::GetBlendSettings();
}

void GraphicsRenderSettings::SetBlendSettings(GraphicsBlendSettings* blendSettings)
{
  RenderSettings::SetBlendSettings(blendSettings);
}

GraphicsDepthSettings* GraphicsRenderSettings::GetDepthSettings()
{
  return (GraphicsDepthSettings*)RenderSettings::GetDepthSettings();
}

void GraphicsRenderSettings::SetDepthSettings(GraphicsDepthSettings* depthSettings)
{
  RenderSettings::SetDepthSettings(depthSettings);
}

GraphicsBlendSettings* GraphicsRenderSettings::GetBlendSettingsMrt(uint index)
{
  if (index >= 8)
  {
    DoNotifyException("Error", "Invalid index. Must be 0-7.");
    return nullptr;
  }

  return (GraphicsBlendSettings*)&mBlendSettings[index];
}

void GraphicsRenderSettings::SetBlendSettingsMrt(GraphicsBlendSettings* blendSettings, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  mBlendSettings[index] = *blendSettings;
}

ShaderInputs* GraphicsRenderSettings::GetGlobalShaderInputs()
{
  return mGlobalShaderInputs;
}

void GraphicsRenderSettings::SetGlobalShaderInputs(ShaderInputs* shaderInputs)
{
  mGlobalShaderInputs = shaderInputs;
}

MultiRenderTarget* GraphicsRenderSettings::GetMultiRenderTarget()
{
  MultiRenderTarget* multiRenderTarget = new MultiRenderTarget(this);
  multiRenderTarget->mReferenceCount.mCount--;
  return multiRenderTarget;
}

RaverieDefineType(ColorTargetMrt, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMethod(Set);
}

void ColorTargetMrt::Set(uint index, RenderTarget* colorTarget)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetColorTargetMrt(colorTarget, index);
}

RaverieDefineType(BlendSettingsMrt, builder, type)
{
  RaverieBindDocumented();

  RaverieBindMethod(Get);
  RaverieBindMethod(Set);
}

void BlendSettingsMrt::Set(uint index, GraphicsBlendSettings* blendSettings)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetBlendSettingsMrt(blendSettings, index);
}

GraphicsBlendSettings* BlendSettingsMrt::Get(uint index)
{
  if (mRenderSettings.IsNull())
  {
    DoNotifyException("Error", "Attempting to call member on null object.");
    return nullptr;
  }
  return mRenderSettings->GetBlendSettingsMrt(index);
}

RaverieDefineType(MultiRenderTarget, builder, type)
{
  RaverieBindDocumented();

  RaverieBindGetterAs(ColorTargetMrt, "ColorTarget");
  RaverieBindSetter(ColorTarget0);
  RaverieBindSetter(ColorTarget1);
  RaverieBindSetter(ColorTarget2);
  RaverieBindSetter(ColorTarget3);
  RaverieBindSetter(ColorTarget4);
  RaverieBindSetter(ColorTarget5);
  RaverieBindSetter(ColorTarget6);
  RaverieBindSetter(ColorTarget7);

  RaverieBindGetterAs(BlendSettingsMrt, "BlendSettings");
  RaverieBindGetterSetter(BlendSettings0);
  RaverieBindGetterSetter(BlendSettings1);
  RaverieBindGetterSetter(BlendSettings2);
  RaverieBindGetterSetter(BlendSettings3);
  RaverieBindGetterSetter(BlendSettings4);
  RaverieBindGetterSetter(BlendSettings5);
  RaverieBindGetterSetter(BlendSettings6);
  RaverieBindGetterSetter(BlendSettings7);
}

MultiRenderTarget::MultiRenderTarget(HandleOf<GraphicsRenderSettings> renderSettings) : mRenderSettings(renderSettings), mColorTargetMrt(renderSettings), mBlendSettingsMrt(renderSettings)
{
}

} // namespace Raverie
