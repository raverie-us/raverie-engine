///////////////////////////////////////////////////////////////////////////////
///
/// \file AudioContent.cpp
/// Implementation of the Audio content classes.
/// 
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "../Sound/SoundStandard.hpp"

namespace Zero
{
ZilchDefineType(AudioContent, builder, type)
{
}

AudioContent::AudioContent()
{
  EditMode = ContentEditMode::ContentItem;
}

//---------------------------------------------------------------------- Factory

ContentItem* MakeAudioContent(ContentInitializer& initializer)
{
  AudioContent* content = new AudioContent();

  content->Filename = initializer.Filename;

  SoundBuilder* builder = new SoundBuilder();
  builder->Generate(initializer);
  content->AddComponent(builder);

  return content;
}

//------------------------------------------------------------------- SoundBuilder
ZilchDefineType(SoundBuilder, builder, type)
{
  ZeroBindDependency(AudioContent);
  ZeroBindDocumented();
  ZeroBindExpanded();

  ZilchBindFieldProperty(Name);
  ZilchBindFieldProperty(mFileLoadType);
  ZilchBindFieldProperty(mNormalize)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mMaxVolume)->Add(new EditorSlider(0.0f, 1.0f, 0.1f))->ZeroFilterBool(mNormalize);
}

void SoundBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;
  mFileLoadType = initializer.Options->mAudioOptions->mStreamingMode;
}

void SoundBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);
  SerializeEnumNameDefault(AudioFileLoadType, mFileLoadType, AudioFileLoadType::Auto);
  SerializeNameDefault(mNormalize, false);
  SerializeNameDefault(mMaxVolume, 0.9f);

  // This should be removed at the next major version (makes sure that we keep the streaming
  // setting for existing sounds)
  if (stream.GetType() != SerializerType::Binary && stream.GetMode() == SerializerMode::Loading)
  {
    SerializeNameDefault(mStreamed, false);
    if (mStreamed)
    {
      mFileLoadType = AudioFileLoadType::StreamFromFile;
      mStreamed = false;
    }
  }
}

void SoundBuilder::BuildContent(BuildOptions& options)
{
  Status status;
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  String destFile = FilePath::Combine(options.OutputPath, BuildString(Name, SoundExtension));

  // Create the AudioFile object and open the source file
  AudioFileData audioFile = AudioFileEncoder::OpenFile(status, sourceFile);

  if (status.Succeeded())
  {
    // Encode the file and write it out to disk
    AudioFileEncoder::WriteFile(status, destFile, audioFile, mNormalize, mMaxVolume);

    if (status.Failed())
      DoNotifyWarning("Error Processing Audio File", status.Message);
  }
  else
    DoNotifyWarning("Error Processing Audio File", status.Message);
}

bool SoundBuilder::NeedsBuilding(BuildOptions& options)
{
  return DirectBuilderComponent::NeedsBuilding(options);
}

void SoundBuilder::BuildListing(ResourceListing& listing)
{
  //Data is the same but loaders are different for streamed and direct load.
  if (mFileLoadType == AudioFileLoadType::StreamFromFile)
    LoaderType = "StreamedSound";
  else if (mFileLoadType == AudioFileLoadType::Uncompressed)
    LoaderType = "Sound";
  else
    LoaderType = "AutoStreamedSound";

  DirectBuilderComponent::BuildListing(listing);
}

//------------------------------------------------------------------------------
void CreateAudioContent(ContentSystem* system)
{
  AddContent<AudioContent>(system);
  AddContentComponent<SoundBuilder>(system);

  ContentTypeEntry audioContent(ZilchTypeId(AudioContent), MakeAudioContent);
  system->CreatorsByExtension["wav"] = audioContent;
  system->CreatorsByExtension["ogg"] = audioContent;
}

}//namespace Zero
