// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(GraphicalEntryRange);
ZilchDefineRange(MultiSpriteEntryRange);
ZilchDefineRange(VertexSemanticRange);
ZilchDefineRange(ParticleListRange);
ZilchDefineRange(Array<HandleOf<Material>>);
ZilchDefineRange(Array<HandleOf<RenderGroup>>);

// Enums
ZilchDefineEnum(BlendMode);
ZilchDefineEnum(BlendFactor);
ZilchDefineEnum(BlendEquation);
ZilchDefineEnum(CullMode);
ZilchDefineEnum(DepthMode);
ZilchDefineEnum(GraphicalSortMethod);
ZilchDefineEnum(MeshEmitMode);
ZilchDefineEnum(PerspectiveMode);
ZilchDefineEnum(PrimitiveType);
ZilchDefineEnum(SpriteGeometryMode);
ZilchDefineEnum(SpriteParticleAnimationMode);
ZilchDefineEnum(SpriteParticleGeometryMode);
ZilchDefineEnum(SpriteParticleSortMode);
ZilchDefineEnum(StencilMode);
ZilchDefineEnum(StencilOp);
ZilchDefineEnum(SystemSpace);
ZilchDefineEnum(TextAlign);
ZilchDefineEnum(TextureCompareFunc);
ZilchDefineEnum(TextureCompareMode);
ZilchDefineEnum(VertexElementType);
ZilchDefineEnum(VertexSemantic);
ZilchDefineEnum(ViewportScaling);

ZilchDeclareExternalType(GraphicsDriverSupport);
ZilchDeclareExternalType(SamplerSettings);

ZilchDefineStaticLibrary(GraphicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRange(GraphicalEntryRange);
  ZilchInitializeRange(MultiSpriteEntryRange);
  ZilchInitializeRange(ParticleListRange);
  ZilchInitializeRange(VertexSemanticRange);
  ZilchInitializeRange(Array<HandleOf<Material>>);
  ZilchInitializeRange(Array<HandleOf<RenderGroup>>);

  // Enums
  ZilchInitializeEnum(BlendMode);
  ZilchInitializeEnum(BlendFactor);
  ZilchInitializeEnum(BlendEquation);
  ZilchInitializeEnum(CullMode);
  ZilchInitializeEnum(DepthMode);
  ZilchInitializeEnum(GraphicalSortMethod);
  ZilchInitializeEnum(MeshEmitMode);
  ZilchInitializeEnum(PerspectiveMode);
  ZilchInitializeEnum(PrimitiveType);
  ZilchInitializeEnum(SpriteGeometryMode);
  ZilchInitializeEnum(SpriteParticleAnimationMode);
  ZilchInitializeEnum(SpriteParticleGeometryMode);
  ZilchInitializeEnum(SpriteParticleSortMode);
  ZilchInitializeEnum(StencilMode);
  ZilchInitializeEnum(StencilOp);
  ZilchInitializeEnum(SystemSpace);
  ZilchInitializeEnum(TextAlign);
  ZilchInitializeEnum(TextureCompareFunc);
  ZilchInitializeEnum(TextureCompareMode);
  ZilchInitializeEnum(VertexElementType);
  ZilchInitializeEnum(VertexSemantic);
  ZilchInitializeEnum(ViewportScaling);

  // Meta Components
  ZilchInitializeType(MaterialFactory);
  ZilchInitializeType(CompositionLabelExtension);
  ZilchInitializeType(HideBaseFilter);

  // Events
  ZilchInitializeType(GraphicalEvent);
  ZilchInitializeType(GraphicalSortEvent);
  ZilchInitializeType(ParticleEvent);
  ZilchInitializeType(RenderTasksEvent);
  ZilchInitializeType(ResourceListEvent);
  ZilchInitializeType(ShaderInputsEvent);

  ZilchInitializeType(Graphical);
  ZilchInitializeType(SlicedDefinition);

  ZilchInitializeType(Atlas);
  ZilchInitializeType(BaseSprite);
  ZilchInitializeTypeAs(GraphicsBlendSettings, "BlendSettings");
  ZilchInitializeType(BlendSettingsMrt);
  ZilchInitializeType(Bone);
  ZilchInitializeType(Camera);
  ZilchInitializeType(ColorTargetMrt);
  ZilchInitializeType(DebugGraphical);
  ZilchInitializeType(DebugGraphicalPrimitive);
  ZilchInitializeType(DebugGraphicalThickLine);
  ZilchInitializeType(DebugGraphicalText);
  ZilchInitializeType(DefinitionSet);
  ZilchInitializeTypeAs(GraphicsDepthSettings, "DepthSettings");
  // ZilchInitializeType(DynamicMeshParticleEmitter);
  ZilchInitializeType(Font);
  ZilchInitializeType(GraphicalEntry);
  ZilchInitializeType(GraphicalRangeInterface);
  ZilchInitializeExternalType(GraphicsDriverSupport);
  ZilchInitializeType(GraphicsEngine);
  ZilchInitializeType(GraphicsRaycastProvider);
  ZilchInitializeType(GraphicsSpace);
  ZilchInitializeType(HeightMapModel);
  ZilchInitializeType(ImageDefinition);
  ZilchInitializeType(IndexBuffer);
  ZilchInitializeType(Material);
  ZilchInitializeType(MaterialBlock);
  ZilchInitializeType(MaterialList);
  ZilchInitializeType(Mesh);
  ZilchInitializeType(Model);
  ZilchInitializeType(MultiRenderTarget);
  ZilchInitializeType(MultiSprite);
  ZilchInitializeType(MultiSpriteEntry);
  ZilchInitializeType(Particle);
  ZilchInitializeType(ParticleAnimator);
  ZilchInitializeType(ParticleCollisionPlane);
  ZilchInitializeType(ParticleCollisionHeightmap);
  ZilchInitializeType(ParticleEmitter);
  ZilchInitializeType(ParticleSystem);
  ZilchInitializeTypeAs(ProxyObject<MaterialBlock>, "MaterialBlockProxy");
  ZilchInitializeType(RenderGroup);
  ZilchInitializeType(RenderGroupList);
  ZilchInitializeTypeAs(GraphicsRenderSettings, "RenderSettings");
  ZilchInitializeType(RenderTarget);
  ZilchInitializeExternalType(SamplerSettings);
  ZilchInitializeType(SelectionIcon);
  ZilchInitializeType(ShaderInputs);
  ZilchInitializeType(Skeleton);
  ZilchInitializeType(SkinnedModel);
  ZilchInitializeType(Sprite);
  ZilchInitializeType(SpriteParticleSystem);
  ZilchInitializeType(SpriteSource);
  ZilchInitializeType(SpriteText);
  ZilchInitializeType(SubRenderGroupPass);
  ZilchInitializeType(TextDefinition);
  ZilchInitializeType(Texture);
  ZilchInitializeType(TextureData);
  ZilchInitializeType(VertexBuffer);
  ZilchInitializeType(ViewportInterface);
  ZilchInitializeType(ZilchFragment);

  // Dependent on RenderGroupList
  ZilchInitializeType(ChildRenderGroupList);

  ZilchInitializeTypeAs(GraphicsStatics, "Graphics");

  // Particle Animators
  ZilchInitializeType(LinearParticleAnimator);
  ZilchInitializeType(ParticleAttractor);
  ZilchInitializeType(ParticleColorAnimator);
  ZilchInitializeType(ParticleTwister);
  ZilchInitializeType(ParticleWander);

  // Particle Emitters
  ZilchInitializeType(ParticleEmitterShared);
  ZilchInitializeType(BoxParticleEmitter);
  ZilchInitializeType(MeshParticleEmitter);
  ZilchInitializeType(SphericalParticleEmitter);
  ZilchInitializeType(MeshParticleEmitter);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void GraphicsLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  ShaderIntrinsicsLibrary::InitializeInstance();

  InitializeResourceManager(DefinitionSetManager);
  InitializeResourceManager(FontManager);

  InitializeResourceManager(AtlasManager);
  InitializeResourceManager(MaterialManager);
  InitializeResourceManager(MeshManager);
  InitializeResourceManager(RenderGroupManager);
  InitializeResourceManager(SpriteSourceManager);
  InitializeResourceManager(TextureManager);
  InitializeResourceManager(ZilchFragmentManager);

  ResourceLibrary::sFragmentType = ZilchTypeId(ZilchFragment);
}

void GraphicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ShaderIntrinsicsLibrary::Destroy();
}

} // namespace Zero
