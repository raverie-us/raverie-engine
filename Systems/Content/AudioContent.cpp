///////////////////////////////////////////////////////////////////////////////
///
/// \file AudioContent.cpp
/// Implementation of the Audio content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "AudioEngine/FileEncoder.h"

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

  ZilchBindFieldProperty(Name);
  ZilchBindFieldProperty(Streamed);
  ZilchBindGetterProperty(FileLength);
  ZilchBindGetterProperty(AudioChannels);
}

void SoundBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;

  Streamed = false;
}

void SoundBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);
  SerializeNameDefault(Streamed, false);
  SerializeNameDefault(mFileLength, 0.0f);
  SerializeNameDefault(mAudioChannels, 0);
}

void SoundBuilder::BuildContent(BuildOptions& options)
{
  Status status;
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  String destFile = FilePath::Combine(options.OutputPath, BuildString(Name, ".snd"));
  unsigned samples, channels, sampleRate;

  Audio::FileEncoder::ProcessFile(status, sourceFile, destFile, samples, channels, sampleRate);

  if (status.Succeeded())
  {
    mFileLength = (float)samples / (float)sampleRate;
    mAudioChannels = channels;
  }
  
  // This should probably be handled differently. The properties need to be saved because the object
  // is serialized before it is loaded, but BuildContent won't be called next time the engine starts,
  // so if we don't save the properties now they'll be lost.
  mOwner->SaveMetaFile();
}

bool SoundBuilder::NeedsBuilding(BuildOptions& options)
{
  return DirectBuilderComponent::NeedsBuilding(options);
}

void SoundBuilder::CopyFile(BuildOptions& options)
{
  String filename = mOwner->Filename;
  String destFile = FilePath::Combine(options.OutputPath, BuildString(Name, ".snd"));
  String sourceFile = FilePath::Combine(options.SourcePath, filename);

  // Copy the file over.
  Zero::CopyFile(destFile, sourceFile);

  // Mark the file as up to date.
  SetFileToCurrentTime(destFile);
}

void SoundBuilder::BuildListing(ResourceListing& listing)
{
  //Data is the same but loaders are different for streamed and direct load.
  if(Streamed)
    LoaderType = "StreamedSound";
  else
    LoaderType = "Sound";

  DirectBuilderComponent::BuildListing(listing);
}

//------------------------------------------------------------------------------
void CreateAudioContent(ContentSystem* system)
{
  AddContent<AudioContent>(system);
  AddContentComponent<SoundBuilder>(system);

  ContentTypeEntry audioContent(ZilchTypeId(AudioContent), MakeAudioContent);
  system->CreatorsByExtension["wav"] = audioContent;
  system->CreatorsByExtension["wv"] = audioContent;
  system->CreatorsByExtension["ogg"] = audioContent;
}

}//namespace Zero
