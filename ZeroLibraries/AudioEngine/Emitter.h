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
    EmitterNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, Math::Vec3Param position, 
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
    EmitterData* Data;
    // Stored data for each listener
    Zero::HashMap<ListenerNode*, EmitterDataPerListener*> DataPerListener;

  };

}

#endif