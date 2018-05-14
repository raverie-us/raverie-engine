///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef EMITTER_H
#define EMITTER_H

namespace Audio
{
  //----------------------------------------------------------------------------------- Emitter Node

  class EmitterData;
  class ListenerNode;
  class EmitterDataPerListener;

  class EmitterNode : public SimpleCollapseNode
  {
  public:
    EmitterNode(Zero::StringParam name, const unsigned ID, Math::Vec3Param position, 
      Math::Vec3Param velocity, ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Pauses all output (doesn't process inputs while paused)
    void Pause();
    // Resumes output
    void Resume();
    // Sets the emitter's current position and velocity
    void SetPosition(const Math::Vec3 newPosition, const Math::Vec3 newVelocity);
    // Sets the emitter's current orientation
    void SetForwardDirection(const Math::Vec3 forwardDirection);
    // Sets the angle for a directional emitter
    void SetDirectionalAngle(const float angleInDegrees, const float reducedVolume);

  private:
    ~EmitterNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void RemoveListener(ListenerNode* listener) override;
    void CalculateData(EmitterDataPerListener* data, Math::Vec3& relativePosition,
      ListenerNode* listener, const unsigned numberOfChannels);

    // Data used by the threaded node

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

    // Stored data for each listener
    Zero::HashMap<ListenerNode*, EmitterDataPerListener*> DataPerListener;

  };

}

#endif