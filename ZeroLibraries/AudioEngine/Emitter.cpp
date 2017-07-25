///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //---------------------------------------------------------------------- Emitter Data Per Listener

  // Stores data for each listener
  class EmitterDataPerListener
  {
  public:
    EmitterDataPerListener() :
      PreviousTotalGain(0),
      PreviousGain1(0),
      PreviousGain2(0),
      PreviousRelativePosition(Math::Vec3(FLT_MAX, FLT_MAX, FLT_MAX)),
      DirectionalVolume(1.0f),
      UseLowPass(false),
      LowPassFrequency(0.0f),
      Gain1(1.0f),
      Gain2(1.0f),
      Channel1(0),
      Channel2(0)
    {}

    // Low pass filter for sounds behind listener
    LowPassFilter LowPass;

    // Previous total attenuated volume from the last mix
    float PreviousTotalGain;
    // Previous gain for speaker 1
    float PreviousGain1;
    // Previous gain for speaker 2 from the last mix
    float PreviousGain2;
    // Interpolated gain for speaker 1 
    InterpolatingObject Gain1Interpolator;
    // Interpolated gain for speaker 2 
    InterpolatingObject Gain2Interpolator;
    // The previous relative position of this listener
    Math::Vec3 PreviousRelativePosition;

    // These values are only re-calculated when the relative position changes
    float DirectionalVolume;
    bool UseLowPass;
    float LowPassFrequency;
    float Gain1;
    float Gain2;
    int Channel1;
    int Channel2;
  };

  //----------------------------------------------------------------------------------- Emitter Data

  class EmitterData
  {
  public:
    EmitterData(Math::Vec3 position, Math::Vec3 velocity) :
      Position(position),
      Velocity(velocity),
      FacingDirection(0,0,0),
      InterpolatingVolume(false),
      Pausing(false),
      Paused(false),
      MinimumVolume(0.2f),
      DirectionalAngleRadians(0)
    {}

    // Current emitter position. 
    Math::Vec3 Position;
    // Current emitter velocity. 
    Math::Vec3 Velocity;
    // Direction object is facing. 
    Math::Vec3 FacingDirection;
    // Used for interpolating between volume changes when pausing. 
    InterpolatingObject VolumeInterpolator;
    // If true, currently interpolating volume. 
    bool InterpolatingVolume;
    // If true, currently interpolating volume to 0 before pausing
    bool Pausing;
    // If true, emitter is paused
    bool Paused;
    // The angle, in radians, of half the directional cone
    float DirectionalAngleRadians;
    // Used to interpolate volume from edge of angle to directly behind emitter
    InterpolatingObject DirectionalInterpolator;
    // The minimum volume of audio applied to all channels
    float MinimumVolume;
  };

  //----------------------------------------------------------------------------------- Emitter Node

  //************************************************************************************************
  EmitterNode::EmitterNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, 
      Math::Vec3Param position, Math::Vec3Param velocity, ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, true, false, isThreaded), 
    Data(nullptr)
  {
    if (!Threaded)
      SetSiblingNodes(new EmitterNode(status, name, ID, position, velocity, nullptr, true), status);
    else
    {
      Data = new EmitterData(position, velocity);
    }
  }

  //************************************************************************************************
  EmitterNode::~EmitterNode()
  {
    if (Threaded)
    {
      forRange(EmitterDataPerListener* data, DataPerListener.Values())
        delete data;

      delete Data;
    }
  }

  //************************************************************************************************
  void EmitterNode::Pause()
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&EmitterNode::Pause, (EmitterNode*)GetSiblingNode()));
    }
    else
    {
      if (!Data->Paused && !Data->Pausing)
      {
        Data->Pausing = true;
        Data->InterpolatingVolume = true;
        Data->VolumeInterpolator.SetValues(1.0f, 0.0f, (unsigned)(0.02f * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  void EmitterNode::Resume()
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&EmitterNode::Resume, (EmitterNode*)GetSiblingNode()));
    }
    else
    {
      if (Data->Paused || Data->Pausing)
      {
        Data->Paused = false;
        Data->Pausing = false;
        Data->InterpolatingVolume = true;
        Data->VolumeInterpolator.SetValues(Data->VolumeInterpolator.GetCurrentValue(), 1.0f, 
          (unsigned)(0.02f * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  void EmitterNode::SetPosition(const Math::Vec3 newPosition, const Math::Vec3 newVelocity)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&EmitterNode::SetPosition, 
            (EmitterNode*)GetSiblingNode(), newPosition, newVelocity));
    }
    else
    {
      Data->Velocity = newVelocity;
      Data->Position = newPosition;
    }
  }

  //************************************************************************************************
  void EmitterNode::SetForwardDirection(const Math::Vec3 forwardDirection)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&EmitterNode::SetForwardDirection, 
            (EmitterNode*)GetSiblingNode(), forwardDirection));
    }
    else
      Data->FacingDirection = forwardDirection;
  }

  //************************************************************************************************
  void EmitterNode::SetDirectionalAngle(const float angleInDegrees, const float reducedVolume)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&EmitterNode::SetDirectionalAngle, 
            (EmitterNode*)GetSiblingNode(), angleInDegrees, reducedVolume));
    }
    else
    {
      // Store half angle, in radians
      Data->DirectionalAngleRadians = (angleInDegrees * Math::cPi / 180.0f) / 2.0f;

      Data->DirectionalInterpolator.SetValues(1.0f, reducedVolume, Math::cPi - Data->DirectionalAngleRadians);
    }
  }

  //************************************************************************************************
  bool EmitterNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // If paused, do nothing
    if (Data->Paused)
      return false;

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // If only one channel of output or no listener, no spatialization, so just return
    if (numberOfChannels == 1 || !listener)
    {
      // Move the input samples to the output buffer
      InputSamples.Swap(*outputBuffer);
      return true;
    }

    // Get the relative position between emitter and listener
    Math::Vec3 relativePosition = listener->GetRelativePosition(Data->Position);

    // If necessary, add new listener data
    if (!DataPerListener.FindPointer(listener))
      DataPerListener[listener] = new EmitterDataPerListener();

    // Save reference for ease of use
    EmitterDataPerListener& listenerData = *DataPerListener[listener];

    // Check if the listener or emitter has moved
    if (!IsWithinLimit(relativePosition.x, listenerData.PreviousRelativePosition.x, 0.01f) 
        || !IsWithinLimit(relativePosition.y, listenerData.PreviousRelativePosition.y, 0.01f)
        || !IsWithinLimit(relativePosition.z, listenerData.PreviousRelativePosition.z, 0.01f))
      CalculateData(&listenerData, relativePosition, listener, numberOfChannels);

    listenerData.PreviousRelativePosition = relativePosition;
    unsigned numberOfFrames = bufferSize / numberOfChannels;

    // Set the gain interpolators
    listenerData.Gain1Interpolator.SetValues(listenerData.PreviousGain1, listenerData.Gain1, numberOfFrames);
    listenerData.Gain2Interpolator.SetValues(listenerData.PreviousGain2, listenerData.Gain2, numberOfFrames);

    // Store this mix's gain values
    listenerData.PreviousGain1 = listenerData.Gain1;
    listenerData.PreviousGain2 = listenerData.Gain2;

    // Apply low pass filter to output (if turned on)
    if (listenerData.UseLowPass)
    {
      // Copy input samples to output buffer while applying filter
      for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
        listenerData.LowPass.ProcessSample(InputSamples.Data() + i, outputBuffer->Data() + i, numberOfChannels);
    }

    // Adjust with gain values
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      float volume = 1.0f;
      // If interpolating volume, get new volume value
      if (Data->InterpolatingVolume)
      {
        volume = Data->VolumeInterpolator.NextValue();
        if (Data->VolumeInterpolator.Finished())
        {
          Data->InterpolatingVolume = false;

          if (Data->Pausing)
            Data->Paused = true;
        }
      }

      // Adjust the volume if this is a directional emitter
      volume *= listenerData.DirectionalVolume;

      // If using low pass filter, value was already copied from InputSamples to outputBuffer
      float* buffer;
      if (listenerData.UseLowPass)
        buffer = outputBuffer->Data();
      // Otherwise, need to copy input samples
      else
        buffer = InputSamples.Data();

      // Combine all channels into one value
      float monoValue = buffer[i];
      for (unsigned j = 1; j < numberOfChannels; ++j)
        monoValue += buffer[i + j];
      monoValue /= numberOfChannels;
      monoValue *= volume;

      // Unspatialized audio to all channels at minimum volume
      for (unsigned j = 0; j < numberOfChannels; ++j)
        (*outputBuffer)[i + j] = buffer[i + j] * Data->MinimumVolume * volume;

      // Spatialized gain to two channels
      (*outputBuffer)[i + listenerData.Channel1] += monoValue * listenerData.Gain1Interpolator.NextValue();
      (*outputBuffer)[i + listenerData.Channel2] += monoValue * listenerData.Gain2Interpolator.NextValue();
    }

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void EmitterNode::RemoveListener(ListenerNode* listener)
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

  //************************************************************************************************
  void EmitterNode::CalculateData(EmitterDataPerListener* data, Math::Vec3& relativePosition,
    ListenerNode* listener, const unsigned numberOfChannels)
  {
    // Save reference for ease of use
    EmitterDataPerListener& listenerData = *data;

    // Get the squared length
    float distanceSq = relativePosition.LengthSq();

    // Check if the emitter should be limited by direction and is not too close
    if (Data->DirectionalAngleRadians > 0.0f && distanceSq > 0.01f)
    {
      // Get the emitter's facing direction relative to the listener
      Math::Vec3 relativeFacing = listener->GetRelativeFacing(Data->FacingDirection);
      // Get the relative angle (facing should always be normalized)
      float angle = Math::ArcCos(Math::Dot(relativePosition.Normalized(), relativeFacing));

      // If the angle to the listener is greater than the emitter's angle, reduce volume
      if (angle > Data->DirectionalAngleRadians)
        listenerData.DirectionalVolume = Data->DirectionalInterpolator.ValueAtDistance(angle 
          - Data->DirectionalAngleRadians);
    }
    else
      listenerData.DirectionalVolume = 1.0f;

    // Check for sounds behind listener
    if (relativePosition.x < 0)
    {
      // Set low pass filter
      float angle = Math::ArcTan2(relativePosition.z, -relativePosition.x);
      float percent = Math::Abs(angle) / (Math::cPi / 2.0f);

      float frequency = 5000.0f + (15000.0f * percent * percent);

      listenerData.LowPass.SetCutoffFrequency(frequency);

      // Mirror source to front for stereo
      if (numberOfChannels < 4)
        relativePosition.x *= -1;

      listenerData.UseLowPass = true;
    }
    else
      listenerData.UseLowPass = false;

    // If emitter and listener are very close or emitter is directly above or below listener, skip calculations
    if (relativePosition.LengthSq() < 0.01f || (relativePosition.x == 0.0f && relativePosition.z == 0.0f))
    {
      listenerData.Gain1 = 1.0f;
      listenerData.Gain2 = 1.0f;
      listenerData.Channel1 = 0;
      listenerData.Channel2 = 1;
    }
    else
    {
      // Get normalized vector to sound source
      Math::Vec2 source = Math::Vec2(relativePosition.x, relativePosition.z);
      source.Normalize();

      // Get gain values and which channels to apply them to
      gAudioSystem->ChannelsManager.GetClosestSpeakerValues(source, numberOfChannels, listenerData.Gain1,
        listenerData.Gain2, listenerData.Channel1, listenerData.Channel2);

      // If a gain is less than zero, full volume goes to the other speaker
      if (listenerData.Gain1 < 0)
      {
        listenerData.Gain1 = 0;
        listenerData.Gain2 = 1.0f;
      }
      else if (listenerData.Gain2 < 0)
      {
        listenerData.Gain2 = 0;
        listenerData.Gain1 = 1.0f;
      }

      // Make sure the gain is at least the minimum
      if (listenerData.Gain1 < Data->MinimumVolume)
        listenerData.Gain1 = Data->MinimumVolume;
      if (listenerData.Gain2 < Data->MinimumVolume)
        listenerData.Gain2 = Data->MinimumVolume;

      // Normalize
      float scaleFactor = 1.0f / Math::Sqrt(listenerData.Gain1 * listenerData.Gain1
        + listenerData.Gain2 * listenerData.Gain2);
      listenerData.Gain1 *= scaleFactor;
      listenerData.Gain2 *= scaleFactor;
    }
  }

}