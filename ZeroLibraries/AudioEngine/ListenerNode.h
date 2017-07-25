///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef LISTENERNODE_H
#define LISTENERNODE_H

namespace Audio
{
  //------------------------------------------------------------------- World Listener Position Info

  // Stores position and velocity information for ListenerNode
  struct ListenerWorldPositionInfo
  {
    ListenerWorldPositionInfo() :
      Position(Math::Vec3(0, 0, 0)),
      Velocity(Math::Vec3(0, 0, 0)),
      ForwardDirection(Math::Vec3(0, 0, 0)),
      UpDirection(Math::Vec3(0, 0, 0))
    {}
    ListenerWorldPositionInfo(Math::Vec3 position, Math::Vec3 velocity, Math::Vec3 forward, Math::Vec3 up) :
      Position(position),
      Velocity(velocity),
      ForwardDirection(forward),
      UpDirection(up)
    {}

    Math::Vec3 Position;
    Math::Vec3 Velocity;
    Math::Vec3 ForwardDirection;
    Math::Vec3 UpDirection;
  };

  //---------------------------------------------------------------------------------- Listener Node

  class ListenerNodeData;

  class ListenerNode : public SimpleCollapseNode
  {
  public:
    ListenerNode(Zero::Status& status, Zero::StringParam name, unsigned ID,
      ListenerWorldPositionInfo positionInfo, ExternalNodeInterface* extInt, bool isThreaded = false);

    // Updates the position and velocity of the listener
    void SetPositionData(ListenerWorldPositionInfo positionInfo);
    // Sets whether this listener hears output or not (still processes sounds when inactive)
    void SetActive(const bool active);
    // Returns true if currently active
    bool GetActive();

  private:
    ~ListenerNode();

    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    // Gets the relative position of this listener
    Math::Vec3 GetRelativePosition(Math::Vec3Param otherPosition);
    // Gets the relative velocity of this listener
    Math::Vec3 GetRelativeVelocity(Math::Vec3Param otherVelocity);
    // Gets the relative facing direction of this listener
    Math::Vec3 GetRelativeFacing(Math::Vec3Param facingDirection);

    // Data used by the threaded node
    ListenerNodeData* ThreadedData;
    // If false, listener will not pass on output
    bool Active;

    friend class EmitterNode;
    friend class AttenuatorNode;
  };

}

#endif
