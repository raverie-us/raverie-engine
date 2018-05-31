///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

inline static void LogAudioIoError(Zero::StringParam message, Zero::String* savedMessage = nullptr)
{
  ZPrint(Zero::String::Format(message.c_str(), "\n").c_str());
  if (savedMessage)
    *savedMessage = message;
}

class SDLAudioDevice
{
public:
  SDLAudioDevice() :
    mDeviceID(0),
    mSampleRate(0),
    mChannels(0),
    mCallback(nullptr),
    mCallbackData(nullptr),
    mType(StreamTypes::Output)
  {}

  SDL_AudioDeviceID mDeviceID;
  unsigned mSampleRate;
  unsigned mChannels;
  IOCallbackType* mCallback;
  void* mCallbackData;
  Zero::String mStreamTypeName;
  StreamTypes::Enum mType;
};

class AudioIoSdlData
{
public:
  AudioIoSdlData() {}

  SDLAudioDevice Streams[StreamTypes::Size];
};

//------------------------------------------------------------------- Audio Input Output using SDL

void SDLCallback(void* data, uint8* buffer, int lengthInBytes)
{
  SDLAudioDevice& device = *((SDLAudioDevice*)data);

  unsigned frames = lengthInBytes / 4 / device.mChannels;

  if (device.mType == StreamTypes::Output)
  {
    device.mCallback((float*)buffer, nullptr, frames, device.mCallbackData);
  }
  else
  {
    device.mCallback(nullptr, (float*)buffer, frames, device.mCallbackData);
  }
}

//************************************************************************************************
AudioInputOutput::AudioInputOutput()
{
  PlatformData = new AudioIoSdlData();
}

//************************************************************************************************
AudioInputOutput::~AudioInputOutput()
{
  delete PlatformData;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeAPI(Zero::String* resultMessage)
{
  ZPrint("Initializing SDL Audio\n");

  // Initialize SDL audio
  if (SDL_Init(SDL_INIT_AUDIO) == -1)
  {
    LogAudioIoError("Initializing SDL Audio was unsuccessful", resultMessage);
    return StreamStatus::ApiProblem;
  }

  return StreamStatus::Initialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeStream(StreamTypes::Enum whichStream,
  Zero::String* resultMessage)
{
  SDLAudioDevice& data = ((AudioIoSdlData*)PlatformData)->Streams[whichStream];
  if (whichStream == StreamTypes::Output)
    data.mStreamTypeName = "Output";
  else
  {
    data.mStreamTypeName = "Input";
    data.mType = StreamTypes::Input;
  }

  SDL_AudioSpec want, have;
  SDL_zero(want);

  want.freq = 48000;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = 4096;
  want.callback = SDLCallback;
  want.userdata = &data;

  ZPrint("Initializing audio %s stream\n", data.mStreamTypeName.c_str());

  int capture = 0;
  if (whichStream == StreamTypes::Input)
    capture = 1;

  data.mDeviceID = SDL_OpenAudioDevice(nullptr, capture, &want, &have, 
    SDL_AUDIO_ALLOW_CHANNELS_CHANGE || SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

  if (data.mDeviceID == 0)
  {
    LogAudioIoError("Could not open audio device", resultMessage);
    return StreamStatus::DeviceProblem;
  }

  data.mChannels = have.channels;
  data.mSampleRate = have.freq;

  ZPrint("Device name : %s\n", SDL_GetAudioDeviceName(data.mDeviceID, capture));
  ZPrint("Channels    : %d\n", data.mChannels);
  ZPrint("Sample rate : %d\n", data.mSampleRate);

  ZPrint("Audio %s stream successfully initialized\n", data.mStreamTypeName.c_str());

  return StreamStatus::Initialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StartStream(StreamTypes::Enum whichStream,
  Zero::String* resultMessage, IOCallbackType* callback, void* callbackData)
{
  SDLAudioDevice& data = ((AudioIoSdlData*)PlatformData)->Streams[whichStream];

  if (data.mDeviceID != 0)
  {
    data.mCallback = callback;
    data.mCallbackData = callbackData;

    SDL_PauseAudioDevice(data.mDeviceID, 0);

    ZPrint("Started audio %s stream\n", data.mStreamTypeName.c_str());

    return StreamStatus::Started;
  }
  else
  {
    LogAudioIoError(String::Format("Unable to start audio %s stream", data.mStreamTypeName), resultMessage);
    
    return StreamStatus::Uninitialized;
  }
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StopStream(StreamTypes::Enum whichStream, Zero::String* resultMessage)
{
  SDLAudioDevice& data = ((AudioIoSdlData*)PlatformData)->Streams[whichStream];

  if (data.mDeviceID != 0)
  {
    SDL_CloseAudioDevice(data.mDeviceID);
    data.mDeviceID = 0;

    ZPrint("Stopped audio %s stream\n", data.mStreamTypeName.c_str());
  }
  else
  {
    ZPrint("Unable to stop audio %s stream: not open\n", data.mStreamTypeName.c_str());
  }

  return StreamStatus::Stopped;
}

//************************************************************************************************
void AudioInputOutput::ShutDownAPI()
{
  // Shut down SDL audio
  SDL_QuitSubSystem(SDL_INIT_AUDIO);

  ZPrint("SDL Audio was shut down\n");
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamChannels(StreamTypes::Enum whichStream)
{
  return ((AudioIoSdlData*)PlatformData)->Streams[whichStream].mChannels;
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  return ((AudioIoSdlData*)PlatformData)->Streams[whichStream].mSampleRate;
}

}
