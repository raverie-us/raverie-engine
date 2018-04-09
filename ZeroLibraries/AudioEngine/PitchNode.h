///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef PITCHNODE_H
#define PITCHNODE_H

namespace Audio
{
  //------------------------------------------------------------------------------------- Pitch Node

  // Adjusts the pitch of all inputs
  class PitchNode : public SimpleCollapseNode
  {
  public:
    PitchNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt, 
      const bool isThreaded = false);

    // Returns the current pitch, in semitones
    float GetPitch();
    // Sets the pitch, in semitones, over a specified time
    void SetPitch(const float pitchSemitones, const float timeToInterpolate);

  private:
    ~PitchNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    PitchChangeHandler PitchObject;

    // Current pitch change in semitones
    float PitchSemitones;
  };
}

#endif
