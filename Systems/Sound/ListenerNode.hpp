///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//--------------------------------------------------------------------- World Listener Position Info

// Stores position and velocity information for ListenerNode
class ListenerWorldPositionInfo
{
public:
  ListenerWorldPositionInfo() :
    mPosition(Vec3::cZero),
    mVelocity(Vec3::cZero)
  {}
  ListenerWorldPositionInfo(const Vec3& position, const Vec3& velocity, const Mat3& worldMatrix) :
    mPosition(position),
    mVelocity(velocity),
    mWorldMatrix(worldMatrix)
  {}

  Vec3 mPosition;
  Vec3 mVelocity;
  Mat3 mWorldMatrix;
};

//------------------------------------------------------------------------------------ Listener Node

class ListenerNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ListenerNode(Zero::StringParam name, unsigned ID, ListenerWorldPositionInfo positionInfo);
  ~ListenerNode();

  // Updates the position and velocity of the listener
  void SetPositionData(ListenerWorldPositionInfo positionInfo);
  // Sets whether this listener hears output or not (still processes sounds when inactive)
  void SetActive(const bool active);
  // Returns true if currently active
  bool GetActive();
  // Returns the current scale modifier applied to all attenuation heard by this listener
  float GetAttenuationScale();
  // Sets the scale modifier applied to all attenuation heard by this listener
  void SetAttenuationScale(float scale);

  // Gets the relative position of this listener
  Vec3 GetRelativePositionThreaded(Vec3Param otherPosition);
  // Gets the relative velocity of this listener
  Vec3 GetRelativeVelocityThreaded(Vec3Param otherVelocity);
  // Gets the relative facing direction of this listener
  Vec3 GetRelativeFacingThreaded(Vec3Param facingDirection);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  void SetPositionDataThreaded(ListenerWorldPositionInfo positionInfo);
  void SetActiveThreaded(bool active);

  // If false, listener will not pass on output
  ThreadedInt mActive;
  // The scale applied to attenuation heard by this listener
  Threaded<float> mAttenuationScale;
  // Used for interpolating volume when changing active state. 
  InterpolatingObject VolumeInterpolatorThreaded;
  // If true, currently interpolating volume. 
  bool mInterpolatingVolumeThreaded;
  // Current position of the listener. 
  Vec3 mPositionWorldThreaded;
  // Current velocity of the listener. 
  Vec3 mVelocityWorldThreaded;
  // Current transform matrix used for calculations. 
  Mat3 mWorldToLocalThreaded;
  // If true, currently interpolating to 0 to deactivate
  bool mDeactivatingThreaded;
};

} // namespace Zero
