///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //-------------------------------------------------------------------------- Microphone Input Node

  //************************************************************************************************
  bool MicrophoneInputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (gAudioSystem->InputBuffer.Empty())
      return false;

    // Start with the number of samples in the system's input buffer
    unsigned samplesToCopy = gAudioSystem->InputBuffer.Size();
    // If this is larger than the output buffer, adjust the amount
    if (samplesToCopy > outputBuffer->Size())
      samplesToCopy = outputBuffer->Size();

    // Copy the samples from the input buffer to the output buffer
    memcpy(outputBuffer->Data(), gAudioSystem->InputBuffer.Data(), sizeof(float) * samplesToCopy);

    // If we copied less samples than the output buffer size, fill the rest with zeros
    if (samplesToCopy < outputBuffer->Size())
      memset(outputBuffer->Data() + samplesToCopy, 0, sizeof(float) * (outputBuffer->Size() - samplesToCopy));

    return true;
  }

}