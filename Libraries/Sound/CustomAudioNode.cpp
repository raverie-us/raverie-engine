///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{

DefineEvent(CustomAudioNodeSamplesNeeded);

} // namespace Events

//-------------------------------------------------------------------------- Custom Audio Node Event

//**************************************************************************************************
ZilchDefineType(CustomAudioNodeEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(SamplesNeeded);
}

//------------------------------------------------------------------------------------- Sound Buffer

//**************************************************************************************************
ZilchDefineType(SoundBuffer, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetter(SampleCount);
  ZilchBindMethod(AddSampleToBuffer);
  ZilchBindMethod(GetSampleAtIndex);
  ZilchBindMethod(Reset);
  ZilchBindMethod(AddMicUncompressedData);
}

//**************************************************************************************************
void SoundBuffer::AddSampleToBuffer(float value)
{
  mBuffer.PushBack(Math::Clamp(value, -1.0f, 1.0f));
}

//**************************************************************************************************
int SoundBuffer::GetSampleCount()
{
  return mBuffer.Size();
}

//**************************************************************************************************
float SoundBuffer::GetSampleAtIndex(int index)
{
  if ((unsigned)index < mBuffer.Size())
    return mBuffer[index];
  else
    return 0.0f;
}

//**************************************************************************************************
void SoundBuffer::Reset()
{
  mBuffer.Clear();
}

//**************************************************************************************************
void SoundBuffer::AddMicUncompressedData(const HandleOf<ArrayClass<float>>& buffer)
{
  mBuffer.Append(buffer->NativeArray.All());
}

//-------------------------------------------------------------------------------- Custom Audio Node

//**************************************************************************************************
ZilchDefineType(CustomAudioNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Channels);
  ZilchBindGetter(MinimumBufferSize);
  ZilchBindGetter(SystemSampleRate);
  ZilchBindMethod(SendBuffer);
  ZilchBindMethod(SendPartialBuffer);
  ZilchBindMethod(SendMicUncompressedData);
  ZilchBindMethod(SendMicCompressedData);

  ZeroBindEvent(Events::CustomAudioNodeSamplesNeeded, CustomAudioNodeEvent);
}

//**************************************************************************************************
CustomAudioNode::CustomAudioNode(StringParam name, unsigned ID) :
  SoundNode(name, ID, false, true),
  mAudioDecoder(nullptr),
  mWaitingForSamplesThreaded(false),
  mChannels(1),
  mTotalSamplesInBuffersThreaded(0),
  mSamplesInExtraBuffersThreaded(0),
  mMinimumSamplesNeededInBuffersThreaded(0),
  mMinimumBufferSize(0)
{
  SetMinimumBufferSize();
}

//**************************************************************************************************
CustomAudioNode::~CustomAudioNode()
{
  if (mAudioDecoder)
    delete mAudioDecoder; 
  
  while (!mBufferListThreaded.Empty())
  {
    SampleBuffer& data = mBufferListThreaded.Front();
    mBufferListThreaded.PopFront();
    delete &data;
  }
}

//**************************************************************************************************
int CustomAudioNode::GetMinimumBufferSize()
{
  return mMinimumBufferSize;
}

//**************************************************************************************************
int CustomAudioNode::GetSystemSampleRate()
{
  return AudioConstants::cSystemSampleRate;
}

//**************************************************************************************************
int CustomAudioNode::GetChannels()
{
  return mChannels.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void CustomAudioNode::SetChannels(int channels)
{
  channels = Math::Clamp(channels, 0, 8);
  mChannels.Set(channels, AudioThreads::MainThread);
  SetMinimumBufferSize();
}

//**************************************************************************************************
void CustomAudioNode::SendBuffer(SoundBuffer* buffer)
{
  if (!buffer)
    DoNotifyException("Audio Error", "Called SendBuffer on CustomAudioNode with a null SoundBuffer");

  Z::gSound->Mixer.AddTask(CreateFunctor(&CustomAudioNode::AddBufferThreaded, this, 
    new SampleBuffer(buffer->mBuffer.Data(), buffer->mBuffer.Size())), this);
}

//**************************************************************************************************
void CustomAudioNode::SendPartialBuffer(SoundBuffer* buffer, int startAtIndex, int howManySamples)
{
  if (!buffer)
    DoNotifyException("Audio Error", "Called SendPartialBuffer on CustomAudioNode with a null SoundBuffer");
  else if (startAtIndex < 0 || ((startAtIndex + howManySamples) >(int)buffer->mBuffer.Size()))
    DoNotifyException("Audio Error", "SendPartialBuffer parameters exceed size of the SoundBuffer");

  Z::gSound->Mixer.AddTask(CreateFunctor(&CustomAudioNode::AddBufferThreaded, this,
    new SampleBuffer(buffer->mBuffer.Data() + startAtIndex, howManySamples)), this);
}

//**************************************************************************************************
void CustomAudioNode::SendMicUncompressedData(const HandleOf<ArrayClass<float>>& audioData)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&CustomAudioNode::AddBufferThreaded, this,
    new SampleBuffer(audioData->NativeArray.Data(), audioData->NativeArray.Size())), this);
}

//**************************************************************************************************
void CustomAudioNode::SendMicCompressedData(const HandleOf<ArrayClass<byte>>& audioData)
{
  // If we haven't created the decoder yet, create it
  if (!mAudioDecoder) 
  {
    mAudioDecoder = new SingleChannelPacketDecoder();
    mAudioDecoder->InitializeDecoder();
  }

  // Decode the compressed data
  float* decodedSamples;
  unsigned sampleCount;
  mAudioDecoder->DecodePacket(audioData->NativeArray.Data(), audioData->NativeArray.Size(),
    decodedSamples, sampleCount);

  Z::gSound->Mixer.AddTask(CreateFunctor(&CustomAudioNode::AddBufferThreaded, this,
    new SampleBuffer(decodedSamples, sampleCount)), this);
}

//**************************************************************************************************
bool CustomAudioNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // If no sample data, check if we need to request more samples and return
  if (mBufferListThreaded.Empty())
  {
    if (!mWaitingForSamplesThreaded)
    {
      mWaitingForSamplesThreaded = true;
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&CustomAudioNode::DispatchSamplesEvent, this,
        mMinimumSamplesNeededInBuffersThreaded * 2), this);
    }
    return false;
  }

  SampleBuffer* samples = &mBufferListThreaded.Front();
  unsigned outputBufferSize = outputBuffer->Size();
  float* outputBufferPosition = outputBuffer->Data();
  unsigned channels = mChannels.Get(AudioThreads::MixThread);
  Array<float> samplesThisFrame(channels);

  for (unsigned i = 0; i < outputBufferSize; i += numberOfChannels, outputBufferPosition += numberOfChannels)
  {
    memcpy(samplesThisFrame.Data(), samples->mBuffer + (samples->mFrameIndex * channels),
      sizeof(float) * channels);

    // Reduce number of samples available
    mTotalSamplesInBuffersThreaded -= channels;

    // Advance sample buffer index by the number of samples in this frame
    ++samples->mFrameIndex;
    
    // Check if we've reached the end of this buffer
    if (samples->mFrameIndex * channels >= samples->mBufferSize)
    {
      // Remove buffer from list and delete
      mBufferListThreaded.PopFront();
      SafeDelete(samples);

      //ResampleFrameIndex = 0;

      if (!mBufferListThreaded.Empty())
        mSamplesInExtraBuffersThreaded -= mBufferListThreaded.Front().mBufferSize;

      // Only need to do the following steps if more samples are needed
      if (i + numberOfChannels < outputBufferSize)
      {
        // Check if the list is empty
        if (mBufferListThreaded.Empty())
        {
          // Set the rest of the output buffer to zero
          memset(outputBufferPosition + numberOfChannels, 0, sizeof(float) *
            (outputBufferSize - (i + numberOfChannels)));
          mTotalSamplesInBuffersThreaded = 0;

          // Stop copying samples
          break;
        }
        else
        {
          // Get the next buffer
          samples = &mBufferListThreaded.Front();
        }
      }
    }

    // Channels match, can just copy
    if (channels == numberOfChannels)
      memcpy(outputBufferPosition, samplesThisFrame.Data(), sizeof(float) * channels);
    // Copy samples into output buffer, adjusting for channel differences
    else
    {
      AudioFrame frame(samplesThisFrame.Data(), channels);
      memcpy(outputBufferPosition, frame.GetSamples(numberOfChannels), sizeof(float) * numberOfChannels);
    }
  }

  if (!mWaitingForSamplesThreaded && mTotalSamplesInBuffersThreaded <= mMinimumSamplesNeededInBuffersThreaded)
  {
    mWaitingForSamplesThreaded = true;
    unsigned samplesNeeded = mMinimumSamplesNeededInBuffersThreaded - mTotalSamplesInBuffersThreaded
      + mMinimumSamplesNeededInBuffersThreaded;
    samplesNeeded -= samplesNeeded % channels;
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&CustomAudioNode::DispatchSamplesEvent, this,
      samplesNeeded), this);
  }

  return true;
}

//**************************************************************************************************
void CustomAudioNode::AddBufferThreaded(SampleBuffer* newBuffer)
{
  if (!mBufferListThreaded.Empty())
    mSamplesInExtraBuffersThreaded += newBuffer->mBufferSize;
  mBufferListThreaded.PushBack(newBuffer);
  mTotalSamplesInBuffersThreaded += newBuffer->mBufferSize;

  if (mTotalSamplesInBuffersThreaded > mMinimumSamplesNeededInBuffersThreaded)
    mWaitingForSamplesThreaded = false;
  else
  {
    unsigned samplesNeeded = mMinimumSamplesNeededInBuffersThreaded - mTotalSamplesInBuffersThreaded
      + mMinimumSamplesNeededInBuffersThreaded;
    samplesNeeded -= samplesNeeded % mChannels.Get(AudioThreads::MixThread);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&CustomAudioNode::DispatchSamplesEvent, this,
      samplesNeeded), this);
  }
}

//**************************************************************************************************
void CustomAudioNode::SetMinimumBufferSize()
{
  mMinimumBufferSize = (unsigned)(AudioConstants::cSystemSampleRate * 0.01f 
    * mChannels.Get(AudioThreads::MainThread) * 4);

  Z::gSound->Mixer.AddTask(CreateFunctor(&CustomAudioNode::mMinimumSamplesNeededInBuffersThreaded,
    this, mMinimumBufferSize * 3), this);
}

//**************************************************************************************************
void CustomAudioNode::DispatchSamplesEvent(unsigned samplesNeeded)
{
  CustomAudioNodeEvent event(samplesNeeded);
  DispatchEvent(Events::CustomAudioNodeSamplesNeeded, &event);
}

//**************************************************************************************************
CustomAudioNode::SampleBuffer::SampleBuffer(float* buffer, unsigned size) : 
  mBuffer(buffer), 
  mBufferSize(size), 
  mFrameIndex(0)
{
  mBuffer = new float[mBufferSize];
  memcpy(mBuffer, buffer, sizeof(float) * mBufferSize);
}

//**************************************************************************************************
CustomAudioNode::SampleBuffer::~SampleBuffer()
{
  delete[] mBuffer;
}

} // namespace Zero
