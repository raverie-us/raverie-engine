///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

namespace Events
{
  DeclareEvent(CustomAudioNodeSamplesNeeded);
}

//-------------------------------------------------------------------------- Custom Audio Node Event

class CustomAudioNodeEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CustomAudioNodeEvent(unsigned samples) :
    SamplesNeeded(samples)
  {}

  unsigned SamplesNeeded;
};

//------------------------------------------------------------------------------------- Sound Buffer

/// Used with a CustomAudioNode to play audio data directly
class SoundBuffer : public ReferenceCountedObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Adds a new audio sample to the end of the buffer.
  void AddSampleToBuffer(float sample);
  /// The number of samples currently in the buffer.
  int GetSampleCount();
  /// Returns the sample at a specific index from the beginning of the buffer.
  float GetSampleAtIndex(int index);
  /// Removes all data from the buffer and resets it.
  void Reset();
  /// Takes the AudioData from a MicrophoneUncompressedFloatData event and adds all of 
  /// the audio samples to the buffer
  void AddMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData);

// Internals
  Zero::Array<float> mBuffer;
};

//-------------------------------------------------------------------------------- Custom Audio Node

/// Uses a SoundBuffer to send audio data directly to the audio engine
class CustomAudioNode : public SoundNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  CustomAudioNode(StringParam name, unsigned ID);
  ~CustomAudioNode();

  /// The minimum number of samples that should be sent when a NeedMoreSamples event is received.
  int GetMinimumBufferSize();
  /// The sample rate currently being used by the audio system.
  int GetSystemSampleRate();
  /// The number of audio channels that will be in the buffer.
  int GetChannels();
  void SetChannels(int numChannels);
  /// Sends a buffer of audio samples to the audio system for output.
  void SendBuffer(SoundBuffer* buffer);
  /// Sends a partial buffer of audio samples to the audio system for output.
  void SendPartialBuffer(SoundBuffer* buffer, int startAtIndex, int howManySamples);
  /// Takes the AudioData from a MicrophoneUncompressedFloatData event and sends all of 
  /// the audio samples to the audio engine for output
  void SendMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData);
  /// Takes the AudioData from a MicrophoneCompressedByteData event, decompresses the data,
  /// and sends all of the audio samples to the audio engine for output
  void SendMicCompressedData(const HandleOf<ArrayClass<byte>>& audioData);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;

  class SampleBuffer
  {
  public:
    SampleBuffer(float* buffer, unsigned size);
    ~SampleBuffer();

    float* mBuffer;
    unsigned mBufferSize;
    unsigned mFrameIndex;

    Link<SampleBuffer> link;
  };

  void AddBufferThreaded(SampleBuffer* newBuffer);
  void SetMinimumBufferSize();
  void DispatchSamplesEvent(unsigned samplesNeeded);

  SingleChannelPacketDecoder* mAudioDecoder;
  InList<SampleBuffer> mBufferListThreaded;

  bool mWaitingForSamplesThreaded;
  Threaded<unsigned> mChannels;
  unsigned mTotalSamplesInBuffersThreaded;
  unsigned mSamplesInExtraBuffersThreaded;
  unsigned mMinimumSamplesNeededInBuffersThreaded;
  unsigned mMinimumBufferSize;
};


}
