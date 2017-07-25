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
DeclareEnum2(AudioFileFormats, WAV, OGG);

/// Audio content is raw wave data and is compressed
/// into various formats.
class AudioContent : public ContentComposition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AudioContent();
};

const String WaveResourceName = "Sound";
const String SoundExtension = ".snd";

/// Sound builder loads wave data and compresses into
/// an engine loadable format (ogg, etc)
class SoundBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// If true, WAV files will be converted into OGG files when they are loaded.
  bool Compressed;
  /// If true, the sound file will be streamed from disk at runtime instead of loaded into memory. 
  /// Streaming files can’t be played multiple times simultaneously and can't use loop tails.
  bool Streamed;
  /// The quality of the .ogg conversion, where 10 is the highest possible quality with the largest memory usage.
  float CompressionQuality;
  /// The length of the audio file, in seconds.
  float GetFileLength() { return mFileLength; }
  /// The number of audio channels in the file.
  int GetAudioChannels() { return mAudioChannels; }
  /// The audio format that the file has been saved in. WAV files that are compressed or are in a format
  /// which can't be processed by the audio engine will be saved in OGG format.
  AudioFileFormats::Enum GetSavedFormat() { return mSavedFormat; }
  /// The sample rate (samples per second) of the audio file.
  int GetSampleRate() { return mSampleRate; }

  SoundBuilder() :
    DirectBuilderComponent(0, SoundExtension, WaveResourceName), 
    CompressionQuality(3),
    Compressed(false),
    Streamed(false),
    mFileLength(0),
    mAudioChannels(0),
    mSampleRate(0)
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
  AudioFileFormats::Enum mSavedFormat;
  int mSampleRate;

};

}//namespace Zero
