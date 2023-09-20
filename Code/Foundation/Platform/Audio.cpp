// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

AudioInputOutput::AudioInputOutput()
{
}

AudioInputOutput::~AudioInputOutput()
{
}

StreamStatus::Enum AudioInputOutput::InitializeAPI(Raverie::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::InitializeStream(StreamTypes::Enum whichStream, Raverie::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::StartStream(StreamTypes::Enum whichStream,
                                                 Raverie::String* resultMessage,
                                                 IOCallbackType* callback,
                                                 void* callbackData)
{
  return StreamStatus::Uninitialized;
}

StreamStatus::Enum AudioInputOutput::StopStream(StreamTypes::Enum whichStream, Raverie::String* resultMessage)
{
  return StreamStatus::Uninitialized;
}

void AudioInputOutput::ShutDownAPI()
{
}

unsigned AudioInputOutput::GetStreamChannels(StreamTypes::Enum whichStream)
{
  return 0;
}

unsigned AudioInputOutput::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  return 0;
}

float AudioInputOutput::GetBufferSizeMultiplier()
{
  return 1.0f;
}

} // namespace Raverie
