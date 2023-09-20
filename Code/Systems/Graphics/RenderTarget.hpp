// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

/// Interface for rendering output. Texture data is managed and recycled by the
/// engine.
class RenderTarget : public SafeId32
{
public:
  RaverieDeclareType(RenderTarget, TypeCopyMode::ReferenceType);

  /// Texture that is rendered to. Can be used as shader input to a separate
  /// rendering operation.
  HandleOf<Texture> mTexture;

  /// Allows the managed Texture being referenced by this RenderTarget to be
  /// reused by the renderer if the same specifications are requested again.
  /// Also deletes this RenderTarget.
  void Release();

private:
  // Should only be created/destroyed by manager
  RenderTarget(RenderTargetManager* manager);
  ~RenderTarget();
  friend class RenderTargetManager;

  RenderTargetManager* mManager;
};

// Used to quickly find reusable textures
class RenderTargetTexture
{
public:
  u64 mLookupId;
  HandleOf<Texture> mTexture;
};

class RenderTargetManager
{
public:
  void Shutdown();

  HandleOf<RenderTarget>
  GetRenderTarget(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings);
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);

  // Deletes RenderTarget and makes its texture available for reuse
  void ClearRenderTarget(RenderTarget* renderTarget);
  // Deletes all RenderTargets and makes textures available for reuse
  void ClearRenderTargets();
  // Releases all Textures that were not reused since last called
  void ClearUnusedTextures();

private:
  typedef Array<RenderTargetTexture> RenderTargetTextureList;
  typedef HashMap<u64, Array<RenderTargetTexture>> RenderTargetTextureMap;

  u64 MakeLookupId(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings);
  HandleOf<Texture> FindTexture(u64 lookupId, RenderTargetTextureMap& textureMap);

  Array<RenderTarget*> mRenderTargets;

  // Textures currently in use by RenderTargets
  RenderTargetTextureList mUsedTextures;
  // Textures that were used for the current frame but can now be reused
  RenderTargetTextureMap mAvailableTextures;
  // Textures that did not get reused before next ClearUnusedTextures call
  RenderTargetTextureMap mUnusedTextures;
};

} // namespace Raverie
