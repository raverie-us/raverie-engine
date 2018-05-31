////////////////////////////////////////////////////////////////////////////////
/// Authors:  Andrea Ellinger, Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//************************************************************************************************
AudioInputOutput::AudioInputOutput()
{
  Error("Not implemented");
}

//************************************************************************************************
AudioInputOutput::~AudioInputOutput()
{
  Error("Not implemented");
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeAPI(Zero::String* resultMessage)
{
  Error("Not implemented");
  return StreamStatus::Uninitialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeStream(StreamTypes::Enum whichStream,
                                                      Zero::String* resultMessage)
{
  Error("Not implemented");
  return StreamStatus::Uninitialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StartStream(StreamTypes::Enum whichStream,
                                                 Zero::String* resultMessage, IOCallbackType* callback, void* callbackData)
{
  Error("Not implemented");
  return StreamStatus::Uninitialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StopStream(StreamTypes::Enum whichStream, Zero::String* resultMessage)
{
  Error("Not implemented");
  return StreamStatus::Uninitialized;
}

//************************************************************************************************
void AudioInputOutput::ShutDownAPI()
{
  Error("Not implemented");
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamChannels(StreamTypes::Enum whichStream)
{
  Error("Not implemented");
  return 0;
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  Error("Not implemented");
  return 0;
}

//************************************************************************************************
MidiInput::MidiInput() :
  mOnMidiData(nullptr),
  mHandle(nullptr),
  mUserData(nullptr)
{
  Error("Not implemented");
}

//************************************************************************************************
MidiInput::~MidiInput()
{
  Error("Not implemented");
}

} // namespace Zero