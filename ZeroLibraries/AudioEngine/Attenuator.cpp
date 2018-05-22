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
  AttenuatorNode::AttenuatorNode(Zero::StringParam name, const unsigned ID, Math::Vec3Param position, 
      const AttenuationData& data, const CurveTypes::Enum curveType, Zero::Array<Math::Vec3> *customCurveData, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, true, false, isThreaded), 
    AttenStartDist(data.StartDistance), 
    UseLowPass(false), 
    LowPassDistance(data.EndDistance / 2.0f),
    AttenEndDist(data.EndDistance), 
    MinimumVolume(data.MinimumVolume), 
    Position(position),
    LowPassCutoffStartValue(15000.0f)
  {
    if (!Threaded)
    {
      SetSiblingNodes(new AttenuatorNode(name, ID, position, data, curveType, customCurveData, extInt, true));
    }
    else
    {
      // Set values on interpolator
      DistanceInterpolator.SetValues(1.0f, MinimumVolume, AttenEndDist - AttenStartDist);

      // Set the falloff curve on the interpolator
      if (curveType == CurveTypes::Custom)
      {
        if (customCurveData)
          DistanceInterpolator.SetCustomCurve(customCurveData);
      }
      else
        DistanceInterpolator.SetCurve(curveType);

      // Set the low pass interpolator 
      LowPassInterpolator.SetValues(LowPassCutoffStartValue, 1000.0f, LowPassDistance);
    }
  }

  //************************************************************************************************
  AttenuatorNode::~AttenuatorNode()
  {
    if (Threaded)
    {
      for (DataPerListenerMapType::valuerange data = DataPerListener.Values(); !data.Empty(); data.PopFront())
        delete data.Front();
    }
  }

  //************************************************************************************************
  void AttenuatorNode::SetPosition(Math::Vec3Param newPosition)
  {
    Position = newPosition;

    if (!Threaded)
      AddTaskForSibling(&AttenuatorNode::Position, newPosition);
  }

  //************************************************************************************************
  void AttenuatorNode::SetAttenuationData(const AttenuationData data)
  {
    AttenStartDist = data.StartDistance;
    AttenEndDist = data.EndDistance;
    MinimumVolume = data.MinimumVolume;

    if (!Threaded)
      AddTaskForSibling(&AttenuatorNode::SetAttenuationData, data);
    else 
      DistanceInterpolator.SetValues(1.0f, MinimumVolume, AttenEndDist - AttenStartDist);
  }

  //************************************************************************************************
  AttenuationData AttenuatorNode::GetAttenuationData()
  {
    return AttenuationData(AttenStartDist, AttenEndDist, MinimumVolume);
  }

  //************************************************************************************************
  void AttenuatorNode::SetCurveType(const CurveTypes::Enum curveType, Zero::Array<Math::Vec3>* customCurveData)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
      {
        Zero::Array<Math::Vec3>* curve(customCurveData);
        if (customCurveData)
          // Interpolator will delete curve on destruction or when replaced with another curve
          curve = new Zero::Array<Math::Vec3>(*customCurveData);
        AddTaskForSibling(&AttenuatorNode::SetCurveType, curveType, curve);
      }
    }
    else
    {
      if (curveType == CurveTypes::Custom)
      {
        if (customCurveData)
          DistanceInterpolator.SetCustomCurve(customCurveData);
      }
      else
        DistanceInterpolator.SetCurve(curveType);
    }
  }

  //************************************************************************************************
  void AttenuatorNode::SetUsingLowPass(const bool useLP)
  {
    UseLowPass = useLP;

    if (!Threaded)
      AddTaskForSibling(&AttenuatorNode::UseLowPass, useLP);
  }

  //************************************************************************************************
  void AttenuatorNode::SetLowPassDistance(const float distance)
  {
    LowPassDistance = distance;

    if (!Threaded)
      AddTaskForSibling(&AttenuatorNode::SetLowPassDistance, distance);
    else
      LowPassInterpolator.SetValues(LowPassCutoffStartValue, LowPassInterpolator.GetEndValue(), 
        AttenEndDist - distance);
  }

  //************************************************************************************************
  void AttenuatorNode::SetLowPassCutoffFreq(const float frequency)
  {
    if (!Threaded)
      AddTaskForSibling(&AttenuatorNode::SetLowPassCutoffFreq, frequency);
    else
      LowPassInterpolator.SetValues(LowPassCutoffStartValue, frequency, AttenEndDist - LowPassDistance);
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
    // Save the distance value
    float distance = relativePosition.Length();
    
    // Account for the listener's attenuation scale
    if (listener->GetAttenuationScale() <= 0.0f)
      distance = AttenEndDist;
    else
      distance /= listener->GetAttenuationScale();

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
    else if (AttenEndDist - AttenStartDist <= 0.1f)
      attenuatedVolume = MinimumVolume;
    // Otherwise, get the value using the falloff curve on the interpolator
    else
      attenuatedVolume = DistanceInterpolator.ValueAtDistance(distance - AttenStartDist);

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
      float cutoffFreq = LowPassInterpolator.ValueAtDistance(distance - LowPassDistance);

      // Set the cutoff frequency on the filter
      listenerData.LowPass.SetCutoffFrequency(cutoffFreq);

      // Apply the filter to each frame of audio samples
      listenerData.LowPass.ProcessBuffer(outputBuffer->Data(), outputBuffer->Data(), numberOfChannels,
        outputBuffer->Size());
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  float AttenuatorNode::GetVolumeChangeFromOutputs()
  {
    if (!Threaded)
      return 0;

    float volume = 0.0f;

    // Get all volumes from outputs
    forRange(SoundNode* node, GetOutputs()->All())
      volume += node->GetVolumeChangeFromOutputs();

    // If there are multiple listeners, the sounds they hear are added together
    float attenuatorVolume = 0.0f;
    forRange(AttenuationPerListener* data, DataPerListener.Values())
      attenuatorVolume += data->PreviousVolume;

    // Return the output volume modified by this node's volume
    return volume * attenuatorVolume;
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