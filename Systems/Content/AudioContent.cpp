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

  // Create the AudiFile object and open the source file
  Audio::AudioFile audioFile;
  audioFile.OpenFile(status, sourceFile);

  if (status.Succeeded())
  {
    // Encode the file and write it out to disk
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

  ContentTypeEntry audioContent(ZilchTypeId(AudioContent), MakeAudioContent);
  system->CreatorsByExtension["wav"] = audioContent;
  system->CreatorsByExtension["ogg"] = audioContent;
}

}//namespace Zero
