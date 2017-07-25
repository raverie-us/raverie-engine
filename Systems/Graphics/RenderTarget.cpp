#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SamplerSettings, builder, type)
{
  type->CreatableInScript = true;
  ZilchBindFieldProperty(mAddressingX);
  ZilchBindFieldProperty(mAddressingY);
  ZilchBindFieldProperty(mFiltering);
  ZilchBindFieldProperty(mCompareMode);
  ZilchBindFieldProperty(mCompareFunc);

  ZilchBindDestructor();
  ZilchBindConstructor();
}

SamplerSettings::SamplerSettings()
  : mAddressingX(TextureAddressing::Clamp)
  , mAddressingY(TextureAddressing::Clamp)
  , mFiltering(TextureFiltering::Nearest)
  , mCompareMode(TextureCompareMode::Disabled)
  , mCompareFunc(TextureCompareFunc::Never)
{
}

u32 SamplerSettings::GetSettings()
{
  u32 settings = 0;
  settings |= AddressingX(mAddressingX);
  settings |= AddressingY(mAddressingY);
  settings |= Filtering(mFiltering);
  settings |= CompareMode(mCompareMode);
  settings |= CompareFunc(mCompareFunc);
  return settings;
}

u32 SamplerSettings::AddressingX(TextureAddressing::Enum addressingX)
{
  return (0x08 | (u32)addressingX) << 0;
}

u32 SamplerSettings::AddressingY(TextureAddressing::Enum addressingY)
{
  return (0x08 | (u32)addressingY) << 4;
}

u32 SamplerSettings::Filtering(TextureFiltering::Enum filtering)
{
  return (0x08 | (u32)filtering) << 8;
}

u32 SamplerSettings::CompareMode(TextureCompareMode::Enum compareMode)
{
  return (0x08 | (u32)compareMode) << 12;
}

u32 SamplerSettings::CompareFunc(TextureCompareFunc::Enum compareFunc)
{
  return (0x08 | (u32)compareFunc) << 16;
}

TextureAddressing::Enum SamplerSettings::AddressingX(u32 samplerSettings)
{
  return (TextureAddressing::Enum)((samplerSettings & 0x00000007) >> 0);
}

TextureAddressing::Enum SamplerSettings::AddressingY(u32 samplerSettings)
{
  return (TextureAddressing::Enum)((samplerSettings & 0x00000070) >> 4);
}

TextureFiltering::Enum SamplerSettings::Filtering(u32 samplerSettings)
{
  return (TextureFiltering::Enum)((samplerSettings & 0x00000700) >> 8);
}

TextureCompareMode::Enum SamplerSettings::CompareMode(u32 samplerSettings)
{
  return (TextureCompareMode::Enum)((samplerSettings & 0x00007000) >> 12);
}

TextureCompareFunc::Enum SamplerSettings::CompareFunc(u32 samplerSettings)
{
  return (TextureCompareFunc::Enum)((samplerSettings & 0x00070000) >> 16);
}

void SamplerSettings::AddValue(u32& samplerSettings, u32 value)
{
  // If value has been set already then the extra check bit will overlap
  if ((samplerSettings & value) != 0)
    return;

  samplerSettings |= value;
}

void SamplerSettings::FillDefaults(u32& samplerSettings, u32 defaultSettings)
{
  // Each value from defaults is multiplied by whether or not the check bit is already present in the settings
  // If settings already has a particular value set then 0 is or'd, resulting in no change
  samplerSettings |= (defaultSettings & 0x0000000F) * (u32)((samplerSettings & 0x00000008) == 0);
  samplerSettings |= (defaultSettings & 0x000000F0) * (u32)((samplerSettings & 0x00000080) == 0);
  samplerSettings |= (defaultSettings & 0x00000F00) * (u32)((samplerSettings & 0x00000800) == 0);
  samplerSettings |= (defaultSettings & 0x0000F000) * (u32)((samplerSettings & 0x00008000) == 0);
  samplerSettings |= (defaultSettings & 0x000F0000) * (u32)((samplerSettings & 0x00080000) == 0);
}

ZilchDefineType(RenderTarget, builder, type)
{
  ZilchBindFieldGetter(mTexture);
  ZilchBindMethod(Release);
}

RenderTarget::RenderTarget(RenderTargetManager* manager)
  : mManager(manager)
{
}

RenderTarget::~RenderTarget()
{
}

void RenderTarget::Release()
{
  if (mTexture->mProtected == false)
    return DoNotifyException("Error", "Cannot release RenderTargets that use runtime Textures.");

  mManager->ClearRenderTarget(this);
}

void RenderTargetManager::Shutdown()
{
  forRange (RenderTarget* renderTarget, mRenderTargets.All())
    delete renderTarget;
  mRenderTargets.Clear();

  mUsedTextures.Clear();
  mAvailableTextures.Clear();
  mUnusedTextures.Clear();
}

HandleOf<RenderTarget> RenderTargetManager::GetRenderTarget(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings)
{
  RenderTarget* renderTarget = new RenderTarget(this);
  mRenderTargets.PushBack(renderTarget);

  u64 lookupId = MakeLookupId(width, height, format, samplerSettings);

  // Check for a reusable texture
  renderTarget->mTexture = FindTexture(lookupId, mAvailableTextures);

  // Check for a reusable texture
  if (renderTarget->mTexture == nullptr)
    renderTarget->mTexture = FindTexture(lookupId, mUnusedTextures);

  // Make new texture if needed
  if (renderTarget->mTexture == nullptr)
  {
    renderTarget->mTexture = Texture::CreateRuntime();
    RenderTargetTexture renderTargetTexture;
    renderTargetTexture.mLookupId = lookupId;
    renderTargetTexture.mTexture = renderTarget->mTexture;
    mUsedTextures.PushBack(renderTargetTexture);

    Texture* texture = renderTarget->mTexture;
    texture->mAddressingX = samplerSettings.mAddressingX;
    texture->mAddressingY = samplerSettings.mAddressingY;
    texture->mFiltering = samplerSettings.mFiltering;
    texture->mCompareMode = samplerSettings.mCompareMode;
    texture->mCompareFunc = samplerSettings.mCompareFunc;
    texture->Upload(width, height, format, nullptr, 0, false);
    // Do not allow modification of textures owned by RenderTargetManager
    texture->mProtected = true;
  }

  return renderTarget;
}

HandleOf<RenderTarget> RenderTargetManager::GetRenderTarget(HandleOf<Texture> texture)
{
  if (texture == nullptr)
    return DoNotifyException("Error", "Null Texture."), nullptr;

  if (texture->mProtected)
    return DoNotifyException("Error", "Cannot create RenderTarget from a non-runtime Texture."), nullptr;

  RenderTarget* renderTarget = new RenderTarget(this);
  mRenderTargets.PushBack(renderTarget);

  // Texture managed elsewhere, just need a handle to it
  renderTarget->mTexture = texture;

  return renderTarget;
}

void RenderTargetManager::ClearRenderTarget(RenderTarget* renderTarget)
{
  for (uint i = 0; i < mUsedTextures.Size(); ++i)
  {
    if (mUsedTextures[i].mTexture == renderTarget->mTexture)
    {
      mAvailableTextures[mUsedTextures[i].mLookupId].PushBack(mUsedTextures[i]);
      mUsedTextures.EraseAt(i);
      mRenderTargets.EraseValue(renderTarget);
      delete renderTarget;
      return;
    }
  }

  Error("RenderTarget's texture was not found in used list.");
}

void RenderTargetManager::ClearRenderTargets()
{
  // Delete RenderTargets to invalidate any handles that might be held in script
  forRange (RenderTarget* renderTarget, mRenderTargets.All())
    delete renderTarget;
  mRenderTargets.Clear();

  // Move textures to available list
  forRange (RenderTargetTexture& renderTargetTexture, mUsedTextures.All())
    mAvailableTextures[renderTargetTexture.mLookupId].PushBack(renderTargetTexture);
  mUsedTextures.Clear();
}

void RenderTargetManager::ClearUnusedTextures()
{
  mUnusedTextures.Clear();
  mUnusedTextures = mAvailableTextures;
  mAvailableTextures.Clear();
}

u64 RenderTargetManager::MakeLookupId(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings)
{
  u64 lookupId = 0;
  // width and height cannot exceed 65535
  lookupId |= (u64)width;
  lookupId |= (u64)height << 16;
  lookupId |= (u64)format << 32;
  // sampler settings do not exceed 24 bits used
  lookupId |= (u64)samplerSettings.GetSettings() << 40;
  return lookupId;
}

HandleOf<Texture> RenderTargetManager::FindTexture(u64 lookupId, RenderTargetTextureMap& textureMap)
{
  if (textureMap.ContainsKey(lookupId))
  {
    RenderTargetTextureList& textures = textureMap[lookupId];
    RenderTargetTexture renderTargetTexture = textures.Back();
    textures.PopBack();
    // Remove empty lists so there's always an entry if map lookup succeeds
    if (textures.Empty())
      textureMap.Erase(lookupId);
    mUsedTextures.PushBack(renderTargetTexture);
    return renderTargetTexture.mTexture;
  }

  return nullptr;
}

} // namespace Zero
