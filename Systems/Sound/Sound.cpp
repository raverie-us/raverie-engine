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
Sound::~Sound()
{
  if (mSoundAsset)
    mSoundAsset->SetExternalInterface(nullptr);
}

//**************************************************************************************************
void Sound::CreateAsset(Status& status, StringParam assetName, StringParam fileName, 
  AudioFileLoadType::Enum loadType)
{
  Audio::FileLoadType::Enum audioLoadType;
  if (loadType == AudioFileLoadType::StreamFromFile)
    audioLoadType = Audio::FileLoadType::Streamed;
  else if (loadType == AudioFileLoadType::Uncompressed)
    audioLoadType = Audio::FileLoadType::Decompressed;
  else
    audioLoadType = Audio::FileLoadType::Auto;

  mSoundAsset = new Audio::SoundAssetFromFile(status, fileName, audioLoadType, this);
  
  if (status.Succeeded())
  {
    mSoundAsset->mName = assetName;
  }
  else
  {
    DoNotifyError("Error Creating Sound", status.Message);

    if (mSoundAsset)
    {
      mSoundAsset->SetExternalInterface(nullptr);
      mSoundAsset = nullptr;
    }
  }
}

//**************************************************************************************************
float Sound::GetLength()
{
  if (mSoundAsset)
    return mSoundAsset->GetLengthOfFile();
  else
    return 0.0f;
}

//**************************************************************************************************
int Sound::GetChannels()
{
  if (mSoundAsset)
    return mSoundAsset->GetChannels();
  else
    return 0;
}

//**************************************************************************************************
bool Sound::GetStreaming()
{
  if (mSoundAsset)
    return mSoundAsset->GetStreaming();
  else
    return false;
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

  if (sound->mSoundAsset)
    sound->mSoundAsset->SetExternalInterface(nullptr);

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

//**************************************************************************************************
void SoundManager::SetSystem(Audio::AudioSystemInterface* system)
{
  mSystem = system;
}

}//namespace Zero
