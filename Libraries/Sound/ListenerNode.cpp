///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//---------------------------------------------------------------------------------- Listener Node

//**************************************************************************************************
ZilchDefineType(ListenerNode, builder, type)
{

}

//**************************************************************************************************
ListenerNode::ListenerNode(Zero::StringParam name, unsigned ID, ListenerWorldPositionInfo positionInfo) :
  SoundNode(name, ID, false, false),
  mActive(true),
  mAttenuationScale(1.0f),
  mInterpolatingVolumeThreaded(false),
  mPositionWorldThreaded(Vec3::cZero),
  mVelocityWorldThreaded(Vec3::cZero),
  mDeactivatingThreaded(false)
{
  mWorldToLocalThreaded.ZeroOut();

  SetPositionDataThreaded(positionInfo);
}

//**************************************************************************************************
ListenerNode::~ListenerNode()
{
  Z::gSound->Mixer.SendListenerRemovedEvent(this);
}

//**************************************************************************************************
void ListenerNode::SetPositionData(ListenerWorldPositionInfo positionInfo)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&ListenerNode::SetPositionDataThreaded, this, positionInfo), this);
}

//**************************************************************************************************
void ListenerNode::SetActive(const bool active)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&ListenerNode::SetActiveThreaded, this, active), this);
}

//**************************************************************************************************
bool ListenerNode::GetActive()
{
  return mActive.Get() == cTrue;
}

//**************************************************************************************************
float ListenerNode::GetAttenuationScale()
{
  return mAttenuationScale.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void ListenerNode::SetAttenuationScale(float scale)
{
  mAttenuationScale.Set(scale, AudioThreads::MainThread);
}

//**************************************************************************************************
Math::Vec3 ListenerNode::GetRelativePositionThreaded(Math::Vec3Param otherPosition)
{
  return Math::Transform(mWorldToLocalThreaded, (otherPosition - mPositionWorldThreaded));
}

//**************************************************************************************************
Math::Vec3 ListenerNode::GetRelativeVelocityThreaded(Math::Vec3Param otherVelocity)
{
  return Math::Transform(mWorldToLocalThreaded, (otherVelocity - mVelocityWorldThreaded));
}

//**************************************************************************************************
Math::Vec3 ListenerNode::GetRelativeFacingThreaded(Math::Vec3Param facingDirection)
{
  return Math::Transform(mWorldToLocalThreaded, facingDirection);
}

//**************************************************************************************************
bool ListenerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // Get input
  bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, this);

  // If not active, can return 
  if (mActive.Get() != cTrue)
    return false;

  // Move input to output buffer
  mInputSamplesThreaded.Swap(*outputBuffer);

  // If we are interpolating between active/inactive, adjust with listener volume 
  if (mInterpolatingVolumeThreaded)
  {
    // If no input, just move interpolator forward
    if (!isThereOutput)
    {
      VolumeInterpolatorThreaded.JumpForward(outputBuffer->Size());
      mInterpolatingVolumeThreaded = !VolumeInterpolatorThreaded.Finished();
      if (!mInterpolatingVolumeThreaded && mDeactivatingThreaded)
      {
        mDeactivatingThreaded = false;
        mActive.Set(cFalse);
      }
    }
    else
    {
      // Step through each audio frame
      BufferRange outputRange = outputBuffer->All();
      for (unsigned i = 0; i < outputBuffer->Size(); i += numberOfChannels)
      {
        float volume = VolumeInterpolatorThreaded.NextValue();
        if (VolumeInterpolatorThreaded.Finished())
        {
          mInterpolatingVolumeThreaded = false;
          if (mDeactivatingThreaded)
          {
            mDeactivatingThreaded = false;
            mActive.Set(cFalse);

            // Set rest of buffer to 0
            memset(outputBuffer->Data() + i, 0, sizeof(float) * outputBuffer->Size() - i);

            // Don't need to do more
            break;
          }
          else
            // Don't need to change the volume on any more samples
            break;
        }

        // Modify volume on all channels
        for (unsigned j = 0; j < numberOfChannels; ++j, outputRange.PopFront())
          outputRange.Front() *= volume;
      }
    }
  }

  return isThereOutput;
}

//**************************************************************************************************
void ListenerNode::SetPositionDataThreaded(ListenerWorldPositionInfo positionInfo)
{
  mPositionWorldThreaded = positionInfo.mPosition;
  mVelocityWorldThreaded = positionInfo.mVelocity;
  
  Vec3 bx = positionInfo.mWorldMatrix.BasisX();
  Vec3 by = positionInfo.mWorldMatrix.BasisY();
  Vec3 bz = positionInfo.mWorldMatrix.BasisZ();

  mWorldToLocalThreaded = Math::Mat3(bz.x, bz.y, bz.z,
                                     by.x, by.y, by.z,
                                     -bx.x, -bx.y, -bx.z);
}

//**************************************************************************************************
void ListenerNode::SetActiveThreaded(bool active)
{
  // If currently active and setting to inactive
  if (!active && mActive.Get() != 0 && !mDeactivatingThreaded)
  {
    mDeactivatingThreaded = true;
    mInterpolatingVolumeThreaded = true;
    VolumeInterpolatorThreaded.SetValues(0.0f, cPropertyChangeFrames);
  }
  // If currently not active and setting to active
  else if (active && (mActive.Get() == 0 || mDeactivatingThreaded))
  {
    mDeactivatingThreaded = false;
    mActive.Set(1);
    mInterpolatingVolumeThreaded = true;
    VolumeInterpolatorThreaded.SetValues(1.0f, cPropertyChangeFrames);
  }
}

} // namespace Zero
