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
#include "AudioEngine/FileAccess.h"

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
  ZilchBindFieldProperty(Compressed)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(CompressionQuality)->Add(new EditorRange(2, 10, 1))->ZeroFilterBool(Compressed);
  ZilchBindGetterProperty(FileLength);
  ZilchBindGetterProperty(AudioChannels);
  ZilchBindGetterProperty(SampleRate);
  ZilchBindGetterProperty(SavedFormat);
}

void SoundBuilder::Generate(ContentInitializer& initializer)
{
  mResourceId = GenerateUniqueId64();
  Name = initializer.Name;

  Compressed = false;
  Streamed = false;
}

void SoundBuilder::Serialize(Serializer& stream)
{
  SerializeName(Name);
  SerializeName(mResourceId);
  SerializeName(Compressed);
  SerializeNameDefault(Streamed, false);
  SerializeNameDefault(CompressionQuality, 3.0f);
  SerializeNameDefault(mFileLength, 0.0f);
  SerializeNameDefault(mAudioChannels, 0);
  SerializeNameDefault(mSampleRate, 0);
  SerializeEnumNameDefault(AudioFileFormats, mSavedFormat, AudioFileFormats::WAV);
}

void SoundBuilder::BuildContent(BuildOptions& options)
{
  String filename = mOwner->Filename;
  String extension = FilePath::GetExtension(filename);

  Status status;
  Audio::AudioData data = Audio::FileAccess::GetFileData(status, mOwner->GetFullPath());
  data.CloseFile();

  if (status.Failed())
  {
    options.Failure = true;
    options.Message = status.Message;
    return;
  }
  
  mFileLength = (float)data.TotalSamples / (float)data.SampleRate / (float)data.Channels;
  mAudioChannels = data.Channels;
  mSampleRate = data.SampleRate;
  
  if (data.Type == Audio::Type_OGG)
  {
    mSavedFormat = AudioFileFormats::OGG;
    CopyFile(options);
  }
  else if (data.Type == Audio::Type_WAV)
  {
    // Already processed, don't need to check anything
    if (extension == "snd")
    {
      mSavedFormat = AudioFileFormats::WAV;
      CopyFile(options);
    }
    else
    {
      bool readableFile(true);
      if (!Compressed)
      {
        // We need to check the WAV file to make sure it's a readable format. Otherwise it will 
        // need to be translated to OGG format.

        // Check the audio format and bit rate (must be PCM and 16 or 24 bit)
        if (data.WavFormat != 1 || !(data.BytesPerSample == 2 || data.BytesPerSample == 3))
          readableFile = false;
      }

      //WAV files will be compressed to ogg
      if (Compressed || !readableFile)
      {
        mSavedFormat = AudioFileFormats::OGG;

        // Make sure quality number is within bounds
        if (CompressionQuality < 2)
          CompressionQuality = 2;
        else if (CompressionQuality > 10)
          CompressionQuality = 10;

        // "Because this is specific to windows, I left ZFS in because the
        //  usage here is super confusing and I don't want to break it"
        //                                                          -Trevor
#define ZFS "\\"
        String commandLineMain("\"%s" ZFS "oggenc2.exe\" \"%s\" -q%d -Q -o \"%s.snd\"");
#undef ZFS
        String commandLine = String::Format(commandLineMain.c_str(),
          options.ToolPath.c_str(),
          FilePath::Combine(options.SourcePath, filename).c_str(),
          (int)CompressionQuality,
          FilePath::Combine(options.OutputPath, Name).c_str());

        SimpleProcess process;
        process.ExecProcess("Process Ogg", commandLine.c_str(),
          options.BuildTextStream);
        int exitCode = process.WaitForClose();

        if (exitCode != 0)
        {
          String errMsg = String::Format("Failed to process sound file %s. It may be in a compressed format that we are unable to read.", filename.c_str());
          options.Failure = true;
          options.Message = errMsg;
        }
      }
      else
      {
        mSavedFormat = AudioFileFormats::WAV;
        CopyFile(options);
      }
    }
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
