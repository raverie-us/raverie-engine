///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  // The minimum volume of audio applied to all channels
  static const float MinimumVolume = 0.2f;
  // Minimum position change to recalculate data
  static const float MinimumPositionChange = 0.01f;
  // The lowest value for the low pass cutoff frequency (for sounds behind listener)
  static const float LowPassCutoffBase = 5000.0f;
  // The additional value added to the low pass cutoff depending on angle
  static const float LowPassCutoffAdditional = 15000.0f;
  // The maximum change allowed in the low pass cutoff frequency
  static const float MaxLowPassDifference = 1000.0f;

  //---------------------------------------------------------------------- Emitter Data Per Listener

  // Stores data for each listener
  class EmitterDataPerListener
  {
  public:
    EmitterDataPerListener() :
      PreviousRelativePosition(Math::Vec3(FLT_MAX, FLT_MAX, FLT_MAX)),
      DirectionalVolume(1.0f),
      UseLowPass(false)
    {
      memset(PreviousGains, 0, sizeof(float) * MaxChannels);
    }

    // Low pass filter for sounds behind listener
    LowPassFilter LowPass;
    // Previous gain values
    float PreviousGains[MaxChannels];
    // The previous relative position of this listener
    Math::Vec3 PreviousRelativePosition;

    // These values are only re-calculated when the relative position changes
    float DirectionalVolume;
    bool UseLowPass;
    float GainValues[MaxChannels];
  };

  //----------------------------------------------------------------------------------- Emitter Node

  //************************************************************************************************
  EmitterNode::EmitterNode(Zero::StringParam name, const unsigned ID, Math::Vec3Param position, 
      Math::Vec3Param velocity, ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, true, false, isThreaded),
    Position(position),
    Velocity(velocity),
    FacingDirection(1.0f, 0.0f, 0.0f),
    Pausing(false),
    Paused(false),
    DirectionalAngleRadians(0),
    PanningObject(nullptr)
  {
    if (!Threaded)
      SetSiblingNodes(new EmitterNode(name, ID, position, velocity, nullptr, true));
    else
    {
      PanningObject = new VBAP();
      PanningObject->Initialize(gAudioSystem->SystemChannelsThreaded);
    }
  }

  //************************************************************************************************
  EmitterNode::~EmitterNode()
  {
    if (Threaded)
    {
      forRange(EmitterDataPerListener* data, DataPerListener.Values())
        delete data;

      delete PanningObject;
    }
  }

  //************************************************************************************************
  void EmitterNode::Pause()
  {
    if (!Threaded)
    {
      AddTaskForSibling(&EmitterNode::Pause);
    }
    else
    {
      if (!Paused && !Pausing)
      {
        Pausing = true;
        VolumeInterpolator.SetValues(1.0f, 0.0f, PropertyChangeFrames);
      }
    }
  }

  //************************************************************************************************
  void EmitterNode::Resume()
  {
    if (!Threaded)
    {
      AddTaskForSibling(&EmitterNode::Resume);
    }
    else
    {
      if (Paused || Pausing)
      {
        Paused = false;
        Pausing = false;
        VolumeInterpolator.SetValues(VolumeInterpolator.GetCurrentValue(), 1.0f, 
          PropertyChangeFrames);
      }
    }
  }

  //************************************************************************************************
  void EmitterNode::SetPosition(const Math::Vec3 newPosition, const Math::Vec3 newVelocity)
  {
    if (!Threaded)
    {
      AddTaskForSibling(&EmitterNode::SetPosition, newPosition, newVelocity);
    }
    else
    {
      Velocity = newVelocity;
      Position = newPosition;
    }
  }

  //************************************************************************************************
  void EmitterNode::SetForwardDirection(const Math::Vec3 forwardDirection)
  {
    if (!Threaded)
      AddTaskForSibling(&EmitterNode::SetForwardDirection, forwardDirection);
    else
      FacingDirection = forwardDirection;
  }

  //************************************************************************************************
  void EmitterNode::SetDirectionalAngle(const float angleInDegrees, const float reducedVolume)
  {
    if (!Threaded)
    {
      AddTaskForSibling(&EmitterNode::SetDirectionalAngle, angleInDegrees, reducedVolume);
    }
    else
    {
      // Store half angle, in radians
      DirectionalAngleRadians = (angleInDegrees * Math::cPi / 180.0f) / 2.0f;

      DirectionalInterpolator.SetValues(1.0f, reducedVolume, Math::cPi - DirectionalAngleRadians);
    }
  }

  //************************************************************************************************
  bool EmitterNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // If paused, do nothing
    if (Paused)
      return false;

    unsigned bufferSize = outputBuffer->Size();

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
    Math::Vec3 relativePosition = listener->GetRelativePosition(Position);

    // If necessary, add new listener data
    if (!DataPerListener.FindPointer(listener))
      DataPerListener[listener] = new EmitterDataPerListener();

    // Save reference for ease of use
    EmitterDataPerListener& listenerData = *DataPerListener[listener];

    // Check if the listener or emitter has moved (don't care about up/down changes)
    bool valuesChanged = false;
    if (!IsWithinLimit(relativePosition.x, listenerData.PreviousRelativePosition.x, MinimumPositionChange)
      || !IsWithinLimit(relativePosition.z, listenerData.PreviousRelativePosition.z, MinimumPositionChange))
    {
      CalculateData(&listenerData, relativePosition, listener, numberOfChannels);
      valuesChanged = true;
    }

    // Save the relative position
    listenerData.PreviousRelativePosition = relativePosition;
    // Save a pointer to the input samples
    float* inputSamples = InputSamples.Data();

    // Apply low pass filter to output (if turned on)
    if (listenerData.UseLowPass)
    {
      listenerData.LowPass.ProcessBuffer(InputSamples.Data(), outputBuffer->Data(), numberOfChannels,
        bufferSize);

      // Input samples are now in the outputBuffer
      inputSamples = outputBuffer->Data();
    }

    // Adjust each frame with gain values
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      float volume = 1.0f;
      // If interpolating volume, get new volume value
      if (!VolumeInterpolator.Finished())
      {
        volume = VolumeInterpolator.NextValue();
        if (Pausing && VolumeInterpolator.Finished())
          Paused = true;
      }

      // Adjust the volume if this is a directional emitter
      volume *= listenerData.DirectionalVolume;

      // Combine all channels into one value
      float monoValue = inputSamples[i];
      for (unsigned j = 1; j < numberOfChannels; ++j)
        monoValue += inputSamples[i + j];
      monoValue /= numberOfChannels;
      monoValue *= volume;

      // Unspatialized audio to all channels at minimum volume
      for (unsigned j = 0; j < numberOfChannels; ++j)
        (*outputBuffer)[i + j] = inputSamples[i + j] * MinimumVolume * volume;

      // Spatialized audio using gain values
      float percent = (float)i / (float)bufferSize;
      if (valuesChanged)
      {
        // If the gain values changed, interpolate from old to new value
        for (unsigned j = 0; j < numberOfChannels; ++j)
        {
          (*outputBuffer)[i + j] += monoValue * (listenerData.PreviousGains[j] + 
            ((listenerData.GainValues[j] - listenerData.PreviousGains[j]) * percent));
        }
      }
      else
      {
        for (unsigned j = 0; j < numberOfChannels; ++j)
          (*outputBuffer)[i + j] += monoValue * listenerData.PreviousGains[j];
      }
    }

    // If gain values changed, copy new ones to previous values
    if (valuesChanged)
      memcpy(listenerData.PreviousGains, listenerData.GainValues, sizeof(float) * MaxChannels);

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
  void EmitterNode::CalculateData(EmitterDataPerListener* data, const Math::Vec3& relativePosition,
    ListenerNode* listener, const unsigned numberOfChannels)
  {
    if (!Threaded)
      return;

    // Save reference for ease of use
    EmitterDataPerListener& listenerData = *data;

    // Don't need to use the height difference for positional calculations
    Math::Vec2 relativePosition2D(relativePosition.x, relativePosition.z);

    // Get the squared length
    float distanceSq = relativePosition2D.LengthSq();

    // Check if the emitter should be limited by direction 
    if (DirectionalAngleRadians > 0.0f)
    {
      // Get the emitter's facing direction relative to the listener
      Math::Vec3 relativeFacing = listener->GetRelativeFacing(FacingDirection);
      // Get the relative angle (facing should always be normalized)
      float angle = Math::ArcCos(Math::Dot(relativePosition.Normalized(), relativeFacing));

      // If the angle to the listener is greater than the emitter's angle, reduce volume
      if (angle > DirectionalAngleRadians)
        listenerData.DirectionalVolume = DirectionalInterpolator.ValueAtDistance(angle 
          - DirectionalAngleRadians);
    }
    else
      listenerData.DirectionalVolume = 1.0f;

    // Check for sounds behind listener
    if (relativePosition2D.x < 0)
    {
      // Get the angle to the listener
      float angle = Math::ArcTan2(relativePosition.z, -relativePosition.x);
      // Translate this to a percentage of a quarter circle (this value should be 1.0 when the 
      // emitter is directly behind the listener and 0.0 when off to the side)
      float percent = Math::Abs(angle) / (Math::cPi / 2.0f);

      // The low pass cutoff frequency ranges from min to max depending on the angle
      // percentage, using a squared curve
      float frequency = LowPassCutoffBase + (LowPassCutoffAdditional * percent * percent);

      // Check if the difference between this frequency and the last frequency used is large
      if (!IsWithinLimit(frequency, listenerData.LowPass.GetCutoffFrequency(), MaxLowPassDifference))
      {
        // Set frequency to be only maxDifferenceAllowed away from the last frequency used
        if (frequency > listenerData.LowPass.GetCutoffFrequency())
          frequency = listenerData.LowPass.GetCutoffFrequency() + MaxLowPassDifference;
        else
          frequency = listenerData.LowPass.GetCutoffFrequency() - MaxLowPassDifference;
      }

      // Set the cutoff frequency on the low pass filter
      listenerData.LowPass.SetCutoffFrequency(frequency);

      listenerData.UseLowPass = true;
    }
    else
    {
      // Make sure the cutoff frequency is set to the maximum value
      listenerData.LowPass.SetCutoffFrequency(LowPassCutoffBase + LowPassCutoffAdditional);

      listenerData.UseLowPass = false;
    }

    // Check if we need to re-initialize for a different number of channels
    if (numberOfChannels != PanningObject->GetNumberOfChannels())
      PanningObject->Initialize(numberOfChannels);

    PanningObject->ComputeGains(relativePosition2D, 0.0f, listenerData.GainValues);
  }

}
