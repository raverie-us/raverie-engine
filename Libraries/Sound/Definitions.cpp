///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
void LogAudioIoError(StringParam message, String* savedMessage)
{
  ZPrint(String::Format(message.c_str(), "\n").c_str());
  if (savedMessage)
    *savedMessage = message;
}

//**************************************************************************************************
bool AudioConstants::IsWithinLimit(float valueToCheck, float centralValue, float limit)
{
  if (valueToCheck > centralValue + limit || valueToCheck < centralValue - limit)
    return false;
  else
    return true;
}

//**************************************************************************************************
void AudioConstants::AppendToBuffer(BufferType* destinationBuffer, const BufferType& sourceBuffer,
  unsigned sourceStartIndex, unsigned numberOfSamples)
{
  unsigned destIndex = destinationBuffer->Size();
  destinationBuffer->Resize(destinationBuffer->Size() + numberOfSamples);
  memcpy(destinationBuffer->Data() + destIndex, sourceBuffer.Data() + sourceStartIndex,
    sizeof(float) * numberOfSamples);
}

//**************************************************************************************************
float AudioConstants::PitchToSemitones(float pitch)
{
  if (pitch == 0)
    return 0.0f;
  else
    return 3986.0f * Math::Log10(Math::Exp2(pitch)) / 100.0f;
}

//**************************************************************************************************
float AudioConstants::SemitonesToPitch(float semitone)
{
  return Math::Log2(Math::Exp2(semitone / 12.0f));
}

//**************************************************************************************************
float AudioConstants::VolumeToDecibels(float volume)
{
  if (volume == 0.0f)
    return -100.0f;
  else
  {
    float decibels = 20.0f * Math::Log10(volume);
    if (decibels < -100.0f)
      decibels = -100.0f;
    return decibels;
  }
}

//**************************************************************************************************
float AudioConstants::DecibelsToVolume(float decibels)
{
  return Math::Pow(10.0f, decibels / 20.0f);
}


} // namespace Zero
