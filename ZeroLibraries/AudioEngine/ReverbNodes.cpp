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
    WetPercent(50.0f),
    OutputFinished(false)
  {
    if (!Threaded)
      SetSiblingNodes(new ReverbNode(status, name, ID, extInt, true), status);
  }

  //************************************************************************************************
  ReverbNode::~ReverbNode()
  {
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
  float ReverbNode::GetWetPercent()
  {
    return WetPercent;
  }

  //************************************************************************************************
  void ReverbNode::SetWetPercent(const float newPercent)
  {
    WetPercent = newPercent;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ReverbNode::SetWetPercent,
        (ReverbNode*)GetSiblingNode(), newPercent));
    }
    else
    {
      forRange(Reverb* filter, FiltersPerListener.Values())
        filter->SetWetPercent(newPercent);
    }
  }

  //************************************************************************************************
  void ReverbNode::InterpolateWetPercent(const float newPercent, const float time)
  {
    WetPercent = newPercent;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ReverbNode::InterpolateWetPercent,
        (ReverbNode*)GetSiblingNode(), newPercent, time));
    }
    else
    {
      forRange(Reverb* filter, FiltersPerListener.Values())
        filter->InterpolateWetPercent(newPercent, time);
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
      filter->SetWetPercent(WetPercent);
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
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

}