// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Enums
RaverieDefineEnum(AudioCueImport);
RaverieDefineEnum(BasisType);
RaverieDefineEnum(ConflictAction);
RaverieDefineEnum(ImageImport);
RaverieDefineEnum(LoopingMode);
RaverieDefineEnum(MeshImport);
RaverieDefineEnum(PhysicsImport);
RaverieDefineEnum(PhysicsMeshType);
RaverieDefineEnum(ScaleConversion);
RaverieDefineEnum(SpriteFill);
RaverieDefineEnum(SpriteSampling);
RaverieDefineEnum(TrackType);
RaverieDefineEnum(TextureAddressing);
RaverieDefineEnum(TextureAnisotropy);
RaverieDefineEnum(TextureCompression);
RaverieDefineEnum(TextureFace);
RaverieDefineEnum(TextureFiltering);
RaverieDefineEnum(TextureFormat);
RaverieDefineEnum(TextureMipMapping);
RaverieDefineEnum(TextureType);
RaverieDefineEnum(AudioFileLoadType);

RaverieDefineArrayType(Array<AnimationClip>);

RaverieDefineStaticLibrary(ContentMetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  RaverieInitializeEnum(AudioCueImport);
  RaverieInitializeEnum(BasisType);
  RaverieInitializeEnum(ConflictAction);
  RaverieInitializeEnum(ImageImport);
  RaverieInitializeEnum(LoopingMode);
  RaverieInitializeEnum(MeshImport);
  RaverieInitializeEnum(PhysicsImport);
  RaverieInitializeEnum(PhysicsMeshType);
  RaverieInitializeEnum(ScaleConversion);
  RaverieInitializeEnum(SpriteFill);
  RaverieInitializeEnum(SpriteSampling);
  RaverieInitializeEnum(TrackType);
  RaverieInitializeEnum(TextureAddressing);
  RaverieInitializeEnum(TextureAnisotropy);
  RaverieInitializeEnum(TextureCompression);
  RaverieInitializeEnum(TextureFace);
  RaverieInitializeEnum(TextureFiltering);
  RaverieInitializeEnum(TextureFormat);
  RaverieInitializeEnum(TextureMipMapping);
  RaverieInitializeEnum(TextureType);
  RaverieInitializeEnum(AudioFileLoadType);

  RaverieInitializeArrayTypeAs(Array<AnimationClip>, "AnimationClips");

  // Meta Components
  RaverieInitializeType(ContentMetaComposition);
  RaverieInitializeType(ContentItemMetaOperations);

  // Events
  RaverieInitializeType(ContentSystemEvent);
  RaverieInitializeType(KeyFrameEvent);
  RaverieInitializeType(TrackEvent);

  RaverieInitializeType(ContentItem);
  RaverieInitializeType(ContentLibrary);
  RaverieInitializeType(ContentSystem);
  RaverieInitializeType(ContentComposition);
  RaverieInitializeType(ContentComponent);
  RaverieInitializeType(BuilderComponent);
  RaverieInitializeType(DataContent);
  RaverieInitializeType(DataBuilder);
  RaverieInitializeType(ContentTags);
  RaverieInitializeType(FontContent);
  RaverieInitializeType(FontBuilder);
  RaverieInitializeType(ImageContent);
  RaverieInitializeType(ImageOptions);
  RaverieInitializeType(ShowNormalGenerationOptionsFilter);
  RaverieInitializeType(GeometryOptions);
  RaverieInitializeType(AudioOptions);
  RaverieInitializeType(ConflictOptions);
  RaverieInitializeType(ImportOptions);
  RaverieInitializeType(ShowPremultipliedAlphaFilter);
  RaverieInitializeType(ShowGammaCorrectionFilter);
  RaverieInitializeType(TextureBuilder);
  RaverieInitializeType(SpriteData);
  RaverieInitializeType(SpriteSourceBuilder);
  RaverieInitializeType(TextContent);
  RaverieInitializeType(BaseTextBuilder);
  RaverieInitializeType(TextBuilder);
  RaverieInitializeType(RaverieScriptBuilder);
  RaverieInitializeType(RaverieFragmentBuilder);
  RaverieInitializeType(ContentCopyright);
  RaverieInitializeType(ContentNotes);
  RaverieInitializeType(ContentEditorOptions);
  RaverieInitializeType(ResourceTemplate);
  RaverieInitializeType(RichAnimation);
  RaverieInitializeType(RichAnimationBuilder);
  RaverieInitializeType(TrackNode);
  RaverieInitializeType(GeometryImport);
  RaverieInitializeType(GeometryResourceEntry);
  RaverieInitializeType(MeshBuilder);
  RaverieInitializeType(PhysicsMeshBuilder);
  RaverieInitializeType(AnimationClip);
  RaverieInitializeType(AnimationBuilder);
  RaverieInitializeType(TextureContent);
  RaverieInitializeType(GeometryContent);
  RaverieInitializeType(AudioContent);
  RaverieInitializeType(TextureInfo);
  RaverieInitializeType(SoundBuilder);
  RaverieInitializeType(BinaryContent);
  RaverieInitializeType(BinaryBuilder);
  RaverieInitializeType(GeneratedArchetype);

  // @trevor.sundberg: The content and engine libraries are co-dependent, and
  // since content references the Archetype type, we get an error that it hasn't
  // yet been initialized since Content is initialized first. This prevents the
  // assert:
  RaverieTypeId(Archetype)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void ContentMetaLibrary::Initialize()
{
  ContentSystem::Initialize();
  Z::gContentSystem = ContentSystem::GetInstance();

  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void ContentMetaLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
  ContentSystem::Destroy();
}

} // namespace Raverie
