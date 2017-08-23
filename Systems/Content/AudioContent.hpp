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

const String SoundExtension = ".snd";

class SoundBuilder : public DirectBuilderComponent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// If true, the sound file will be streamed from disk at runtime instead of loaded into memory. 
  /// Streaming files can't be played multiple times simultaneously and can't use loop tails.
  bool mStreamed;
  /// If true, the audio will be normalized when loaded so that the highest volume peak matches
  /// the MaxVolume value.
  bool mNormalize;
  /// The volume of the sound will be altered so that the highest volume peak matches this value.
  /// All audio samples will be adjusted equally.
  float mMaxVolume;

  SoundBuilder() :
    DirectBuilderComponent(0, SoundExtension, "Sound"), 
    mStreamed(false),
    mNormalize(false),
    mMaxVolume(0.9f)
  {}

  //BuilderComponent Interface
  void Generate(ContentInitializer& initializer) override;
  void Serialize(Serializer& stream) override;
  void BuildContent(BuildOptions& options) override;
  bool NeedsBuilding(BuildOptions& options) override;
  void BuildListing(ResourceListing& listing) override;

};

}//namespace Zero
