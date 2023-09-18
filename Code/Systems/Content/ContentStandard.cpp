// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(AudioCueImport);
ZilchDefineEnum(BasisType);
ZilchDefineEnum(ConflictAction);
ZilchDefineEnum(ImageImport);
ZilchDefineEnum(LoopingMode);
ZilchDefineEnum(MeshImport);
ZilchDefineEnum(PhysicsImport);
ZilchDefineEnum(PhysicsMeshType);
ZilchDefineEnum(ScaleConversion);
ZilchDefineEnum(SpriteFill);
ZilchDefineEnum(SpriteSampling);
ZilchDefineEnum(TrackType);
ZilchDefineEnum(TextureAddressing);
ZilchDefineEnum(TextureAnisotropy);
ZilchDefineEnum(TextureCompression);
ZilchDefineEnum(TextureFace);
ZilchDefineEnum(TextureFiltering);
ZilchDefineEnum(TextureFormat);
ZilchDefineEnum(TextureMipMapping);
ZilchDefineEnum(TextureType);
ZilchDefineEnum(AudioFileLoadType);

ZeroDefineArrayType(Array<AnimationClip>);

ZilchDefineStaticLibrary(ContentMetaLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(AudioCueImport);
  ZilchInitializeEnum(BasisType);
  ZilchInitializeEnum(ConflictAction);
  ZilchInitializeEnum(ImageImport);
  ZilchInitializeEnum(LoopingMode);
  ZilchInitializeEnum(MeshImport);
  ZilchInitializeEnum(PhysicsImport);
  ZilchInitializeEnum(PhysicsMeshType);
  ZilchInitializeEnum(ScaleConversion);
  ZilchInitializeEnum(SpriteFill);
  ZilchInitializeEnum(SpriteSampling);
  ZilchInitializeEnum(TrackType);
  ZilchInitializeEnum(TextureAddressing);
  ZilchInitializeEnum(TextureAnisotropy);
  ZilchInitializeEnum(TextureCompression);
  ZilchInitializeEnum(TextureFace);
  ZilchInitializeEnum(TextureFiltering);
  ZilchInitializeEnum(TextureFormat);
  ZilchInitializeEnum(TextureMipMapping);
  ZilchInitializeEnum(TextureType);
  ZilchInitializeEnum(AudioFileLoadType);

  ZeroInitializeArrayTypeAs(Array<AnimationClip>, "AnimationClips");

  // Meta Components
  ZilchInitializeType(ContentMetaComposition);
  ZilchInitializeType(ContentItemMetaOperations);

  // Events
  ZilchInitializeType(ContentSystemEvent);
  ZilchInitializeType(KeyFrameEvent);
  ZilchInitializeType(TrackEvent);

  ZilchInitializeType(ContentItem);
  ZilchInitializeType(ContentLibrary);
  ZilchInitializeType(ContentSystem);
  ZilchInitializeType(ContentComposition);
  ZilchInitializeType(ContentComponent);
  ZilchInitializeType(BuilderComponent);
  ZilchInitializeType(DataContent);
  ZilchInitializeType(DataBuilder);
  ZilchInitializeType(ContentTags);
  ZilchInitializeType(FontContent);
  ZilchInitializeType(FontBuilder);
  ZilchInitializeType(ImageContent);
  ZilchInitializeType(ImageOptions);
  ZilchInitializeType(ShowNormalGenerationOptionsFilter);
  ZilchInitializeType(GeometryOptions);
  ZilchInitializeType(AudioOptions);
  ZilchInitializeType(ConflictOptions);
  ZilchInitializeType(ImportOptions);
  ZilchInitializeType(ShowPremultipliedAlphaFilter);
  ZilchInitializeType(ShowGammaCorrectionFilter);
  ZilchInitializeType(TextureBuilder);
  ZilchInitializeType(SpriteData);
  ZilchInitializeType(SpriteSourceBuilder);
  ZilchInitializeType(TextContent);
  ZilchInitializeType(BaseTextBuilder);
  ZilchInitializeType(TextBuilder);
  ZilchInitializeType(ZilchScriptBuilder);
  ZilchInitializeType(ZilchFragmentBuilder);
  ZilchInitializeType(ContentCopyright);
  ZilchInitializeType(ContentNotes);
  ZilchInitializeType(ContentEditorOptions);
  ZilchInitializeType(ResourceTemplate);
  ZilchInitializeType(RichAnimation);
  ZilchInitializeType(RichAnimationBuilder);
  ZilchInitializeType(TrackNode);
  ZilchInitializeType(GeometryImport);
  ZilchInitializeType(GeometryResourceEntry);
  ZilchInitializeType(MeshBuilder);
  ZilchInitializeType(PhysicsMeshBuilder);
  ZilchInitializeType(AnimationClip);
  ZilchInitializeType(AnimationBuilder);
  ZilchInitializeType(TextureContent);
  ZilchInitializeType(GeometryContent);
  ZilchInitializeType(AudioContent);
  ZilchInitializeType(TextureInfo);
  ZilchInitializeType(SoundBuilder);
  ZilchInitializeType(BinaryContent);
  ZilchInitializeType(BinaryBuilder);
  ZilchInitializeType(GeneratedArchetype);

  // @trevor.sundberg: The content and engine libraries are co-dependent, and
  // since content references the Archetype type, we get an error that it hasn't
  // yet been initialized since Content is initialized first. This prevents the
  // assert:
  ZilchTypeId(Archetype)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

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

} // namespace Zero
