///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{

  //---------------------------------------------------------------------------------- Low Pass Node

  //************************************************************************************************
  LowPassNode::LowPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    CutoffFrequency(20001.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new LowPassNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  LowPassNode::~LowPassNode()
  {
    forRange(LowPassFilter* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  float LowPassNode::GetCutoffFrequency()
  {
    return CutoffFrequency;
  }

  //************************************************************************************************
  bool LowPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // If the filter is turned off, return
    if (CutoffFrequency >= 20000.0f)
    {
      // Move the input samples to the output buffer
      InputSamples.Swap(*outputBuffer);
      return true;
    }

    // Check if the listener is in the map
    LowPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new LowPassFilter;
      filter->SetCutoffFrequency(CutoffFrequency);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
      filter->ProcessSample(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void LowPassNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      // If there is still another listener, we need to combine the history of the filters to avoid clicks
      if (FiltersPerListener.Size() > 1)
      {
        LowPassFilter* filter = FiltersPerListener[listener];
        FiltersPerListener.Erase(listener);
        FilterMapType::valuerange range = FiltersPerListener.All();
        filter->MergeWith(*range.Front());
      }

      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //************************************************************************************************
  void LowPassNode::SetCutoffFrequency(const float freq)
  {
    CutoffFrequency = freq;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&LowPassNode::SetCutoffFrequency, 
          (LowPassNode*)GetSiblingNode(), freq));
    }
    else
    {
      forRange(LowPassFilter* filter, FiltersPerListener.Values())
        filter->SetCutoffFrequency(freq);
    }
  }

  //---------------------------------------------------------------------------------- HighPass Node

  //************************************************************************************************
  HighPassNode::HighPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    CutoffFrequency(10.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new HighPassNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  HighPassNode::~HighPassNode()
  {
    forRange(HighPassFilter* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  float HighPassNode::GetCutoffFrequency()
  {
    return CutoffFrequency;
  }

  //************************************************************************************************
  void HighPassNode::SetCutoffFrequency(const float freq)
  {
    CutoffFrequency = freq;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&HighPassNode::SetCutoffFrequency,
        (HighPassNode*)GetSiblingNode(), freq));
    }
    else
    {
      forRange(HighPassFilter* filter, FiltersPerListener.Values())
        filter->SetCutoffFrequency(freq);
    }
  }

  //************************************************************************************************
  bool HighPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // If the filter is turned off, return
    if (CutoffFrequency >= 20000.0f)
    {
      // Move the input samples to the output buffer
      InputSamples.Swap(*outputBuffer);
      return true;
    }

    // Check if the listener is in the map
    HighPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new HighPassFilter;
      filter->SetCutoffFrequency(CutoffFrequency);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
      filter->ProcessSample(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void HighPassNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      // If there is still another listener, we need to combine the history of the filters to avoid clicks
      if (FiltersPerListener.Size() > 1)
      {
        HighPassFilter* filter = FiltersPerListener[listener];
        FiltersPerListener.Erase(listener);
        FilterMapType::valuerange range = FiltersPerListener.All();
        filter->MergeWith(*range.Front());
      }

      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //--------------------------------------------------------------------------------- Band Pass Node

  //************************************************************************************************
  BandPassNode::BandPassNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    CentralFrequency(0.0f), 
    Quality(0.669f)
  {
    if (!Threaded)
      SetSiblingNodes(new BandPassNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  BandPassNode::~BandPassNode()
  {
    forRange(BandPassFilter* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  float BandPassNode::GetCentralFrequency()
  {
    return CentralFrequency;
  }

  //************************************************************************************************
  void BandPassNode::SetCentralFrequency(const float frequency)
  {
    CentralFrequency = frequency;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&BandPassNode::SetCentralFrequency, 
            (BandPassNode*)GetSiblingNode(), frequency));
    }
    else
    {
      forRange(BandPassFilter* filter, FiltersPerListener.Values())
        filter->SetFrequency(frequency);
    }
  }

  //************************************************************************************************
  float BandPassNode::GetQuality()
  {
    return Quality;
  }

  //************************************************************************************************
  void BandPassNode::SetQuality(const float Q)
  {
    Quality = Q;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&BandPassNode::SetQuality, 
          (BandPassNode*)GetSiblingNode(), Q));
    }
    else
    {
      forRange(BandPassFilter* filter, FiltersPerListener.Values())
        filter->SetQuality(Q);
    }
  }

  //************************************************************************************************
  bool BandPassNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Check if the listener is in the map
    BandPassFilter* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new BandPassFilter;
      filter->SetFrequency(CentralFrequency);
      filter->SetQuality(Quality);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
      filter->ProcessSample(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void BandPassNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      // If there is still another listener, we need to combine the history of the filters to avoid clicks
      if (FiltersPerListener.Size() > 1)
      {
        BandPassFilter* filter = FiltersPerListener[listener];
        FiltersPerListener.Erase(listener);
        FilterMapType::valuerange range = FiltersPerListener.All();
        filter->MergeWith(*range.Front());
      }

      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //------------------------------------------------------------------------------------- Delay Node

  //************************************************************************************************
  DelayNode::DelayNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    DelayMSec(100.0f), 
    FeedbackPct(0),
    WetPercent(50.0f),
    HasHadInput(false)
  {
    if (!Threaded)
      SetSiblingNodes(new DelayNode(status, name, ID, extInt, true), status);
  }

  //************************************************************************************************
  DelayNode::~DelayNode()
  {
    forRange(DelayLine* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  void DelayNode::SetDelayMSec(const float delay)
  {
    DelayMSec = delay;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&DelayNode::SetDelayMSec, 
          (DelayNode*)GetSiblingNode(), delay));
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetDelayMSec(delay);
    }
  }

  //************************************************************************************************
  float DelayNode::GetFeedbackPct()
  {
    return FeedbackPct;
  }

  //************************************************************************************************
  float DelayNode::GetDelayMSec()
  {
    return DelayMSec;
  }

  //************************************************************************************************
  void DelayNode::SetFeedbackPct(const float feedbackPct)
  {
    FeedbackPct = feedbackPct;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&DelayNode::SetFeedbackPct, 
          (DelayNode*)GetSiblingNode(), feedbackPct));
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetFeedbackPct(feedbackPct);
    }
  }

  //************************************************************************************************
  float DelayNode::GetWetLevelPct()
  {
    return WetPercent;
  }

  //************************************************************************************************
  void DelayNode::SetWetLevelPct(const float wetPct)
  {
    WetPercent = wetPct;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&DelayNode::SetWetLevelPct,
          (DelayNode*)GetSiblingNode(), wetPct));
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetWetLevelPct(wetPct);
    }
  }

  //************************************************************************************************
  void DelayNode::InterpolateWetLevelPct(const float percent, const float time)
  {
    WetPercent = percent;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&DelayNode::InterpolateWetLevelPct, 
            (DelayNode*)GetSiblingNode(), percent, time));
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->InterpolateWetLevelPct(percent, time);
    }
  }

  //************************************************************************************************
  bool DelayNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned outputBufferSize = outputBuffer->Size();

    // Get input
    bool isThereInput = AccumulateInputSamples(outputBufferSize, numberOfChannels, listener);

    // If no input and delayed output is done, return
    if (!isThereInput && !HasHadInput)
      return false;

    // Check if the listener is in the map
    DelayLine* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new DelayLine;
      filter->SetDelayMSec(DelayMSec);
      filter->SetFeedbackPct(FeedbackPct);
      filter->SetWetLevelPct(WetPercent);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    bool outputCheck(false);
    filter->ProcessBuffer(InputSamples.Data(), outputBuffer->Data(), numberOfChannels, outputBufferSize);
    for (unsigned i = 0; i < outputBufferSize && !outputCheck; ++i)
    {
      if ((*outputBuffer)[i] != 0.0f)
        outputCheck = true;
    }

    if (!outputCheck && !isThereInput)
      HasHadInput = false;
    else
      HasHadInput = true;

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void DelayNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //------------------------------------------------------------------------------ Flanger Node Data

  //************************************************************************************************
  FlangerNode::Data::Data(float frequency, float feedback) :
    Delay(new DelayLine),
    LFO(new Oscillator)
  {
    Delay->SetDelayMSec(0);
    Delay->SetFeedbackPct(feedback);
    Delay->SetWetLevelPct(50.0f);
    LFO->SetFrequency(frequency);
    LFO->SetPolarity(Oscillator::Unipolar);
    LFO->SetNoteOn(true);
  }

  //************************************************************************************************
  FlangerNode::Data::~Data()
  {
    delete Delay;
    delete LFO;
  }

  //----------------------------------------------------------------------------------- Flanger Node

  //************************************************************************************************
  FlangerNode::FlangerNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    MaxDelay(5.0f),
    ModFrequency(0.18f), 
    FeedbackPct(0), 
    OscillatorType(Sine)
  {
    if (!Threaded)
      SetSiblingNodes(new FlangerNode(status, name, ID, extInt, true), status);
  }

  //************************************************************************************************
  FlangerNode::~FlangerNode()
  {
    if (Threaded)
    {
      forRange(Data* data, FiltersPerListener.Values())
        delete data;
    }
  }

  //************************************************************************************************
  float FlangerNode::GetMaxDelayMSec()
  {
    return MaxDelay;
  }

  //************************************************************************************************
  void FlangerNode::SetMaxDelayMSec(const float delay)
  {
    MaxDelay = delay;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&FlangerNode::SetMaxDelayMSec, 
          (FlangerNode*)GetSiblingNode(), delay));
  }

  //************************************************************************************************
  float FlangerNode::GetModFrequency()
  {
    return ModFrequency;
  }

  //************************************************************************************************
  void FlangerNode::SetModFrequency(const float frequency)
  {
    ModFrequency = frequency;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&FlangerNode::SetModFrequency, 
            (FlangerNode*)GetSiblingNode(), frequency));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetFrequency(frequency);
    }
  }

  //************************************************************************************************
  float FlangerNode::GetFeedbackPct()
  {
    return FeedbackPct;
  }

  //************************************************************************************************
  void FlangerNode::SetFeedbackPct(const float percent)
  {
    FeedbackPct = percent;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&FlangerNode::SetFeedbackPct, 
            (FlangerNode*)GetSiblingNode(), percent));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->Delay->SetFeedbackPct(percent);
    }
  }

  //************************************************************************************************
  int FlangerNode::GetOscillatorType()
  {
    return OscillatorType;
  }

  //************************************************************************************************
  void FlangerNode::SetOscillatorType(const OscillatorTypes type)
  {
    OscillatorType = type;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&FlangerNode::SetOscillatorType, 
            (FlangerNode*)GetSiblingNode(), type));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetType((Oscillator::Types)type);
    }
  }

  //************************************************************************************************
  bool FlangerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Check if the listener is in the map
    Data* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new Data(ModFrequency, FeedbackPct);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
    {
      filter->Delay->SetDelayMSec(filter->LFO->GetNextSample() * MaxDelay);

      filter->Delay->ProcessBuffer(InputSamples.Data() + frame, outputBuffer->Data() + frame, 
        numberOfChannels, numberOfChannels);
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void FlangerNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //------------------------------------------------------------------------------- Chorus Node Data

  //************************************************************************************************
  ChorusNode::Data::Data(float frequency, float minDelay, float feedback) :
    Delay(new DelayLine),
    LFO(new Oscillator)
  {
    Delay->SetDelayMSec(minDelay);
    Delay->SetFeedbackPct(feedback);
    Delay->SetWetLevelPct(50.0f);
    LFO->SetFrequency(frequency);
    LFO->SetPolarity(Oscillator::Unipolar);
    LFO->SetNoteOn(true);
  }

  //************************************************************************************************
  ChorusNode::Data::~Data()
  {
    delete Delay;
    delete LFO;
  }

  //------------------------------------------------------------------------------------ Chorus Node

  //************************************************************************************************
  ChorusNode::ChorusNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface *extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    MinDelay(5.0f),
    MaxDelay(20.0f), 
    ModFrequency(0.1f), 
    FeedbackPct(0), 
    OscillatorType(Sine),
    ChorusOffset(40.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new ChorusNode(status, name, ID, extInt, true), status);
  }

  //************************************************************************************************
  ChorusNode::~ChorusNode()
  {
    if (Threaded)
    {
      forRange(Data* data, FiltersPerListener.Values())
        delete data;
    }
  }

  //************************************************************************************************
  float ChorusNode::GetMaxDelayMSec()
  {
    return MaxDelay;
  }

  //************************************************************************************************
  void ChorusNode::SetMaxDelayMSec(const float delay)
  {
    MaxDelay = delay;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::SetMaxDelayMSec, 
        (ChorusNode*)GetSiblingNode(), delay));
  }

  //************************************************************************************************
  float ChorusNode::GetMinDelayMSec()
  {
    return MinDelay;
  }

  //************************************************************************************************
  void ChorusNode::SetMinDelayMSec(const float delay)
  {
    MinDelay = delay;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::SetMaxDelayMSec, 
        (ChorusNode*)GetSiblingNode(), delay));
  }

  //************************************************************************************************
  float ChorusNode::GetModFrequency()
  {
    return ModFrequency;
  }

  //************************************************************************************************
  void ChorusNode::SetModFrequency(const float frequency)
  {
    ModFrequency = frequency;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::SetModFrequency, 
          (ChorusNode*)GetSiblingNode(), frequency));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetFrequency(frequency);
    }
  }

  //************************************************************************************************
  float ChorusNode::GetFeedbackPct()
  {
    return FeedbackPct;
  }

  //************************************************************************************************
  void ChorusNode::SetFeedbackPct(const float percent)
  {
    FeedbackPct = percent;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::SetFeedbackPct, 
          (ChorusNode*)GetSiblingNode(), percent));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->Delay->SetFeedbackPct(percent);
    }
  }

  //************************************************************************************************
  OscillatorTypes ChorusNode::GetOscillatorType()
  {
    return OscillatorType;
  }

  //************************************************************************************************
  void ChorusNode::SetOscillatorType(const OscillatorTypes type)
  {
    OscillatorType = type;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::SetOscillatorType, 
          (ChorusNode*)GetSiblingNode(), type));
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetType((Oscillator::Types)type);
    }
  }

  //************************************************************************************************
  float ChorusNode::GetOffsetMSec()
  {
    return ChorusOffset;
  }

  //************************************************************************************************
  void ChorusNode::SetOffsetMSec(const float offset)
  {
    ChorusOffset = offset;

    if (!Threaded && GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ChorusNode::ChorusOffset, 
        (ChorusNode*)GetSiblingNode(), offset));
  }

  //************************************************************************************************
  bool ChorusNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Check if the listener is in the map
    Data* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new Data(ModFrequency, MinDelay, FeedbackPct);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
    {
      filter->Delay->SetDelayMSec((filter->LFO->GetNextSample() * (MaxDelay - MinDelay)) 
        + (MinDelay + ChorusOffset));

      filter->Delay->ProcessBuffer(InputSamples.Data() + frame, outputBuffer->Data() + frame, 
        numberOfChannels, numberOfChannels);
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void ChorusNode::RemoveListener(ListenerNode* listener)
  {
    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //--------------------------------------------------------------------------------- Add Noise Node

  //************************************************************************************************
  AddNoiseNode::AddNoiseNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    AdditiveNoiseDB(-40.0f),
    MultipleNoiseDB(-10.0f),
    AdditiveNoiseCutoffHz(2000.0f),
    MultipleNoiseCutoffHz(50.0f),
    AddCount(0),
    MultiplyCount(0),
    AddNoise(0.0f),
    MultiplyNoise(0.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new AddNoiseNode(status, name, ID, nullptr, true), status);
    else
    {
      AddPeriod = gAudioSystem->SystemSampleRate * 0.5f / AdditiveNoiseCutoffHz;
      MultiplyPeriod = gAudioSystem->SystemSampleRate * 0.5f / MultipleNoiseCutoffHz;
      AddGain = Math::Pow(10.0f, 0.05f * AdditiveNoiseDB);
      MultiplyGain = Math::Pow(10.0f, 0.05f * MultipleNoiseDB);
    }
  }

  //************************************************************************************************
  void AddNoiseNode::SetAdditiveNoiseGainDB(const float decibels)
  {
    AdditiveNoiseDB = decibels;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AddNoiseNode::SetAdditiveNoiseGainDB, 
          (AddNoiseNode*)GetSiblingNode(), decibels));
    }
    else
      AddGain = Math::Pow(10.0f, 0.05f * AdditiveNoiseDB);
  }

  //************************************************************************************************
  float AddNoiseNode::GetAdditiveNoiseGainDB()
  {
    return AdditiveNoiseDB;
  }

  //************************************************************************************************
  void AddNoiseNode::SetMultipleNoiseGainDB(const float decibels)
  {
    MultipleNoiseDB = decibels;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AddNoiseNode::SetMultipleNoiseGainDB, 
          (AddNoiseNode*)GetSiblingNode(), decibels));
    }
    else
      MultiplyGain = Math::Pow(10.0f, 0.05f * MultipleNoiseDB);
  }

  //************************************************************************************************
  float AddNoiseNode::GetMultipleNoiseGainDB()
  {
    return MultipleNoiseDB;
  }

  //************************************************************************************************
  void AddNoiseNode::SetAdditiveCutoffHz(const float cutoff)
  {
    AdditiveNoiseCutoffHz = cutoff;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AddNoiseNode::SetAdditiveCutoffHz, 
          (AddNoiseNode*)GetSiblingNode(), cutoff));
    }
    else
      AddPeriod = gAudioSystem->SystemSampleRate * 0.5f / AdditiveNoiseCutoffHz;
  }

  //************************************************************************************************
  float AddNoiseNode::GetAdditiveCutoffHz()
  {
    return AdditiveNoiseCutoffHz;
  }

  //************************************************************************************************
  void AddNoiseNode::SetMultipleCutoffHz(const float cutoff)
  {
    MultipleNoiseCutoffHz = cutoff;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AddNoiseNode::SetMultipleCutoffHz, 
          (AddNoiseNode*)GetSiblingNode(), cutoff));
    }
    else
      MultiplyPeriod = gAudioSystem->SystemSampleRate * 0.5f / MultipleNoiseCutoffHz;
  }

  //************************************************************************************************
  float AddNoiseNode::GetMultipleCutoffHz()
  {
    return MultipleNoiseCutoffHz;
  }

  //************************************************************************************************
  bool AddNoiseNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      for (unsigned j = 0; j < numberOfChannels; ++j)
      {
        (*outputBuffer)[i + j] = (InputSamples[i + j] * (1.0f - MultiplyGain
          + (MultiplyGain * MultiplyNoise))) + (AddGain * AddNoise);
      }

      ++AddCount;
      if (AddCount >= AddPeriod)
      {
        AddCount -= (int)AddPeriod;
        AddNoise = RandomValues.FloatRange(-1.0f, 1.0f);
      }

      ++MultiplyCount;
      if (MultiplyCount >= MultiplyPeriod)
      {
        MultiplyCount -= (int)MultiplyPeriod;
        MultiplyNoise = RandomValues.FloatRange(0.0f, 1.0f);
      }
    }

    AddBypass(outputBuffer);

    return true;
  }

  //-------------------------------------------------------------------------------- Modulation Node

  //************************************************************************************************
  ModulationNode::ModulationNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface * extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    Amplitude(false),
    Frequency(10.0f),
    WetValue(1.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new ModulationNode(status, name, ID, nullptr, true), status);
  }

  //************************************************************************************************
  ModulationNode::~ModulationNode()
  {
    if (Threaded)
    {
      forRange(Oscillator* osc, OscillatorsPerListener.Values())
        delete osc;
    }
  }

  //************************************************************************************************
  bool ModulationNode::GetUsingAmplitude()
  {
    return Amplitude;
  }

  //************************************************************************************************
  void ModulationNode::SetUsingAmplitude(const bool useAmplitudeMod)
  {
    Amplitude = useAmplitudeMod;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ModulationNode::SetUsingAmplitude,
            (ModulationNode*)GetSiblingNode(), useAmplitudeMod));
    }
    else
    {
      forRange(Oscillator* sineWave, OscillatorsPerListener.Values())
      {
        if (Amplitude)
          sineWave->SetPolarity(Oscillator::Unipolar);
        else
          sineWave->SetPolarity(Oscillator::Bipolar);
      }
    }
  }

  //************************************************************************************************
  float ModulationNode::GetFrequency()
  {
    return Frequency;
  }

  //************************************************************************************************
  void ModulationNode::SetFrequency(const float newFrequency)
  {
    Frequency = newFrequency;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ModulationNode::SetFrequency, 
            (ModulationNode*)GetSiblingNode(), newFrequency));
    }
    else
    {
      forRange(Oscillator* sineWave, OscillatorsPerListener.Values())
        sineWave->SetFrequency(newFrequency);
    }
  }

  //************************************************************************************************
  float ModulationNode::GetWetPercent()
  {
    return WetValue * 100.0f;
  }

  //************************************************************************************************
  void ModulationNode::SetWetPercent(const float percent)
  {
    WetValue = percent / 100.0f;

    if (WetValue < 0.0f)
      WetValue = 0.0f;
    if (WetValue > 1.0f)
      WetValue = 1.0f;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&ModulationNode::SetWetPercent, 
          (ModulationNode*)GetSiblingNode(), percent));
  }

  //************************************************************************************************
  bool ModulationNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Check if the listener is in the map
    Oscillator* sineWave = OscillatorsPerListener.FindValue(listener, nullptr);
    if (!sineWave)
    {
      sineWave = new Oscillator;
      sineWave->SetNoteOn(true);
      sineWave->SetFrequency(Frequency);
      if (Amplitude)
        sineWave->SetPolarity(Oscillator::Unipolar);
      else
        sineWave->SetPolarity(Oscillator::Bipolar);
      OscillatorsPerListener[listener] = sineWave;
    }

    // Go through all frames
    for (unsigned frame = 0; frame < bufferSize; frame += numberOfChannels)
    {
      float waveValue = sineWave->GetNextSample();

      // Multiply signal with modulator wave, taking into account gain and wet percent
      for (unsigned channel = 0; channel < numberOfChannels; ++channel)
        (*outputBuffer)[frame + channel] = (InputSamples[frame + channel] * waveValue * WetValue)
        + (InputSamples[frame + channel] * (1.0f - WetValue));
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void ModulationNode::RemoveListener(ListenerNode* listener)
  {
    if (OscillatorsPerListener.FindValue(listener, nullptr))
    {
      delete OscillatorsPerListener[listener];
      OscillatorsPerListener.Erase(listener);
    }
  }


}