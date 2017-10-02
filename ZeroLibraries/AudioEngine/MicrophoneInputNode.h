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
    MicrophoneInputNode(Zero::Status& status, Zero::StringParam name, unsigned ID,
      ExternalNodeInterface* extInt, bool isThreaded = false) :
      SoundNode(status, name, ID, extInt, false, false, isThreaded)
    {
      if (!isThreaded)
        SetSiblingNodes(new MicrophoneInputNode(status, name, ID, nullptr, true), status);
    }

  private:
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void CollapseNode() override {}
  };
  
}