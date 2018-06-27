///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
//-------------------------------------------------------------------------------------------- Sound

//**************************************************************************************************
String SoundToString(const BoundType* type, const byte* instance)
{
  Sound* sound = (Sound*)instance;
  return BuildString("Sound: ", sound->Name);
}

//**************************************************************************************************
ZilchDefineType(Sound, builder, type)
{
  type->ToStringFunction = SoundToString;
  type->Add(new SoundDisplay());
  ZeroBindDocumented();

  ZilchBindGetterProperty(Length);
  ZilchBindGetterProperty(Channels);
}

//**************************************************************************************************
void Sound::CreateAsset(Status& status, StringParam assetName, StringParam fileName, 
  AudioFileLoadType::Enum loadType)
{
  // If the load type is set to auto, determine the type based on the length of the file
  if (loadType == AudioFileLoadType::Auto)
  {
    // Open the audio file
    File audioFile;
    audioFile.Open(fileName, FileMode::Read, FileAccessPattern::Sequential);
    if (audioFile.IsOpen())
    {
      // Read in the header data
      FileHeader header;
      audioFile.Read(status, (byte*)&header, sizeof(header));
      // Close the file so it doesn't interfere with creating the asset
      audioFile.Close();

      if (status.Succeeded())
      {
        float fileLength = (float)header.SamplesPerChannel / (float)AudioConstants::cSystemSampleRate;

        if (fileLength < mStreamFromMemoryLength)
          loadType = AudioFileLoadType::Uncompressed;
        else if (fileLength < mStreamFromFileLength)
          loadType = AudioFileLoadType::StreamFromMemory;
        else
          loadType = AudioFileLoadType::StreamFromFile;
      }
    }
  }

  if (loadType == AudioFileLoadType::StreamFromFile || loadType == AudioFileLoadType::StreamFromMemory)
    mAsset = new StreamingSoundAsset(status, fileName, loadType, Name);
  else
    mAsset = new DecompressedSoundAsset(status, fileName, Name);

  if (status.Failed())
  {
    DoNotifyError("Error Creating Sound", status.Message);

    if (mAsset)
      SafeDelete(mAsset);
  }
}

//**************************************************************************************************
float Sound::GetLength()
{
  if (mAsset)
    return mAsset->mFileLength;
  else
    return 0.0f;
}

//**************************************************************************************************
int Sound::GetChannels()
{
  if (mAsset)
    return mAsset->mChannels;
  else
    return 0;
}

//**************************************************************************************************
bool Sound::GetStreaming()
{
  if (mAsset)
    return mAsset->mStreaming;
  else
    return false;
}

//------------------------------------------------------------------------------------ Sound Display

//**************************************************************************************************
ZilchDefineType(SoundDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundDisplay::GetName(HandleParam object)
{
  Sound* sound = object.Get<Sound*>(GetOptions::AssertOnNull);
  return BuildString("Sound: ", sound->Name);
}

//**************************************************************************************************
String SoundDisplay::GetDebugText(HandleParam object)
{
  return GetName(object);
}

//------------------------------------------------------------------------------------- Sound Loader

//**************************************************************************************************
HandleOf<Resource> SoundLoader::LoadFromBlock(ResourceEntry& entry)
{
  return LoadFromFile(entry);
}

//**************************************************************************************************
HandleOf<Resource> SoundLoader::LoadFromFile(ResourceEntry& entry)
{
  Sound* sound = new Sound();
  bool loaded = LoadSound(sound, entry);

  if (loaded)
  {
    SoundManager::GetInstance()->AddResource(entry, sound);
    return sound;
  }
  else
  {
    delete sound;
    return nullptr;
  }
}

//**************************************************************************************************
void SoundLoader::ReloadFromFile(Resource* resource, ResourceEntry& entry)
{
  Sound* sound = (Sound*)resource;

  LoadSound(sound, entry);
}

//**************************************************************************************************
bool SoundLoader::LoadSound(Sound* sound, ResourceEntry& entry)
{
  Zero::Status status;
  sound->CreateAsset(status, entry.Name, entry.FullPath.c_str(), mLoadType);

  if (status.Failed())
  {
    DoNotifyError("Failed Sound Load", status.Message.c_str());
    return false;
  }
  else
    return true;
}

//------------------------------------------------------------------------------------ Sound Manager

ImplementResourceManager(SoundManager, Sound);

//**************************************************************************************************
SoundManager::SoundManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("Sound", new SoundLoader(AudioFileLoadType::Uncompressed));
  AddLoader("StreamedSound", new SoundLoader(AudioFileLoadType::StreamFromFile));
  AddLoader("AutoStreamedSound", new SoundLoader(AudioFileLoadType::Auto));
  mCategory = "Sound";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Sounds", "*.wav;*.ogg"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.wav"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.ogg"));
  mCanReload = true;
  DefaultResourceName = "DefaultSound";
}

}//namespace Zero
