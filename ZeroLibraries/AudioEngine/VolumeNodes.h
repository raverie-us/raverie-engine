///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef VOLUMENODES_H
#define VOLUMENODES_H

namespace Audio
{
  class InterpolatingObject;

  //------------------------------------------------------------------------------------ Volume Node

  // Adjusts the volume of all inputs
  class VolumeNode : public SimpleCollapseNode
  {
  public:
    VolumeNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns the current volume
    float GetVolume();
    // Sets the volume over a specified time
    void SetVolume(const float volume, float timeToInterpolate);

  private:
    ~VolumeNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    struct Data
    {
      Data() :
        Interpolating(false),
        Index(0)
      {}

      // If true, currently interpolating volume
      bool Interpolating;
      // Current interpolation index
      unsigned Index;
    };

    // The volume of the modification
    float Volume;
    // Used to interpolate between volumes
    InterpolatingObject* Interpolate;

    Data CurrentData;
    Data PreviousData;
  };

  //----------------------------------------------------------------------------------- Panning Node

  // Pans audio by controlling the volume of left and right channels
  // Always processes audio as stereo, no matter what the current mix is
  class PanningNode : public SimpleCollapseNode
  {
  public:
    PanningNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns whether the node combines all audio into one channel before panning
    bool GetSumToMono();
    // Sets whether the node combines audio to one channel before panning
    void SetSumToMono(const bool isMono);
    // Returns the current volume of the left channel
    float GetLeftVolume();
    // Sets the volume of the left channel
    void SetLeftVolume(const float volume, float time);
    // Returns the current volume of the right channel
    float GetRightVolume();
    // Sets the volume of the right channel
    void SetRightVolume(const float volume, float time);

  private:
    ~PanningNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    struct Data
    {
      Data() :
        Interpolating(false),
        LeftIndex(0),
        RightIndex(0)
      {}

      // If true, currently interpolating volume
      bool Interpolating;
      // Current interpolation index
      unsigned LeftIndex;
      unsigned RightIndex;
    };

    // If true, the node combines all audio into one channel before panning
    bool SumToMono;
    // Current volume of the left channel
    float LeftVolume;
    // Current volume of the right channel
    float RightVolume;
    // Interpolates the left channel volume from one value to another
    InterpolatingObject* LeftInterpolator;
    // Interpolates the right channel volume from one value to another
    InterpolatingObject* RightInterpolator;
    // If true, the volume is currently modified and the node will perform its calculations
    bool Active;

    Data CurrentData;
    Data PreviousData;
  };

}

#endif
