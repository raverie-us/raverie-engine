#pragma once

namespace Zero
{

/// Used when requesting a RenderTarget to configure how its texture is sampled.
class SamplerSettings
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SamplerSettings();

  /// How to treat uv coordinates outside of [0, 1] along the Texture's width.
  TextureAddressing::Enum mAddressingX;
  /// How to treat uv coordinates outside of [0, 1] along the Texture's height.
  TextureAddressing::Enum mAddressingY;
  /// How samples should be blended under minification/magnification.
  TextureFiltering::Enum mFiltering;
  /// If sampling in hardware should perform comparison instead of fetching. Requires using SamplerShadow2d in the shader.
  TextureCompareMode::Enum mCompareMode;
  /// Which method of comparison should be used if CompareMode is set to Enable.
  TextureCompareFunc::Enum mCompareFunc;

  // Internal

  // Converts all settings on current object to one compact integer
  u32 GetSettings();

  // Compacts values that can be binary or'd together, used for hashing
  // Converted values use an extra bit so that unused values can be detected
  static u32 AddressingX(TextureAddressing::Enum addressingX);
  static u32 AddressingY(TextureAddressing::Enum addressingY);
  static u32 Filtering(TextureFiltering::Enum filtering);
  static u32 CompareMode(TextureCompareMode::Enum compareMode);
  static u32 CompareFunc(TextureCompareFunc::Enum compareFunc);

  // Retrieves individual enum values from the compacted integer
  // Results in 0 if the requested value was never set
  static TextureAddressing::Enum AddressingX(u32 samplerSettings);
  static TextureAddressing::Enum AddressingY(u32 samplerSettings);
  static TextureFiltering::Enum Filtering(u32 samplerSettings);
  static TextureCompareMode::Enum CompareMode(u32 samplerSettings);
  static TextureCompareFunc::Enum CompareFunc(u32 samplerSettings);

  // Adds a converted value only if it hasn't been set already
  static void AddValue(u32& samplerSettings, u32 value);
  // Uses values from defaults to fill any unset values
  static void FillDefaults(u32& samplerSettings, u32 defaultSettings);
};

/// Interface for rendering output. Texture data is managed and recycled by the engine.
class RenderTarget : public SafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Texture that is rendered to. Can be used as shader input to a separate rendering operation.
  HandleOf<Texture> mTexture;

  /// Allows the managed Texture being referenced by this RenderTarget to be reused by the renderer
  /// if the same specifications are requested again. Also deletes this RenderTarget.
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

  HandleOf<RenderTarget> GetRenderTarget(uint width, uint height, TextureFormat::Enum format, SamplerSettings samplerSettings);
  HandleOf<RenderTarget> GetRenderTarget(HandleOf<Texture> texture);

  // Deletes RenderTarget and makes its texture available for reuse
  void ClearRenderTarget(RenderTarget* renderTarget);
  // Deletes all RenderTargets and makes textures available for reuse
  void ClearRenderTargets();
  // Releases all Textures that were not reused since last called
  void ClearUnusedTextures();

private:
  typedef Array<RenderTargetTexture> RenderTargetTextureList;
  typedef HashMap< u64, Array<RenderTargetTexture> > RenderTargetTextureMap;

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

} // namespace Zero
