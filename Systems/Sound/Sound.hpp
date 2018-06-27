///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//-------------------------------------------------------------------------------------------- Sound

/// The resource for a single audio file.
class Sound : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  
  Sound() : mAsset(nullptr) {}

  /// The length of the audio file, in seconds.
  float GetLength();
  /// The number of audio channels in the file.
  int GetChannels();
  /// This will be true if the audio file is set to stream from disk.
  bool GetStreaming();

// Internals
  HandleOf<SoundAsset> mAsset;
  
  const float mStreamFromMemoryLength = 30.0f;
  const float mStreamFromFileLength = 60.0f;

  void CreateAsset(Status& status, StringParam assetName, StringParam fileName, 
    AudioFileLoadType::Enum loadType);
};

//------------------------------------------------------------------------------------ Sound Display

class SoundDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

//------------------------------------------------------------------------------------- Sound Loader

class SoundLoader : public ResourceLoader
{
public:
  SoundLoader(AudioFileLoadType::Enum loadType) : mLoadType(loadType) {}

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry) override;
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry) override;
  void ReloadFromFile(Resource* resource, ResourceEntry& entry) override;
  bool LoadSound(Sound* sound, ResourceEntry& entry);

  AudioFileLoadType::Enum mLoadType;
};

//------------------------------------------------------------------------------------ Sound Manager

class SoundManager : public ResourceManager
{
public:
  DeclareResourceManager(SoundManager, Sound);
  SoundManager(BoundType* resourceType);

};  


}//namespace Zero
