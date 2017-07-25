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
  ZilchBindGetterProperty(SampleCount);
  ZilchBindGetterProperty(SampleRate);
}

//**************************************************************************************************
Sound::~Sound()
{
  if (mSoundAsset)
    mSoundAsset->SetExternalInterface(nullptr);
}

//**************************************************************************************************
void Sound::CreateAsset(Status& status, StringParam assetName, StringParam fileName, bool streaming)
{
  mSoundAsset = new Audio::SoundAssetFromFile(status, fileName, streaming, this);
  
  if (status.Succeeded())
  {
    mSoundAsset->Name = assetName;
    mStreaming = streaming;
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
int Sound::GetSampleCount()
{
  if (mSoundAsset)
    return mSoundAsset->GetNumberOfSamples();
  else
    return 0;
}

//**************************************************************************************************
int Sound::GetSampleRate()
{
  if (mSoundAsset)
    return mSoundAsset->GetSampleRate();
  else
    return 0;
}

//**************************************************************************************************
AudioFileFormats::Enum Sound::GetFormat()
{
  if (mSoundAsset)
  {
    Audio::AudioFileTypes type = mSoundAsset->GetFileType();
    switch (type)
    {
    case Audio::WAV_Type:
      return AudioFileFormats::WAV;
      break;
    case Audio::OGG_Type:
      return AudioFileFormats::OGG;
      break;
    default:
      return AudioFileFormats::WAV;
      break;
    }
  }
  else
    return AudioFileFormats::WAV;
}

//**************************************************************************************************
bool Sound::GetStreaming()
{
  return mStreaming;
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
  sound->CreateAsset(status, entry.Name, entry.FullPath.c_str(), mStreamed);

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
  AddLoader("Sound", new SoundLoader(false));
  AddLoader("StreamedSound", new SoundLoader(true));
  mCategory = "Sound";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("All Sounds", "*.wav;*.wv;*.ogg"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.wav"));
  mOpenFileFilters.PushBack(FileDialogFilter("*.wv"));
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
