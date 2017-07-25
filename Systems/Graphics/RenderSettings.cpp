#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ShaderInputs, builder, type)
{
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
  ZilchShaderGenerator* shaderGenerator = Z::gEngine->has(GraphicsEngine)->mShaderGenerator;

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

DefineThreadSafeReferenceCountedHandle(BlendSettings);
ZilchDefineType(BlendSettings, builder, type)
{
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
}

BlendSettings::BlendSettings()
{
  ConstructThreadSafeReferenceCountedHandle();

  mBlendMode = BlendMode::Disabled;
  mBlendEquation = BlendEquation::Add;
  mSourceFactor = BlendFactor::Zero;
  mDestFactor = BlendFactor::Zero;
  mBlendEquationAlpha = BlendEquation::Add;
  mSourceFactorAlpha = BlendFactor::Zero;
  mDestFactorAlpha = BlendFactor::Zero;
}

BlendSettings::BlendSettings(const BlendSettings& other)
{
  ConstructThreadSafeReferenceCountedHandle();
  *this = other;
}

BlendSettings::~BlendSettings()
{
  DestructThreadSafeReferenceCountedHandle();
}

void BlendSettings::SetBlendAlpha()
{
  mBlendMode = BlendMode::Separate;
  mSourceFactor = BlendFactor::SourceAlpha;
  mDestFactor = BlendFactor::InvSourceAlpha;
  mSourceFactorAlpha = BlendFactor::One;
  mDestFactorAlpha = BlendFactor::One;
}

void BlendSettings::SetBlendAdditive()
{
  mBlendMode = BlendMode::Enabled;
  mSourceFactor = BlendFactor::SourceAlpha;
  mDestFactor = BlendFactor::One;
}

DefineThreadSafeReferenceCountedHandle(DepthSettings);
ZilchDefineType(DepthSettings, builder, type)
{
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
}

DepthSettings::DepthSettings()
{
  ConstructThreadSafeReferenceCountedHandle();

  mDepthMode = DepthMode::Disabled;
  mDepthCompareFunc = TextureCompareFunc::Never;

  mStencilMode = StencilMode::Disabled;
  mStencilCompareFunc = TextureCompareFunc::Never;
  mStencilFailOp = StencilOp::Zero;
  mDepthFailOp = StencilOp::Zero;
  mDepthPassOp = StencilOp::Zero;
  mStencilReadMask = 0xFF;
  mStencilWriteMask = 0xFF;
  mStencilTestValue = 0;

  mStencilCompareFuncBackFace = TextureCompareFunc::Never;
  mStencilFailOpBackFace = StencilOp::Zero;
  mDepthFailOpBackFace = StencilOp::Zero;
  mDepthPassOpBackFace = StencilOp::Zero;
  mStencilReadMaskBackFace = 0xFF;
  mStencilWriteMaskBackFace = 0xFF;
  mStencilTestValueBackFace = 0;
}

DepthSettings::DepthSettings(const DepthSettings& other)
{
  ConstructThreadSafeReferenceCountedHandle();
  *this = other;
}

DepthSettings::~DepthSettings()
{
  DestructThreadSafeReferenceCountedHandle();
}

void DepthSettings::SetDepthRead(TextureCompareFunc::Enum depthCompareFunc)
{
  mDepthMode = DepthMode::Read;
  mDepthCompareFunc = depthCompareFunc;
}

void DepthSettings::SetDepthWrite(TextureCompareFunc::Enum depthCompareFunc)
{
  mDepthMode = DepthMode::Write;
  mDepthCompareFunc = depthCompareFunc;
}

void DepthSettings::SetStencilTestMode(TextureCompareFunc::Enum stencilCompareFunc)
{
  mStencilMode = StencilMode::Enabled;
  mStencilCompareFunc = stencilCompareFunc;
  mStencilFailOp = StencilOp::Keep;
  mDepthFailOp = StencilOp::Keep;
  mDepthPassOp = StencilOp::Keep;
}

void DepthSettings::SetStencilIncrement()
{
  mStencilMode = StencilMode::Enabled;
  mStencilCompareFunc = TextureCompareFunc::Always;
  mStencilFailOp = StencilOp::Keep;
  mDepthFailOp = StencilOp::Keep;
  mDepthPassOp = StencilOp::Increment;
}

void DepthSettings::SetStencilDecrement()
{
  mStencilMode = StencilMode::Enabled;
  mStencilCompareFunc = TextureCompareFunc::Always;
  mStencilFailOp = StencilOp::Keep;
  mDepthFailOp = StencilOp::Keep;
  mDepthPassOp = StencilOp::Decrement;
}

ZilchDefineType(RenderSettings, builder, type)
{
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

RenderSettings::RenderSettings()
{
  ClearAll();
}

void RenderSettings::ClearAll()
{
  ClearTargets();
  ClearSettings();
  mCullMode = CullMode::Disabled;
  mScissorMode = ScissorMode::Disabled;
  mGlobalShaderInputs = nullptr;
}

void RenderSettings::ClearTargets()
{
  mTargetsWidth = 0;
  mTargetsHeight = 0;

  for (uint i = 0; i < 8; ++i)
  {
    mColorTextures[i] = nullptr;
    mColorTargets[i] = nullptr;
    mBlendSettings[i] = BlendSettings();
  }

  mDepthTexture = nullptr;
  mDepthTarget = nullptr;
  mDepthSettings = DepthSettings();

  mSingleColorTarget = true;
}

void RenderSettings::ClearSettings()
{
  for (uint i = 0; i < 8; ++i)
    mBlendSettings[i] = BlendSettings();

  mDepthSettings = DepthSettings();
}

void RenderSettings::SetColorTarget(RenderTarget* target)
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

void RenderSettings::SetDepthTarget(RenderTarget* target)
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

void RenderSettings::SetColorTargetMrt(RenderTarget* target, uint index)
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

BlendSettings* RenderSettings::GetBlendSettingsMrt(uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7."), nullptr;

  return &mBlendSettings[index];
}

void RenderSettings::SetBlendSettingsMrt(BlendSettings* blendSettings, uint index)
{
  if (index >= 8)
    return DoNotifyException("Error", "Invalid index. Must be 0-7.");

  mBlendSettings[index] = *blendSettings;
}

BlendSettings* RenderSettings::GetBlendSettings()
{
  return mBlendSettings;
}

void RenderSettings::SetBlendSettings(BlendSettings* blendSettings)
{
  mBlendSettings[0] = *blendSettings;
}

DepthSettings* RenderSettings::GetDepthSettings()
{
  return &mDepthSettings;
}

void RenderSettings::SetDepthSettings(DepthSettings* depthSettings)
{
  mDepthSettings = *depthSettings;
}

ShaderInputs* RenderSettings::GetGlobalShaderInputs()
{
  return mGlobalShaderInputs;
}

void RenderSettings::SetGlobalShaderInputs(ShaderInputs* shaderInputs)
{
  mGlobalShaderInputs = shaderInputs;
}

MultiRenderTarget* RenderSettings::GetMultiRenderTarget()
{
  MultiRenderTarget* multiRenderTarget = new MultiRenderTarget(this);
  multiRenderTarget->mReferenceCount.mCount--;
  return multiRenderTarget;
}

ZilchDefineType(ColorTargetMrt, builder, type)
{
  ZilchBindMethod(Set);
}

void ColorTargetMrt::Set(uint index, RenderTarget* colorTarget)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetColorTargetMrt(colorTarget, index);
}

ZilchDefineType(BlendSettingsMrt, builder, type)
{
  ZilchBindMethod(Get);
  ZilchBindMethod(Set);
}

void BlendSettingsMrt::Set(uint index, BlendSettings* blendSettings)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object.");
  mRenderSettings->SetBlendSettingsMrt(blendSettings, index);
}

BlendSettings* BlendSettingsMrt::Get(uint index)
{
  if (mRenderSettings.IsNull())
    return DoNotifyException("Error", "Attempting to call member on null object."), nullptr;
  return mRenderSettings->GetBlendSettingsMrt(index);
}

ZilchDefineType(MultiRenderTarget, builder, type)
{
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

MultiRenderTarget::MultiRenderTarget(HandleOf<RenderSettings> renderSettings)
  : mRenderSettings(renderSettings)
  , mColorTargetMrt(renderSettings)
  , mBlendSettingsMrt(renderSettings)
{
}

} // namespace Zero
