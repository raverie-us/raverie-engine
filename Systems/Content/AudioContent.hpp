///////////////////////////////////////////////////////////////////////////////
///
/// \file AudioContent.hpp
/// Declaration of the Audio content classes.
/// 
/// Authors: Chris Peters
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class AudioContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AudioContent();
};

const String WaveResourceName = "Sound";
const String SoundExtension = ".snd";

class SoundBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// If true, the sound file will be streamed from disk at runtime instead of loaded into memory. 
  /// Streaming files can’t be played multiple times simultaneously and can't use loop tails.
  bool Streamed;
  /// The length of the audio file, in seconds.
  float GetFileLength() { return mFileLength; }
  /// The number of audio channels in the file.
  int GetAudioChannels() { return mAudioChannels; }

  SoundBuilder() :
    DirectBuilderComponent(0, SoundExtension, WaveResourceName), 
    Streamed(false),
    mFileLength(0),
    mAudioChannels(0)
  {}

  //BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildContent(BuildOptions& options) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void CopyFile(BuildOptions& options);
  void BuildListing(ResourceListing& listing) override;

private:
  float mFileLength;
  int mAudioChannels;

};

}//namespace Zero
