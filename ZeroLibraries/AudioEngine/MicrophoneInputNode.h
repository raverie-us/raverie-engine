///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Audio
{
  //-------------------------------------------------------------------------- Microphone Input Node

  class MicrophoneInputNode : public SoundNode
  {
  public:
    MicrophoneInputNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
      bool isThreaded = false);

    // Returns the current volume
    float GetVolume();
    // Sets the volume modifier
    void SetVolume(float newVolume);
    // Returns whether the node is active
    bool GetActive();
    // Sets whether the node is active and outputting microphone data
    void SetActive(bool active);

  private:
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void CollapseNode() override {}

    bool Active;
    float Volume;
    InterpolatingObject VolumeInterpolator;
    bool Stopping;
    float CurrentVolume;
  };
  
}
