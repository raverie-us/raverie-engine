// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineExternalBaseType(SamplerSettings, TypeCopyMode::ReferenceType, builder, type)
{
  RaverieBindDocumented();

  type->CreatableInScript = true;
  RaverieBindFieldProperty(mAddressingX);
  RaverieBindFieldProperty(mAddressingY);
  RaverieBindFieldProperty(mFiltering);
  RaverieBindFieldProperty(mCompareMode);
  RaverieBindFieldProperty(mCompareFunc);

  RaverieBindDestructor();
  RaverieBindConstructor();
}

RaverieDefineType(RenderTarget, builder, type)
{
  RaverieBindDocumented();

  RaverieBindFieldGetter(mTexture);
  RaverieBindMethod(Release);
}

RenderTarget::RenderTarget(RenderTargetManager* manager) : mManager(manager)
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
  {
    DoNotifyException("Error", "Null Texture.");
    return nullptr;
  }

  if (texture->mProtected)
  {
    DoNotifyException("Error", "Cannot create RenderTarget from a non-runtime Texture.");
    return nullptr;
  }
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

} // namespace Raverie
