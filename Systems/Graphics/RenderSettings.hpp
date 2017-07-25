#pragma once

namespace Zero
{

/// A set of shader inputs for overriding values per object or globally.
class ShaderInputs : public ReferenceCountedThreadSafeId32
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, bool input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, int input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, IntVec2 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, IntVec3 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, IntVec4 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, float input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Vec2 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Vec3 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Vec4 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Mat3 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Mat4 input);
  /// Add an input value for a specific fragment.
  void Add(String fragmentName, String inputName, Texture* input);

  /// Remove a specific input that was added.
  void Remove(String fragmentName, String inputName);

  /// Remove all added inputs.
  void Clear();

  // Internally used by all the overloads.
  void Add(String fragmentName, String inputName, ShaderInputType::Enum type, AnyParam value);

  // Map of fragmentName and inputName to inputs.
  typedef Pair<String, String> StringPair;
  HashMap<StringPair, ShaderInput> mShaderInputs;
};

/// Settings for how pixel shader outputs are combined with the RenderTarget's current values.
class BlendSettings
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareThreadSafeReferenceCountedHandle(BlendSettings);

  BlendSettings();
  BlendSettings(const BlendSettings& other);
  ~BlendSettings();

  void SetBlendAlpha();
  void SetBlendAdditive();

  // TODO: Macro comments for auto-doc
  /// If blend equations should be applied to output.
  DeclareByteEnumGetSet(BlendMode::Enum, BlendMode);
  /// How source and destination values should be combined.
  DeclareByteEnumGetSet(BlendEquation::Enum, BlendEquation);
  /// What source value should be multiplied with before combined.
  DeclareByteEnumGetSet(BlendFactor::Enum, SourceFactor);
  /// What destination value should be multiplied with before combined.
  DeclareByteEnumGetSet(BlendFactor::Enum, DestFactor);

  // Separable color/alpha settings
  /// How source and destination values should be combined, for alpha channel if in separate mode.
  DeclareByteEnumGetSet(BlendEquation::Enum, BlendEquationAlpha);
  /// What source value should be multiplied with before combined, for alpha channel if in separate mode.
  DeclareByteEnumGetSet(BlendFactor::Enum, SourceFactorAlpha);
  /// What destination value should be multiplied with before combined, for alpha channel if in separate mode.
  DeclareByteEnumGetSet(BlendFactor::Enum, DestFactorAlpha);
};

/// Settings for how the depth buffer should control pixel output.
class DepthSettings
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  DeclareThreadSafeReferenceCountedHandle(DepthSettings);

  DepthSettings();
  DepthSettings(const DepthSettings& other);
  ~DepthSettings();

  void SetDepthRead(TextureCompareFunc::Enum depthCompareFunc);
  void SetDepthWrite(TextureCompareFunc::Enum depthCompareFunc);

  void SetStencilTestMode(TextureCompareFunc::Enum stencilCompareFunc);
  void SetStencilIncrement();
  void SetStencilDecrement();

  // TODO: Macro comments for auto-doc
  // Depth settings
  /// If pixel depth should pass comparison to a depth buffer in order to output.
  /// And if value should be written to the depth buffer when comparison passes.
  DeclareByteEnumGetSet(DepthMode::Enum, DepthMode);
  /// Comparison function for depth test.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, DepthCompareFunc);

  // Stencil settings
  /// If pixels should also pass a comparison with the stencil buffer in order to output.
  DeclareByteEnumGetSet(StencilMode::Enum, StencilMode);
  /// Comparison function for stencil test.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, StencilCompareFunc);
  /// Operation to perform on stencil value if stencil test fails.
  DeclareByteEnumGetSet(StencilOp::Enum, StencilFailOp);
  /// Operation to perform on stencil value if stencil test passes but depth test fails.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthFailOp);
  /// Operation to perform on stencil value if both stencil and depth tests pass.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthPassOp);
  /// Bit mask for buffer value and test value when being compared.
  byte mStencilReadMask;
  /// Bit mask for which bits in the buffer can be modified.
  byte mStencilWriteMask;
  /// Value that will be used to compare against the stencil buffer for all pixels.
  byte mStencilTestValue;

  // If separable front/back face settings are desired
  /// Comparison function for stencil test, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(TextureCompareFunc::Enum, StencilCompareFuncBackFace);
  /// Operation to perform on stencil value if stencil test fails, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, StencilFailOpBackFace);
  /// Operation to perform on stencil value if stencil test passes but depth test fails, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthFailOpBackFace);
  /// Operation to perform on stencil value if both stencil and depth tests pass, for triangle back faces if in separate mode.
  DeclareByteEnumGetSet(StencilOp::Enum, DepthPassOpBackFace);
  /// Bit mask for buffer value and test value when being compared, for triangle back faces if in separate mode.
  byte mStencilReadMaskBackFace;
  /// Bit mask for which bits in the buffer can be modified, for triangle back faces if in separate mode.
  byte mStencilWriteMaskBackFace;
  /// Value that will be used to compare against the stencil buffer for all pixels, for triangle back faces if in separate mode.
  byte mStencilTestValueBackFace;
};

/// Contains all output targets and render settings needed for a render task.
class RenderSettings
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RenderSettings();

  /// The RenderTarget of a color format to output to, implicitly RenderTarget0.
  void SetColorTarget(RenderTarget* target);

  /// The RenderTarget of a depth format to use as a depth buffer for depth/stencil testing.
  void SetDepthTarget(RenderTarget* target);

  /// Settings to use when blending shader output with the ColorTarget, implicitly BlendSettings0.
  BlendSettings* GetBlendSettings();
  void SetBlendSettings(BlendSettings* blendSettings);

  /// Settings to use when doing depth/stencil testing with DepthTarget.
  DepthSettings* GetDepthSettings();
  void SetDepthSettings(DepthSettings* depthSettings);

  /// Shader input values to be globally overridden for all objects/shaders.
  ShaderInputs* GetGlobalShaderInputs();
  void SetGlobalShaderInputs(ShaderInputs* shaderInputs);
  HandleOf<ShaderInputs> mGlobalShaderInputs;

  // Do not call from C++, unless you delete the returned pointer yourself.
  /// Interface for configuring multiple color target outputs.
  MultiRenderTarget* GetMultiRenderTarget();

  // Helpers for MultiRenderTarget interface.
  void SetColorTargetMrt(RenderTarget* target, uint location);
  BlendSettings* GetBlendSettingsMrt(uint location);
  void SetBlendSettingsMrt(BlendSettings* blendSettings, uint location);

  // Clears all targets and settings.
  void ClearAll();
  // Clears all set targets.
  void ClearTargets();
  // Clears all blend and depth settings.
  void ClearSettings();

  uint mTargetsWidth;
  uint mTargetsHeight;

  // Texture pointers needed for validation and update check.
  Texture* mColorTextures[8];
  Texture* mDepthTexture;

  // Render data pointers.
  TextureRenderData* mColorTargets[8];
  TextureRenderData* mDepthTarget;

  BlendSettings mBlendSettings[8];
  DepthSettings mDepthSettings;

  bool mSingleColorTarget;

  // TODO: Macro comments for auto-doc
  /// If a certain side of triangles should not be rendered. Front faces defined by counter-clockwise vertex winding.
  DeclareByteEnumGetSet(CullMode::Enum, CullMode);

  // Not exposed, only for old ui.
  ByteEnum<ScissorMode::Enum> mScissorMode;
};

// MultiRenderTarget classes are just for providing an uncluttered script interface for RenderSettings

/// Indexable interface for settings ColorTargets.
class ColorTargetMrt : public ThreadSafeReferenceCounted
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ColorTargetMrt(HandleOf<RenderSettings> renderSettings) : mRenderSettings(renderSettings) {}

  /// Set the RenderTarget to use for the given index.
  void Set(uint index, RenderTarget* colorTarget);

  HandleOf<RenderSettings> mRenderSettings;
};

/// Indexable interface for settings BlendSettings.
class BlendSettingsMrt : public ThreadSafeReferenceCounted
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  BlendSettingsMrt(HandleOf<RenderSettings> renderSettings) : mRenderSettings(renderSettings) {}

  /// Get the current BlendSettings for a color target at the given index.
  BlendSettings* Get(uint index);
  /// Set the BlendSettings for a color target at the given index.
  void Set(uint index, BlendSettings* blendSettings);

  HandleOf<RenderSettings> mRenderSettings;
};

/// Interface for configuring multiple color target outputs.
class MultiRenderTarget : public ThreadSafeReferenceCounted
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiRenderTarget(HandleOf<RenderSettings> renderSettings);

  /// Indexable interface for settings ColorTargets.
  ColorTargetMrt* GetColorTargetMrt() { return &mColorTargetMrt; }
  /// Indexable interface for settings BlendSettings.
  BlendSettingsMrt* GetBlendSettingsMrt() { return &mBlendSettingsMrt; }

#define Getter(type, name, index)                                                              \
  type* Get##name##index()                                                                     \
  {                                                                                            \
    if (mRenderSettings.IsNull())                                                              \
      return DoNotifyException("Error", "Attempting to call member on null object."), nullptr; \
    return mRenderSettings->Get##name##Mrt(index);                                             \
  }

#define Setter(type, name, index)                                                     \
  void Set##name##index(type* value)                                                  \
  {                                                                                   \
    if (mRenderSettings.IsNull())                                                     \
      return DoNotifyException("Error", "Attempting to call member on null object."); \
    mRenderSettings->Set##name##Mrt(value, index);                                    \
  }

  // TODO: Macro comments for auto-doc
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 0);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 1);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 2);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 3);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 4);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 5);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 6);
  /// RenderTarget at a specific index for configuring multiple render targets.
  Setter(RenderTarget, ColorTarget, 7);

  // TODO: Macro comments for auto-doc
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 0);
  Setter(BlendSettings, BlendSettings, 0);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 1);
  Setter(BlendSettings, BlendSettings, 1);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 2);
  Setter(BlendSettings, BlendSettings, 2);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 3);
  Setter(BlendSettings, BlendSettings, 3);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 4);
  Setter(BlendSettings, BlendSettings, 4);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 5);
  Setter(BlendSettings, BlendSettings, 5);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 6);
  Setter(BlendSettings, BlendSettings, 6);
  /// BlendSettings for a RenderTarget at a specific index for configuring multiple render targets.
  Getter(BlendSettings, BlendSettings, 7);
  Setter(BlendSettings, BlendSettings, 7);

#undef Getter
#undef Setter

  HandleOf<RenderSettings> mRenderSettings;
  ColorTargetMrt mColorTargetMrt;
  BlendSettingsMrt mBlendSettingsMrt;
};

} // namespace Zero
