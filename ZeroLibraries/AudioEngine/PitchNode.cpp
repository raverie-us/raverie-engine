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
  PitchNode::PitchNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    PitchSemitones(0)
  {
    if (!Threaded)
      SetSiblingNodes(new PitchNode(name, ID, nullptr, true));
  }

  //************************************************************************************************
  void PitchNode::SetPitch(const float pitchSemitones, const float timeToInterpolate)
  {
    if (!Threaded)
    {
      PitchSemitones = pitchSemitones;
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&PitchNode::SetPitch, (PitchNode*)GetSiblingNode(),
          pitchSemitones, timeToInterpolate));
    }
    else
    {
      float newFactor = Math::Pow(2.0f, pitchSemitones / 12.0f);
      PitchObject.SetPitchFactor(newFactor, timeToInterpolate);
    }
  }

  //************************************************************************************************
  float PitchNode::GetPitch()
  {
    return PitchSemitones;
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
