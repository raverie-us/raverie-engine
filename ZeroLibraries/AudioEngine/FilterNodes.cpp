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
  LowPassNode::LowPassNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    CutoffFrequency(20001.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new LowPassNode(name, ID, nullptr, true));
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
    filter->ProcessBuffer(InputSamples.Data(), outputBuffer->Data(), numberOfChannels, bufferSize);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void LowPassNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

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
      AddTaskForSibling(&LowPassNode::SetCutoffFrequency, freq);
    }
    else
    {
      forRange(LowPassFilter* filter, FiltersPerListener.Values())
        filter->SetCutoffFrequency(freq);
    }
  }

  //---------------------------------------------------------------------------------- HighPass Node

  //************************************************************************************************
  HighPassNode::HighPassNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    CutoffFrequency(10.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new HighPassNode(name, ID, nullptr, true));
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
      AddTaskForSibling(&HighPassNode::SetCutoffFrequency, freq);
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
      filter->ProcessFrame(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void HighPassNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

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
  BandPassNode::BandPassNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    CentralFrequency(0.0f), 
    Quality(0.669f)
  {
    if (!Threaded)
      SetSiblingNodes(new BandPassNode(name, ID, nullptr, true));
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
      AddTaskForSibling(&BandPassNode::SetCentralFrequency, frequency);
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
      AddTaskForSibling(&BandPassNode::SetQuality, Q);
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
      filter->ProcessFrame(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void BandPassNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

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
  DelayNode::DelayNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    DelayMSec(100.0f), 
    FeedbackValue(0),
    WetValue(0.5f),
    HasHadInput(false)
  {
    if (!Threaded)
      SetSiblingNodes(new DelayNode(name, ID, extInt, true));
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
      AddTaskForSibling(&DelayNode::SetDelayMSec, delay);
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetDelayMSec(delay);
    }
  }

  //************************************************************************************************
  float DelayNode::GetFeedback()
  {
    return FeedbackValue;
  }

  //************************************************************************************************
  float DelayNode::GetDelayMSec()
  {
    return DelayMSec;
  }

  //************************************************************************************************
  void DelayNode::SetFeedback(const float feedbackValue)
  {
    FeedbackValue = feedbackValue;

    if (!Threaded)
    {
      AddTaskForSibling(&DelayNode::SetFeedback, feedbackValue);
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetFeedback(feedbackValue);
    }
  }

  //************************************************************************************************
  float DelayNode::GetWetLevel()
  {
    return WetValue;
  }

  //************************************************************************************************
  void DelayNode::SetWetLevel(const float wetLevelValue)
  {
    WetValue = wetLevelValue;

    if (!Threaded)
    {
      AddTaskForSibling(&DelayNode::SetWetLevel, wetLevelValue);
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->SetWetLevel(wetLevelValue);
    }
  }

  //************************************************************************************************
  void DelayNode::InterpolateWetLevel(const float newValue, const float time)
  {
    WetValue = newValue;

    if (!Threaded)
    {
      AddTaskForSibling(&DelayNode::InterpolateWetLevel, newValue, time);
    }
    else
    {
      forRange(DelayLine* filter, FiltersPerListener.Values())
        filter->InterpolateWetLevel(newValue, time);
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
      filter->SetFeedback(FeedbackValue);
      filter->SetWetLevel(WetValue);
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
    if (!Threaded)
      return;

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
    Delay->SetFeedback(feedback);
    Delay->SetWetLevel(0.5f);
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
  FlangerNode::FlangerNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface *extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    MaxDelay(5.0f),
    ModFrequency(0.18f), 
    Feedback(0), 
    OscillatorType(OscillatorTypes::Sine)
  {
    if (!Threaded)
      SetSiblingNodes(new FlangerNode(name, ID, extInt, true));
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

    if (!Threaded)
      AddTaskForSibling(&FlangerNode::SetMaxDelayMSec, delay);
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
      AddTaskForSibling(&FlangerNode::SetModFrequency, frequency);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetFrequency(frequency);
    }
  }

  //************************************************************************************************
  float FlangerNode::GetFeedback()
  {
    return Feedback;
  }

  //************************************************************************************************
  void FlangerNode::SetFeedback(const float feedbackValue)
  {
    Feedback = feedbackValue;

    if (!Threaded)
    {
      AddTaskForSibling(&FlangerNode::SetFeedback, feedbackValue);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->Delay->SetFeedback(feedbackValue);
    }
  }

  //************************************************************************************************
  int FlangerNode::GetOscillatorType()
  {
    return OscillatorType;
  }

  //************************************************************************************************
  void FlangerNode::SetOscillatorType(const OscillatorTypes::Enum type)
  {
    OscillatorType = type;

    if (!Threaded)
    {
      AddTaskForSibling(&FlangerNode::SetOscillatorType, type);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetType(type);
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
      filter = new Data(ModFrequency, Feedback);
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
    if (!Threaded)
      return;

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
    Delay->SetFeedback(feedback);
    Delay->SetWetLevel(0.5f);
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
  ChorusNode::ChorusNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface *extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    MinDelay(5.0f),
    MaxDelay(20.0f), 
    ModFrequency(0.1f), 
    Feedback(0), 
    OscillatorType(OscillatorTypes::Sine),
    ChorusOffset(40.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new ChorusNode(name, ID, extInt, true));
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

    if (!Threaded)
      AddTaskForSibling(&ChorusNode::SetMaxDelayMSec, delay);
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

    if (!Threaded)
      AddTaskForSibling(&ChorusNode::SetMaxDelayMSec, delay);
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
      AddTaskForSibling(&ChorusNode::SetModFrequency, frequency);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetFrequency(frequency);
    }
  }

  //************************************************************************************************
  float ChorusNode::GetFeedback()
  {
    return Feedback;
  }

  //************************************************************************************************
  void ChorusNode::SetFeedback(const float feedbackValue)
  {
    Feedback = feedbackValue;

    if (!Threaded)
    {
      AddTaskForSibling(&ChorusNode::SetFeedback, feedbackValue);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->Delay->SetFeedback(feedbackValue);
    }
  }

  //************************************************************************************************
  OscillatorTypes::Enum ChorusNode::GetOscillatorType()
  {
    return OscillatorType;
  }

  //************************************************************************************************
  void ChorusNode::SetOscillatorType(const OscillatorTypes::Enum type)
  {
    OscillatorType = type;

    if (!Threaded)
    {
      AddTaskForSibling(&ChorusNode::SetOscillatorType, type);
    }
    else
    {
      forRange(Data* data, FiltersPerListener.Values())
        data->LFO->SetType(type);
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

    if (!Threaded)
      AddTaskForSibling(&ChorusNode::ChorusOffset, offset);
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
      filter = new Data(ModFrequency, MinDelay, Feedback);
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
    if (!Threaded)
      return;

    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

  //--------------------------------------------------------------------------------- Add Noise Node

  //************************************************************************************************
  AddNoiseNode::AddNoiseNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
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
      SetSiblingNodes(new AddNoiseNode(name, ID, nullptr, true));
    else
    {
      AddPeriod = cSystemSampleRate * 0.5f / AdditiveNoiseCutoffHz;
      MultiplyPeriod = cSystemSampleRate * 0.5f / MultipleNoiseCutoffHz;
      AddGain = Math::Pow(10.0f, 0.05f * AdditiveNoiseDB);
      MultiplyGain = Math::Pow(10.0f, 0.05f * MultipleNoiseDB);
    }
  }

  //************************************************************************************************
  void AddNoiseNode::SetAdditiveNoiseGainDB(const float decibels)
  {
    AdditiveNoiseDB = decibels;

    if (!Threaded)
      AddTaskForSibling(&AddNoiseNode::SetAdditiveNoiseGainDB, decibels);
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
      AddTaskForSibling(&AddNoiseNode::SetMultipleNoiseGainDB, decibels);
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
      AddTaskForSibling(&AddNoiseNode::SetAdditiveCutoffHz, cutoff);
    else
      AddPeriod = cSystemSampleRate * 0.5f / AdditiveNoiseCutoffHz;
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
      AddTaskForSibling(&AddNoiseNode::SetMultipleCutoffHz, cutoff);
    else
      MultiplyPeriod = cSystemSampleRate * 0.5f / MultipleNoiseCutoffHz;
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
  ModulationNode::ModulationNode(Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface * extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    Amplitude(false),
    Frequency(10.0f),
    WetLevelValue(1.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new ModulationNode(name, ID, nullptr, true));
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
      AddTaskForSibling(&ModulationNode::SetUsingAmplitude, useAmplitudeMod);
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
      AddTaskForSibling(&ModulationNode::SetFrequency, newFrequency);
    }
    else
    {
      forRange(Oscillator* sineWave, OscillatorsPerListener.Values())
        sineWave->SetFrequency(newFrequency);
    }
  }

  //************************************************************************************************
  float ModulationNode::GetWetLevel()
  {
    return WetLevelValue;
  }

  //************************************************************************************************
  void ModulationNode::SetWetLevel(const float wetLevel)
  {
    WetLevelValue = wetLevel;

    if (WetLevelValue < 0.0f)
      WetLevelValue = 0.0f;
    if (WetLevelValue > 1.0f)
      WetLevelValue = 1.0f;

    if (!Threaded)
      AddTaskForSibling(&ModulationNode::SetWetLevel, wetLevel);
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
        (*outputBuffer)[frame + channel] = (InputSamples[frame + channel] * waveValue * WetLevelValue)
        + (InputSamples[frame + channel] * (1.0f - WetLevelValue));
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void ModulationNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

    if (OscillatorsPerListener.FindValue(listener, nullptr))
    {
      delete OscillatorsPerListener[listener];
      OscillatorsPerListener.Erase(listener);
    }
  }


}