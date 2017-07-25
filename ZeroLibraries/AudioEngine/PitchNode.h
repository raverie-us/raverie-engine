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
  class InterpolatingObject;

  //------------------------------------------------------------------------------------- Pitch Node

  // Adjusts the pitch of all inputs
  class PitchNode : public SimpleCollapseNode
  {
  public:
    PitchNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the current pitch
    int GetPitch();
    // Sets the pitch over a specified time
    void SetPitch(const int pitchCents, const float timeToInterpolate);

  private:
    ~PitchNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    struct Data
    {
      Data();
      Data& operator=(const Data& other);

      // Frames of interpolation that have been processed
      int FramesProcessed;
      // If true, the pitch is currently being interpolated
      bool Interpolating;
      // The last frame of samples from the last mix
      float LastSamples[MaxChannels];
      // Keeps track of the source frame index
      double PitchIndex;

      double BufferFraction;
    };

    // Current pitch change in cents
    int PitchCents;
    // Current pitch modification factor
    float PitchFactor;
    // Frames to interpolate over when changing pitch
    int FramesToInterpolate;
    // Time of interpolation when changing pitch
    float TimeToInterpolate;
    // Used to interpolate pitch
    InterpolatingObject* Interpolate;

    Data CurrentData;
    Data PreviousData;
  };
}

#endif
