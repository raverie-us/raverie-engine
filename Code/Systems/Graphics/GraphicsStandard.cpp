// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(GraphicalEntryRange);
RaverieDefineRange(MultiSpriteEntryRange);
RaverieDefineRange(VertexSemanticRange);
RaverieDefineRange(ParticleListRange);
RaverieDefineRange(Array<HandleOf<Material>>);
RaverieDefineRange(Array<HandleOf<RenderGroup>>);

// Enums
RaverieDefineEnum(BlendMode);
RaverieDefineEnum(BlendFactor);
RaverieDefineEnum(BlendEquation);
RaverieDefineEnum(CullMode);
RaverieDefineEnum(DepthMode);
RaverieDefineEnum(GraphicalSortMethod);
RaverieDefineEnum(MeshEmitMode);
RaverieDefineEnum(PerspectiveMode);
RaverieDefineEnum(PrimitiveType);
RaverieDefineEnum(SpriteGeometryMode);
RaverieDefineEnum(SpriteParticleAnimationMode);
RaverieDefineEnum(SpriteParticleGeometryMode);
RaverieDefineEnum(SpriteParticleSortMode);
RaverieDefineEnum(StencilMode);
RaverieDefineEnum(StencilOp);
RaverieDefineEnum(SystemSpace);
RaverieDefineEnum(TextAlign);
RaverieDefineEnum(TextureCompareFunc);
RaverieDefineEnum(TextureCompareMode);
RaverieDefineEnum(VertexElementType);
RaverieDefineEnum(VertexSemantic);
RaverieDefineEnum(ViewportScaling);

RaverieDeclareExternalType(GraphicsDriverSupport);
RaverieDeclareExternalType(SamplerSettings);

RaverieDefineStaticLibrary(GraphicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRange(GraphicalEntryRange);
  RaverieInitializeRange(MultiSpriteEntryRange);
  RaverieInitializeRange(ParticleListRange);
  RaverieInitializeRange(VertexSemanticRange);
  RaverieInitializeRange(Array<HandleOf<Material>>);
  RaverieInitializeRange(Array<HandleOf<RenderGroup>>);

  // Enums
  RaverieInitializeEnum(BlendMode);
  RaverieInitializeEnum(BlendFactor);
  RaverieInitializeEnum(BlendEquation);
  RaverieInitializeEnum(CullMode);
  RaverieInitializeEnum(DepthMode);
  RaverieInitializeEnum(GraphicalSortMethod);
  RaverieInitializeEnum(MeshEmitMode);
  RaverieInitializeEnum(PerspectiveMode);
  RaverieInitializeEnum(PrimitiveType);
  RaverieInitializeEnum(SpriteGeometryMode);
  RaverieInitializeEnum(SpriteParticleAnimationMode);
  RaverieInitializeEnum(SpriteParticleGeometryMode);
  RaverieInitializeEnum(SpriteParticleSortMode);
  RaverieInitializeEnum(StencilMode);
  RaverieInitializeEnum(StencilOp);
  RaverieInitializeEnum(SystemSpace);
  RaverieInitializeEnum(TextAlign);
  RaverieInitializeEnum(TextureCompareFunc);
  RaverieInitializeEnum(TextureCompareMode);
  RaverieInitializeEnum(VertexElementType);
  RaverieInitializeEnum(VertexSemantic);
  RaverieInitializeEnum(ViewportScaling);

  // Meta Components
  RaverieInitializeType(MaterialFactory);
  RaverieInitializeType(CompositionLabelExtension);
  RaverieInitializeType(HideBaseFilter);

  // Events
  RaverieInitializeType(GraphicalEvent);
  RaverieInitializeType(GraphicalSortEvent);
  RaverieInitializeType(ParticleEvent);
  RaverieInitializeType(RenderTasksEvent);
  RaverieInitializeType(ResourceListEvent);
  RaverieInitializeType(ShaderInputsEvent);

  RaverieInitializeType(Graphical);
  RaverieInitializeType(SlicedDefinition);

  RaverieInitializeType(Atlas);
  RaverieInitializeType(BaseSprite);
  RaverieInitializeTypeAs(GraphicsBlendSettings, "BlendSettings");
  RaverieInitializeType(BlendSettingsMrt);
  RaverieInitializeType(Bone);
  RaverieInitializeType(Camera);
  RaverieInitializeType(ColorTargetMrt);
  RaverieInitializeType(DebugGraphical);
  RaverieInitializeType(DebugGraphicalPrimitive);
  RaverieInitializeType(DebugGraphicalThickLine);
  RaverieInitializeType(DebugGraphicalText);
  RaverieInitializeType(DefinitionSet);
  RaverieInitializeTypeAs(GraphicsDepthSettings, "DepthSettings");
  // RaverieInitializeType(DynamicMeshParticleEmitter);
  RaverieInitializeType(Font);
  RaverieInitializeType(GraphicalEntry);
  RaverieInitializeType(GraphicalRangeInterface);
  RaverieInitializeExternalType(GraphicsDriverSupport);
  RaverieInitializeType(GraphicsEngine);
  RaverieInitializeType(GraphicsRaycastProvider);
  RaverieInitializeType(GraphicsSpace);
  RaverieInitializeType(HeightMapModel);
  RaverieInitializeType(ImageDefinition);
  RaverieInitializeType(IndexBuffer);
  RaverieInitializeType(Material);
  RaverieInitializeType(MaterialBlock);
  RaverieInitializeType(MaterialList);
  RaverieInitializeType(Mesh);
  RaverieInitializeType(Model);
  RaverieInitializeType(MultiRenderTarget);
  RaverieInitializeType(MultiSprite);
  RaverieInitializeType(MultiSpriteEntry);
  RaverieInitializeType(Particle);
  RaverieInitializeType(ParticleAnimator);
  RaverieInitializeType(ParticleCollisionPlane);
  RaverieInitializeType(ParticleCollisionHeightmap);
  RaverieInitializeType(ParticleEmitter);
  RaverieInitializeType(ParticleSystem);
  RaverieInitializeTypeAs(ProxyObject<MaterialBlock>, "MaterialBlockProxy");
  RaverieInitializeType(RenderGroup);
  RaverieInitializeType(RenderGroupList);
  RaverieInitializeTypeAs(GraphicsRenderSettings, "RenderSettings");
  RaverieInitializeType(RenderTarget);
  RaverieInitializeExternalType(SamplerSettings);
  RaverieInitializeType(SelectionIcon);
  RaverieInitializeType(ShaderInputs);
  RaverieInitializeType(Skeleton);
  RaverieInitializeType(SkinnedModel);
  RaverieInitializeType(Sprite);
  RaverieInitializeType(SpriteParticleSystem);
  RaverieInitializeType(SpriteSource);
  RaverieInitializeType(SpriteText);
  RaverieInitializeType(SubRenderGroupPass);
  RaverieInitializeType(TextDefinition);
  RaverieInitializeType(Texture);
  RaverieInitializeType(TextureData);
  RaverieInitializeType(VertexBuffer);
  RaverieInitializeType(ViewportInterface);
  RaverieInitializeType(RaverieFragment);

  // Dependent on RenderGroupList
  RaverieInitializeType(ChildRenderGroupList);

  RaverieInitializeTypeAs(GraphicsStatics, "Graphics");

  // Particle Animators
  RaverieInitializeType(LinearParticleAnimator);
  RaverieInitializeType(ParticleAttractor);
  RaverieInitializeType(ParticleColorAnimator);
  RaverieInitializeType(ParticleTwister);
  RaverieInitializeType(ParticleWander);

  // Particle Emitters
  RaverieInitializeType(ParticleEmitterShared);
  RaverieInitializeType(BoxParticleEmitter);
  RaverieInitializeType(MeshParticleEmitter);
  RaverieInitializeType(SphericalParticleEmitter);
  RaverieInitializeType(MeshParticleEmitter);

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
  InitializeResourceManager(RaverieFragmentManager);

  ResourceLibrary::sFragmentType = RaverieTypeId(RaverieFragment);
}

void GraphicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ShaderIntrinsicsLibrary::Destroy();
}

} // namespace Raverie
