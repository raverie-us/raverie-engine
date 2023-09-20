// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
/// The choices for how to load and play an audio file.
/// <param name="StreamFromFile">The audio data will be read from the file and
/// decompressed as it plays.</param> <param name="StreamFromMemory">The
/// compressed audio data will be read into memory when the Sound resource is
/// loaded and will be decompressed as it plays.</param> <param
/// name="Uncompressed">The audio data will be decompressed and held in memory
/// when the Sound resource is loaded.</param> <param name="Auto">This will
/// choose whether to stream a file depending on its length. Files longer than
/// 30 seconds will be streamed from memory, and those longer than 1 minute will
/// be streamed from file.</param>
DeclareEnum4(AudioFileLoadType, StreamFromFile, StreamFromMemory, Uncompressed, Auto);

class AudioContent : public ContentComposition
{
public:
  RaverieDeclareType(AudioContent, TypeCopyMode::ReferenceType);

  AudioContent();
};

const String SoundExtension = ".snd";

class SoundBuilder : public DirectBuilderComponent
{
public:
  RaverieDeclareType(SoundBuilder, TypeCopyMode::ReferenceType);

  /// If Streamed is selected, or if Auto is selected and the file is longer
  /// than one minute, the sound file will be streamed from disk at runtime
  /// instead of loaded into memory. Streaming files can't be played multiple
  /// times simultaneously and can't use loop tails.
  AudioFileLoadType::Enum mFileLoadType;
  /// If true, the audio will be normalized when loaded so that the highest
  /// volume peak matches the MaxVolume value.
  bool mNormalize;
  /// The volume of the sound will be altered so that the highest volume peak
  /// matches this value. All audio samples will be adjusted equally.
  float mMaxVolume;

  SoundBuilder() :
      DirectBuilderComponent(0, SoundExtension, "Sound"),
      mFileLoadType(AudioFileLoadType::Auto),
      mNormalize(false),
      mMaxVolume(0.9f),
      mStreamed(false)
  {
  }

  // BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildContent(BuildOptions& options) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;

  // This should be removed at the next major version
  bool mStreamed;
};

} // namespace Raverie
