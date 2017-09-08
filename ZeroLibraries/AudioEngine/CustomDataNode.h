///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Audio
{
  //--------------------------------------------------------------------- Custom Data Sample Request

  struct CustomDataSampleRequest
  {
    CustomDataSampleRequest(unsigned samples) : SamplesNeeded(samples) {}

    unsigned SamplesNeeded;
  };

  //------------------------------------------------------------------------------- Custom Data Node

  class CustomDataNode : public SoundNode
  {
  public:
    CustomDataNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, 
      ExternalNodeInterface* extInt, const bool isThreaded = false);
    
    // Hands off a buffer of samples to be played by this node. The node will handle deleting the data.
    void AddSamples(float* samples, const unsigned sampleCount);
    // Returns the minimum number of samples that must be provided when requested
    unsigned GetMinimumBufferSize();
    // Returns the sample rate of the audio system
    unsigned GetSystemSampleRate();
    // Returns the current number of channels used by this node for output
    unsigned GetNumberOfChannels();
    // Sets the number of channels used by this node for output
    void SetNumberOfChannels(const unsigned channels);

  private:
    ~CustomDataNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void CollapseNode() override {}

    struct SampleBuffer
    {
      SampleBuffer(float* buffer, unsigned size) : Buffer(buffer), BufferSize(size), FrameIndex(0) {}
      ~SampleBuffer() { delete[] Buffer; }

      float* Buffer;
      unsigned BufferSize;
      unsigned FrameIndex;

      Zero::Link<SampleBuffer> link;
    };

    void AddBufferToList(SampleBuffer* newBuffer);
    void SetMinimumBufferSize();

    Zero::InList<SampleBuffer> BufferList;

    bool WaitingForSamples;
    unsigned Channels;
    unsigned SampleRate;
    unsigned TotalSamplesInBuffers;
    unsigned SamplesInExtraBuffers;
    Zero::Array<float> SamplesThisFrame;
    unsigned MinimumSamplesNeededInBuffers;
    unsigned MinimumBufferSize;
  };
}