///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------ Reverb Node

  //************************************************************************************************
  ReverbNode::ReverbNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
    ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    TimeMSec(1000.0f),
    WetLevelValue(0.5f),
    OutputFinished(false)
  {
    if (!Threaded)
      SetSiblingNodes(new ReverbNode(status, name, ID, extInt, true), status);
  }

  //************************************************************************************************
  ReverbNode::~ReverbNode()
  {
    if (!Threaded)
      return;

    forRange(Reverb* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  float ReverbNode::GetTime()
  {
    return TimeMSec;
  }

  //************************************************************************************************
  void ReverbNode::SetTime(const float newTime)
  {
    TimeMSec = newTime;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ReverbNode::SetTime, (ReverbNode*)GetSiblingNode(),
          newTime));
    }
    else
    {
      forRange(Reverb* filter, FiltersPerListener.Values())
        filter->SetTime(newTime);
    }
  }

  //************************************************************************************************
  float ReverbNode::GetWetLevel()
  {
    return WetLevelValue;
  }

  //************************************************************************************************
  void ReverbNode::SetWetLevel(const float wetLevel)
  {
    WetLevelValue = wetLevel;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ReverbNode::SetWetLevel,
        (ReverbNode*)GetSiblingNode(), wetLevel));
    }
    else
    {
      forRange(Reverb* filter, FiltersPerListener.Values())
        filter->SetWetLevel(wetLevel);
    }
  }

  //************************************************************************************************
  void ReverbNode::InterpolateWetLevel(const float newWetLevel, const float time)
  {
    WetLevelValue = newWetLevel;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ReverbNode::InterpolateWetLevel,
        (ReverbNode*)GetSiblingNode(), newWetLevel, time));
    }
    else
    {
      forRange(Reverb* filter, FiltersPerListener.Values())
        filter->InterpolateWetLevel(newWetLevel, time);
    }
  }

  //************************************************************************************************
  bool ReverbNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input
    bool isThereInput = AccumulateInputSamples(bufferSize, numberOfChannels, listener);

    if (!isThereInput && OutputFinished)
      return false;

    // Check if the listener is in the map
    Reverb* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new Reverb;
      filter->SetTime(TimeMSec);
      filter->SetWetLevel(WetLevelValue);
      FiltersPerListener[listener] = filter;
    }

    bool hasOutput = filter->ProcessBuffer(InputSamples.Data(), outputBuffer->Data(),
      numberOfChannels, bufferSize);

    if (!isThereInput && !hasOutput)
      OutputFinished = true;
    else
      OutputFinished = false;

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void ReverbNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

}