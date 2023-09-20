// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{

/// A set of shader inputs for overriding values per object or globally.
class ShaderInputs : public ReferenceCountedThreadSafeId32
{
public:
  RaverieDeclareType(ShaderInputs, TypeCopyMode::ReferenceType);

  ~ShaderInputs();

  static HandleOf<ShaderInputs> Create();

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

  // We've encountered problems with freeing ShaderInputs more than once, so we're
  // placing this guard here to ensure that we don't free it multiple times.
  static const u32 cGuardId = 0xF00DF00D;
  u32 mGuardId = cGuardId;
};

/// Settings for how pixel shader outputs are combined with the RenderTarget's
/// current values.
class GraphicsBlendSettings : public BlendSettings
{
public:
  RaverieDeclareType(GraphicsBlendSettings, TypeCopyMode::ReferenceType);
  DeclareThreadSafeReferenceCountedHandleNoData(GraphicsBlendSettings);

  static void ConstructedStatic(BlendSettings* settings);
  static void DestructedStatic(BlendSettings* settings);
  void ConstructedInstance();
  void DestructedInstance();
};

/// Settings for how the depth buffer should control pixel output.
class GraphicsDepthSettings : public DepthSettings
{
public:
  RaverieDeclareType(GraphicsDepthSettings, TypeCopyMode::ReferenceType);
  DeclareThreadSafeReferenceCountedHandleNoData(GraphicsDepthSettings);

  static void ConstructedStatic(DepthSettings* settings);
  static void DestructedStatic(DepthSettings* settings);
  void ConstructedInstance();
  void DestructedInstance();
};

/// Contains all output targets and render settings needed for a render task.
class GraphicsRenderSettings : public RenderSettings
{
public:
  RaverieDeclareType(GraphicsRenderSettings, TypeCopyMode::ReferenceType);

  GraphicsRenderSettings();

  /// Settings to use when blending shader output with the ColorTarget,
  /// implicitly BlendSettings0.
  GraphicsBlendSettings* GetBlendSettings();
  void SetBlendSettings(GraphicsBlendSettings* blendSettings);

  /// Settings to use when doing depth/stencil testing with DepthTarget.
  GraphicsDepthSettings* GetDepthSettings();
  void SetDepthSettings(GraphicsDepthSettings* depthSettings);

  GraphicsBlendSettings* GetBlendSettingsMrt(uint location);
  void SetBlendSettingsMrt(GraphicsBlendSettings* blendSettings, uint location);

  /// The RenderTarget of a color format to output to, implicitly RenderTarget0.
  void SetColorTarget(RenderTarget* target);

  /// The RenderTarget of a depth format to use as a depth buffer for
  /// depth/stencil testing.
  void SetDepthTarget(RenderTarget* target);

  /// Shader input values to be globally overridden for all objects/shaders.
  ShaderInputs* GetGlobalShaderInputs();
  void SetGlobalShaderInputs(ShaderInputs* shaderInputs);
  HandleOf<ShaderInputs> mGlobalShaderInputs;

  // Do not call from C++, unless you delete the returned pointer yourself.
  /// Interface for configuring multiple color target outputs.
  MultiRenderTarget* GetMultiRenderTarget();

  // Helpers for MultiRenderTarget interface.
  void SetColorTargetMrt(RenderTarget* target, uint location);

  // Clears all targets and settings.
  void ClearAll();
};

// MultiRenderTarget classes are just for providing an uncluttered script
// interface for RenderSettings

/// Indexable interface for settings ColorTargets.
class ColorTargetMrt : public ThreadSafeReferenceCounted
{
public:
  RaverieDeclareType(ColorTargetMrt, TypeCopyMode::ReferenceType);

  ColorTargetMrt(HandleOf<GraphicsRenderSettings> renderSettings) : mRenderSettings(renderSettings)
  {
  }

  /// Set the RenderTarget to use for the given index.
  void Set(uint index, RenderTarget* colorTarget);

  HandleOf<GraphicsRenderSettings> mRenderSettings;
};

/// Indexable interface for settings BlendSettings.
class BlendSettingsMrt : public ThreadSafeReferenceCounted
{
public:
  RaverieDeclareType(BlendSettingsMrt, TypeCopyMode::ReferenceType);

  BlendSettingsMrt(HandleOf<GraphicsRenderSettings> renderSettings) : mRenderSettings(renderSettings)
  {
  }

  /// Get the current BlendSettings for a color target at the given index.
  GraphicsBlendSettings* Get(uint index);
  /// Set the BlendSettings for a color target at the given index.
  void Set(uint index, GraphicsBlendSettings* blendSettings);

  HandleOf<GraphicsRenderSettings> mRenderSettings;
};

/// Interface for configuring multiple color target outputs.
class MultiRenderTarget : public ThreadSafeReferenceCounted
{
public:
  RaverieDeclareType(MultiRenderTarget, TypeCopyMode::ReferenceType);

  MultiRenderTarget(HandleOf<GraphicsRenderSettings> renderSettings);

  /// Indexable interface for settings ColorTargets.
  ColorTargetMrt* GetColorTargetMrt()
  {
    return &mColorTargetMrt;
  }
  /// Indexable interface for settings BlendSettings.
  BlendSettingsMrt* GetBlendSettingsMrt()
  {
    return &mBlendSettingsMrt;
  }

#define Getter(type, name, index)                                                                                                                                                                      \
  type* Get##name##index()                                                                                                                                                                             \
  {                                                                                                                                                                                                    \
    if (mRenderSettings.IsNull())                                                                                                                                                                      \
    {                                                                                                                                                                                                  \
      DoNotifyException("Error", "Attempting to call member on null object.");                                                                                                                         \
      return nullptr;                                                                                                                                                                                  \
    }                                                                                                                                                                                                  \
    return mRenderSettings->Get##name##Mrt(index);                                                                                                                                                     \
  }

#define Setter(type, name, index)                                                                                                                                                                      \
  void Set##name##index(type* value)                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    if (mRenderSettings.IsNull())                                                                                                                                                                      \
      return DoNotifyException("Error", "Attempting to call member on null object.");                                                                                                                  \
    mRenderSettings->Set##name##Mrt(value, index);                                                                                                                                                     \
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
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 0);
  Setter(GraphicsBlendSettings, BlendSettings, 0);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 1);
  Setter(GraphicsBlendSettings, BlendSettings, 1);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 2);
  Setter(GraphicsBlendSettings, BlendSettings, 2);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 3);
  Setter(GraphicsBlendSettings, BlendSettings, 3);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 4);
  Setter(GraphicsBlendSettings, BlendSettings, 4);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 5);
  Setter(GraphicsBlendSettings, BlendSettings, 5);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 6);
  Setter(GraphicsBlendSettings, BlendSettings, 6);
  /// GraphicsBlendSettings for a RenderTarget at a specific index for
  /// configuring multiple render targets.
  Getter(GraphicsBlendSettings, BlendSettings, 7);
  Setter(GraphicsBlendSettings, BlendSettings, 7);

#undef Getter
#undef Setter

  HandleOf<GraphicsRenderSettings> mRenderSettings;
  ColorTargetMrt mColorTargetMrt;
  BlendSettingsMrt mBlendSettingsMrt;
};

} // namespace Raverie
