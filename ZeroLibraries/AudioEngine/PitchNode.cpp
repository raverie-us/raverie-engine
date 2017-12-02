///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------- Pitch Node

  //************************************************************************************************
  PitchNode::PitchNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    PitchCents(0)
  {
    if (!Threaded)
      SetSiblingNodes(new PitchNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  void PitchNode::SetPitch(const int pitchCents, const float timeToInterpolate)
  {
    if (!Threaded)
    {
      PitchCents = pitchCents;
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&PitchNode::SetPitch, (PitchNode*)GetSiblingNode(),
          pitchCents, timeToInterpolate));
    }
    else
    {
      float newFactor = Math::Pow(2.0f, pitchCents / 1200.0f);
      PitchObject.SetPitchFactor(newFactor, timeToInterpolate);
    }
  }

  //************************************************************************************************
  int PitchNode::GetPitch()
  {
    return PitchCents;
  }

  //************************************************************************************************
  bool PitchNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // If no inputs, return
    if (!HasInputs())
      return false;

    // If this is the first request, calculate the buffer size
    if (firstRequest)
      PitchObject.CalculateBufferSize(outputBuffer->Size(), numberOfChannels);
    // Otherwise reset variables to the beginning of the mix
    else
      PitchObject.ResetToStartOfMix();

    // If no valid input, set the last samples to zero and return
    if (!AccumulateInputSamples(PitchObject.GetInputSampleCount(), numberOfChannels, listener))
    {
      PitchObject.ResetLastSamples();
      return false;
    }

    PitchObject.ProcessBuffer(&InputSamples, outputBuffer);

    return true;
  }

}
