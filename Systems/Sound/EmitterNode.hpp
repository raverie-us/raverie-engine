///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//------------------------------------------------------------------------ Emitter Data Per Listener

// Stores data for each listener
class EmitterDataPerListener
{
public:
  EmitterDataPerListener();

  // Low pass filter for sounds behind listener
  LowPassFilter LowPass;
  // Previous gain values
  float mPreviousGains[AudioConstants::cMaxChannels];
  // The previous relative position of this listener
  Math::Vec3 mPreviousRelativePosition;

  // These values are only re-calculated when the relative position changes
  float mDirectionalVolume;
  bool mUseLowPass;
  float mGainValues[AudioConstants::cMaxChannels];
};

//------------------------------------------------------------------------------------- Emitter Node

class EmitterNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  EmitterNode(StringParam name, const unsigned ID, Vec3Param position, Vec3Param velocity);

  // Pauses and resumes all output (doesn't process inputs while paused)
  void SetPausedThreaded(bool paused);
  // Sets the emitter's current position and velocity
  void SetPositionThreaded(const Vec3 newPosition, const Vec3 newVelocity);
  // Sets the emitter's current orientation
  void SetForwardDirectionThreaded(const Vec3 forwardDirection);
  // Sets the angle for a directional emitter
  void SetDirectionalAngleThreaded(const float angleInDegrees, const float reducedVolume);

private:
  // Note: all functions and data below should be accessed only on the mix thread

  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void RemoveListenerThreaded(SoundEvent* event) override;
  void CalculateData(EmitterDataPerListener* data, const Vec3& relativePosition,
    ListenerNode* listener, const unsigned numberOfChannels);

  // Current emitter position
  Vec3 mPosition;
  // Current emitter velocity
  Vec3 mVelocity;
  // Direction object is facing
  Vec3 mFacingDirection;
  // Used for interpolating between volume changes when pausing
  InterpolatingObject VolumeInterpolator;
  // If true, currently interpolating volume to 0 before pausing
  bool mPausing;
  // If true, emitter is paused
  bool mPaused;
  // The angle, in radians, of half the directional cone
  float mDirectionalAngleRadians;
  // Used to interpolate volume from edge of angle to directly behind emitter
  InterpolatingObject DirectionalInterpolator;
  // Object used to calculate channel gains for panning
  VBAP PanningObject;

  // Stored data for each listener
  Zero::HashMap<ListenerNode*, EmitterDataPerListener*> DataPerListener;

  // The minimum volume of audio applied to all channels
  const float cMinimumVolume = 0.2f;
  // Minimum position change to recalculate data
  const float cMinimumPositionChange = 0.01f;
  // The lowest value for the low pass cutoff frequency (for sounds behind listener)
  const float cLowPassCutoffBase = 5000.0f;
  // The additional value added to the low pass cutoff depending on angle
  const float cLowPassCutoffAdditional = 15000.0f;
  // The maximum change allowed in the low pass cutoff frequency
  const float cMaxLowPassDifference = 1000.0f;
};

} // namespace Zero
