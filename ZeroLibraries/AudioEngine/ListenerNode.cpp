///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //----------------------------------------------------------------------------- Listener Node Data

  class ListenerNodeData
  {
  public:
    ListenerNodeData(ListenerWorldPositionInfo& positionInfo);

    // Used for interpolating volume when changing active state. 
    InterpolatingObject VolumeInterpolator;
    // If true, currently interpolating volume. 
    bool InterpolatingVolume;
    // Current position of the listener. 
    Math::Vec3 PositionWorld;
    // Current velocity of the listener. 
    Math::Vec3 VelocityWorld;
    // Current transform matrix used for calculations. 
    Math::Mat3 WorldToLocal;
    // If true, currently interpolating to 0 to deactivate
    bool Deactivating;
  };

  //************************************************************************************************
  ListenerNodeData::ListenerNodeData(ListenerWorldPositionInfo& positionInfo) :
    InterpolatingVolume(false),
    Deactivating(false),
    PositionWorld(positionInfo.Position),
    VelocityWorld(positionInfo.Velocity)
  {
    Math::Vec3 right = positionInfo.ForwardDirection.Cross(positionInfo.UpDirection);

    positionInfo.ForwardDirection.Normalize();
    right.Normalize();
    positionInfo.UpDirection.Normalize();

    Math::Mat3 orientation;
    orientation.SetBasis(0, positionInfo.ForwardDirection);
    orientation.SetBasis(1, positionInfo.UpDirection);
    orientation.SetBasis(2, -right);

    WorldToLocal = orientation.Inverted();
  }

  //---------------------------------------------------------------------------------- Listener Node

  //************************************************************************************************
  ListenerNode::ListenerNode(Zero::Status& status, Zero::StringParam name, unsigned ID,
    ListenerWorldPositionInfo positionInfo, ExternalNodeInterface* extInt, bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    ThreadedData(nullptr),
    Active(true)
  {
    if (!Threaded)
      SetSiblingNodes(new ListenerNode(status, name, ID, positionInfo, nullptr, true), status);
    else
      ThreadedData = new ListenerNodeData(positionInfo);
  }

  //************************************************************************************************
  ListenerNode::~ListenerNode()
  {
    if (Threaded)
    {
      forRange(SoundNode& node, gAudioSystem->NodeListThreaded.All())
        node.RemoveListener(this);

      delete ThreadedData;
    }
  }

  //************************************************************************************************
  bool ListenerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // Get input
    bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, this);

    // If not active, can return 
    if (!Active)
      return false;

    // Move input to output buffer
    InputSamples.Swap(*outputBuffer);

    // If we are interpolating between active/inactive, adjust with listener volume 
    if (ThreadedData->InterpolatingVolume)
    {
      // If no input, just move interpolator forward
      if (!isThereOutput)
      {
        ThreadedData->VolumeInterpolator.JumpForward(outputBuffer->Size());
        ThreadedData->InterpolatingVolume = !ThreadedData->VolumeInterpolator.Finished();
        if (!ThreadedData->InterpolatingVolume && ThreadedData->Deactivating)
        {
          ThreadedData->Deactivating = false;
          Active = false;
        }
      }
      else
      {
        // Step through each audio frame
        for (unsigned i = 0; i < outputBuffer->Size(); i += numberOfChannels)
        {
          float volume = ThreadedData->VolumeInterpolator.NextValue();
          if (ThreadedData->VolumeInterpolator.Finished())
          {
            ThreadedData->InterpolatingVolume = false;
            if (ThreadedData->Deactivating)
            {
              ThreadedData->Deactivating = false;
              Active = false;

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
          for (unsigned j = 0; j < numberOfChannels; ++j)
            (*outputBuffer)[i + j] *= volume;
        }
      }
    }

    return isThereOutput;
  }

  //************************************************************************************************
  void ListenerNode::SetPositionData(ListenerWorldPositionInfo positionInfo)
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ListenerNode::SetPositionData,
        (ListenerNode*)GetSiblingNode(), positionInfo));
    }
    else
    {
      ThreadedData->PositionWorld = positionInfo.Position;
      ThreadedData->VelocityWorld = positionInfo.Velocity;

      Math::Vec3 right = positionInfo.ForwardDirection.Cross(positionInfo.UpDirection);

      positionInfo.ForwardDirection.Normalize();
      right.Normalize();
      positionInfo.UpDirection.Normalize();

      Math::Mat3 orientation;
      orientation.SetBasis(0, positionInfo.ForwardDirection);
      orientation.SetBasis(1, positionInfo.UpDirection);
      orientation.SetBasis(2, -right);

      ThreadedData->WorldToLocal = orientation.Inverted();
    }
  }

  //************************************************************************************************
  void ListenerNode::SetActive(const bool active)
  {
    if (!Threaded)
    {
      Active = active;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&ListenerNode::SetActive,
        (ListenerNode*)GetSiblingNode(), active));
    }
    else
    {
      // If currently active and setting to inactive
      if (Active && !active && !ThreadedData->Deactivating)
      {
        ThreadedData->Deactivating = true;
        ThreadedData->InterpolatingVolume = true;
        ThreadedData->VolumeInterpolator.SetValues(ThreadedData->VolumeInterpolator.GetCurrentValue(),
          0.0f, (unsigned)(0.02f * gAudioSystem->SystemSampleRate));
      }
      // If currently not active and setting to active
      else if ((!Active || ThreadedData->Deactivating) && active)
      {
        ThreadedData->Deactivating = false;
        Active = true;
        ThreadedData->InterpolatingVolume = true;
        ThreadedData->VolumeInterpolator.SetValues(ThreadedData->VolumeInterpolator.GetCurrentValue(),
          1.0f, (unsigned)(0.02f * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  bool ListenerNode::GetActive()
  {
    return Active;
  }

  //************************************************************************************************
  Math::Vec3 ListenerNode::GetRelativePosition(Math::Vec3Param otherPosition)
  {
    return Math::Transform(ThreadedData->WorldToLocal, (otherPosition - ThreadedData->PositionWorld));
  }

  //************************************************************************************************
  Math::Vec3 ListenerNode::GetRelativeVelocity(Math::Vec3Param otherVelocity)
  {
    return Math::Transform(ThreadedData->WorldToLocal, (otherVelocity - ThreadedData->VelocityWorld));
  }

  //************************************************************************************************
  Math::Vec3 ListenerNode::GetRelativeFacing(Math::Vec3Param facingDirection)
  {
    return Math::Transform(ThreadedData->WorldToLocal, facingDirection);
  }

}