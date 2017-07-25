///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson, Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(GraphicalEntryRange);
ZilchDefineRange(MultiSpriteEntryRange);
ZilchDefineRange(VertexSemanticRange);
ZilchDefineRange(ParticleListRange);

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

ZilchDefineStaticLibrary(GraphicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRange(GraphicalEntryRange);
  ZilchInitializeRange(MultiSpriteEntryRange);
  ZilchInitializeRange(ParticleListRange);
  ZilchInitializeRange(VertexSemanticRange);

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
  ZilchInitializeType(BlendSettings);
  ZilchInitializeType(BlendSettingsMrt);
  ZilchInitializeType(Bone);
  ZilchInitializeType(Camera);
  ZilchInitializeType(ColorTargetMrt);
  ZilchInitializeType(DebugGraphical);
  ZilchInitializeType(DebugGraphicalPrimitive);
  ZilchInitializeType(DebugGraphicalThickLine);
  ZilchInitializeType(DebugGraphicalText);
  ZilchInitializeType(DefinitionSet);
  ZilchInitializeType(DepthSettings);
  //ZilchInitializeType(DynamicMeshParticleEmitter);
  ZilchInitializeType(Font);
  ZilchInitializeType(GraphicalEntry);
  ZilchInitializeType(GraphicalRangeInterface);
  ZilchInitializeType(GraphicsDriverSupport);
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
  ZilchInitializeType(ProxyObject<MaterialBlock>);
  ZilchInitializeType(RenderGroup);
  ZilchInitializeType(RenderGroupList);
  ZilchInitializeType(RenderSettings);
  ZilchInitializeType(RenderTarget);
  ZilchInitializeType(SamplerSettings);
  ZilchInitializeType(SelectionIcon);
  ZilchInitializeType(ShaderInputs);
  ZilchInitializeType(Skeleton);
  ZilchInitializeType(SkinnedModel);
  ZilchInitializeType(Sprite);
  ZilchInitializeType(SpriteParticleSystem);
  ZilchInitializeType(SpriteSource);
  ZilchInitializeType(SpriteText);
  ZilchInitializeType(TextDefinition);
  ZilchInitializeType(Texture);
  ZilchInitializeType(TextureData);
  ZilchInitializeType(VertexBuffer);
  ZilchInitializeType(ZilchFragment);

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
