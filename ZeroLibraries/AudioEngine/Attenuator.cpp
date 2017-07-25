///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{

  //----------------------------------------------------------------------- Attenuation Per Listener

  class AttenuationPerListener
  {
  public:
    AttenuationPerListener() : PreviousVolume(0) {}

    // Volume used last mix for this listener
    float PreviousVolume;
    // Low pass filter for this listener
    LowPassFilter LowPass;
  };

  //-------------------------------------------------------------------------------- Attenuator Node

  //************************************************************************************************
  AttenuatorNode::AttenuatorNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      Math::Vec3Param position, const AttenuationData& data, const CurveTypes curveType, 
      Zero::Array<Math::Vec3> *customCurveData, ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, true, false, isThreaded), 
    AttenStartDist(data.StartDistance), 
    UseLowPass(false), 
    LowPassDistance(data.EndDistance / 2.0f),
    AttenEndDist(data.EndDistance), 
    MinimumVolume(data.MinimumVolume), 
    Position(position),
    DistanceInterpolator(nullptr),
    LowPassInterpolator(nullptr)
  {
    if (!Threaded)
    {
      SetSiblingNodes(new AttenuatorNode(status, name, ID, position, data, curveType, 
        customCurveData, extInt, true), status);
    }
    else
    {
      // Set values on interpolator
      DistanceInterpolator = gAudioSystem->GetInterpolatorThreaded();
      DistanceInterpolator->SetValues(1.0f, MinimumVolume, AttenEndDist - AttenStartDist);

      // Set the falloff curve on the interpolator
      if (curveType == CustomCurveType)
      {
        if (customCurveData)
          DistanceInterpolator->SetCustomCurve(customCurveData);
      }
      else
        DistanceInterpolator->SetCurve(curveType);

      // Set the low pass interpolator 
      LowPassInterpolator = gAudioSystem->GetInterpolatorThreaded();
      LowPassInterpolator->SetValues(15000.0f, 1000.0f, LowPassDistance);
    }
  }

  //************************************************************************************************
  AttenuatorNode::~AttenuatorNode()
  {
    if (Threaded)
    {
      gAudioSystem->ReleaseInterpolatorThreaded(DistanceInterpolator);
      gAudioSystem->ReleaseInterpolatorThreaded(LowPassInterpolator);

      for (DataPerListenerMapType::valuerange data = DataPerListener.Values(); !data.Empty(); data.PopFront())
        delete data.Front();
    }
  }

  //************************************************************************************************
  void AttenuatorNode::SetPosition(Math::Vec3Param newPosition)
  {
    Position = newPosition;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::Position,
          (AttenuatorNode*)GetSiblingNode(), newPosition));
  }

  //************************************************************************************************
  void AttenuatorNode::SetAttenuationData(const AttenuationData data)
  {
    AttenStartDist = data.StartDistance;
    AttenEndDist = data.EndDistance;
    MinimumVolume = data.MinimumVolume;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::SetAttenuationData,
          (AttenuatorNode*)GetSiblingNode(), data));
    else if (Threaded)
      DistanceInterpolator->SetValues(1.0f, MinimumVolume, AttenEndDist - AttenStartDist);
  }

  //************************************************************************************************
  AttenuationData AttenuatorNode::GetAttenuationData()
  {
    return AttenuationData(AttenStartDist, AttenEndDist, MinimumVolume);
  }

  //************************************************************************************************
  void AttenuatorNode::SetCurveType(const CurveTypes curveType, Zero::Array<Math::Vec3>* customCurveData)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
      {
        Zero::Array<Math::Vec3>* curve(customCurveData);
        if (customCurveData)
          // Interpolator will delete curve on destruction or when replaced with another curve
          curve = new Zero::Array<Math::Vec3>(*customCurveData);
        gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::SetCurveType, 
          (AttenuatorNode*)GetSiblingNode(), curveType, curve));
      }
    }
    else
    {
      if (curveType == CustomCurveType)
      {
        if (customCurveData)
          DistanceInterpolator->SetCustomCurve(customCurveData);
      }
      else
        DistanceInterpolator->SetCurve(curveType);
    }
  }

  //************************************************************************************************
  void AttenuatorNode::SetUsingLowPass(const bool useLP)
  {
    UseLowPass = useLP;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::UseLowPass, 
          (AttenuatorNode*)GetSiblingNode(), useLP));
  }

  //************************************************************************************************
  void AttenuatorNode::SetLowPassDistance(const float distance)
  {
    LowPassDistance = distance;

    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::SetLowPassDistance,
            (AttenuatorNode*)GetSiblingNode(), distance));
    }
    else
      LowPassInterpolator->SetValues(15000.0f, LowPassInterpolator->GetEndValue(), AttenEndDist - distance);
  }

  //************************************************************************************************
  void AttenuatorNode::SetLowPassCutoffFreq(const float frequency)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&AttenuatorNode::SetLowPassCutoffFreq, 
            (AttenuatorNode*)GetSiblingNode(), frequency));
    }
    else
      LowPassInterpolator->SetValues(15000.0f, frequency, AttenEndDist - LowPassDistance);
  }

  //************************************************************************************************
  bool AttenuatorNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // If no listener then no attenuation
    if (!listener)
    {
      InputSamples.Swap(*outputBuffer);
      return true;
    }

    // Get the relative position with the listener
    Math::Vec3 relativePosition = listener->GetRelativePosition(Position);

    float distance = relativePosition.Length();

    // If we are outside the max distance and the minimum volume is zero, there is no audio
    if (distance >= AttenEndDist && MinimumVolume == 0)
      return false;

    float attenuatedVolume;
    // If the distance is further than the attenuation end distance, the volume is the end volume
    if (distance >= AttenEndDist)
      attenuatedVolume = MinimumVolume;
    // If the distance is less than the attenuation start distance, the volume is not attenuated
    else if (distance <= AttenStartDist)
      attenuatedVolume = 1.0f;
    // If the attenuation start and end are too close together than just use end volume
    else if (AttenEndDist - AttenStartDist == 0.1f)
      attenuatedVolume = MinimumVolume;
    // Otherwise, get the value using the falloff curve on the interpolator
    else
      attenuatedVolume = DistanceInterpolator->ValueAtDistance(distance - AttenStartDist);

    // Check if the listener needs to be added to the map
    if (!DataPerListener.FindValue(listener, nullptr))
      DataPerListener[listener] = new AttenuationPerListener();

    AttenuationPerListener& listenerData = *DataPerListener[listener];

    // Apply volume adjustment to each frame of samples
    InterpolatingObject volume;
    volume.SetValues(listenerData.PreviousVolume, attenuatedVolume, bufferSize / numberOfChannels);
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      float frameVolume = volume.NextValue();

      // Copy samples from input buffer and apply volume adjustment
      for (unsigned j = 0; j < numberOfChannels; ++j)
        (*outputBuffer)[i + j] = InputSamples[i + j] * frameVolume;
    }

    // Store the previous volume
    listenerData.PreviousVolume = attenuatedVolume;

    // Check if using low pass filter
    if (UseLowPass && distance > LowPassDistance)
    {
      // Find cutoff frequency for this distance (will return end value if distance is past AttenEndDist)
      float cutoffFreq = LowPassInterpolator->ValueAtDistance(distance - LowPassDistance);

      // Set the cutoff frequency on the filter
      listenerData.LowPass.SetCutoffFrequency(cutoffFreq);

      // Apply the filter to each frame of audio samples
      float* buffer = outputBuffer->Data();
      for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
        listenerData.LowPass.ProcessSample(buffer + i, buffer + i, numberOfChannels);
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  float AttenuatorNode::GetAttenuatedVolume()
  {
    if (!Threaded)
      return 0;

    float volume(1.0);
    forRange(AttenuationPerListener* data, DataPerListener.Values())
      volume *= data->PreviousVolume;

    return volume;
  }

  //************************************************************************************************
  void AttenuatorNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

    // Check if the listener is in the map
    if (listener && DataPerListener.FindValue(listener, nullptr))
    {
      // Delete the listener's data and remove it from the map
      delete DataPerListener[listener];
      DataPerListener.Erase(listener);
    }
  }

}