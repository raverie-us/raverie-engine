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
#include "AudioEngine/AudioSystemInterface.h"

namespace Zero
{
ZilchDefineType(AudioContent, builder, type)
{
}

AudioContent::AudioContent()
{
  EditMode = ContentEditMode::ContentItem;
}

//---------------------------------------------------------------------- Sound Info

ZilchDefineType(SoundInfo, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(AudioContent);
  ZeroBindExpanded();

  ZilchBindGetterProperty(FileLength);
  ZilchBindGetterProperty(AudioChannels);
}

void SoundInfo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mFileLength, 0.0f);
  SerializeNameDefault(mAudioChannels, 0);
}

//---------------------------------------------------------------------- Factory

ContentItem* MakeAudioContent(ContentInitializer& initializer)
{
  AudioContent* content = new AudioContent();

  content->Filename = initializer.Filename;

  SoundInfo* info = new SoundInfo();
  content->AddComponent(info);

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
  ZilchBindFieldProperty(Streamed);
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
}

void SoundBuilder::BuildContent(BuildOptions& options)
{
  Status status;
  String sourceFile = FilePath::Combine(options.SourcePath, mOwner->Filename);
  String destFile = FilePath::Combine(options.OutputPath, BuildString(Name, SoundExtension));

  Audio::AudioFile audioFile;
  audioFile.OpenFile(status, sourceFile);

  if (status.Succeeded())
  {
    SoundInfo* info = mOwner->has(SoundInfo);
    if (info)
    {
      info->mFileLength = audioFile.FileLength;
      info->mAudioChannels = audioFile.Channels;
    }

    // This should probably be handled differently. The properties need to be saved because the object
    // is serialized before it is loaded, but BuildContent won't be called next time the engine starts,
    // so if we don't save the properties now they'll be lost.
    mOwner->SaveMetaFile();

    audioFile.WriteEncodedFile(status, destFile);

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

void SoundBuilder::CopyFile(BuildOptions& options)
{
  String filename = mOwner->Filename;
  String destFile = FilePath::Combine(options.OutputPath, BuildString(Name, SoundExtension));
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
  AddContentComponent<SoundInfo>(system);

  ContentTypeEntry audioContent(ZilchTypeId(AudioContent), MakeAudioContent);
  system->CreatorsByExtension["wav"] = audioContent;
  system->CreatorsByExtension["wv"] = audioContent;
  system->CreatorsByExtension["ogg"] = audioContent;
}

}//namespace Zero
