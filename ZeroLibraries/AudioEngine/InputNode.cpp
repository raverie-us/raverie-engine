///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------- Input Node

  //************************************************************************************************
  InputNode::InputNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundNode(status, name, ID, extInt, false, true, isThreaded), 
    WaitingForSamples(false), 
    Channels(1), 
    SampleRate(AudioSystemInternal::SampleRate),
    TotalSamplesInBuffers(0), 
    SamplesInExtraBuffers(0)
  {
    SetMinimumBufferSize();

    if (!Threaded)
      SetSiblingNodes(new InputNode(status, name, ID, nullptr, true), status);
    else
      SamplesThisFrame.Resize(1);
  }

  //************************************************************************************************
  InputNode::~InputNode()
  {
    while (!BufferList.Empty())
    {
      SampleBuffer& data = BufferList.Front();
      BufferList.PopFront();
      delete &data;
    }
  }

  //************************************************************************************************
  void InputNode::AddSamples(float* samples, const unsigned sampleCount)
  {
    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&InputNode::AddBufferToList, (InputNode*)GetSiblingNode(), 
        new SampleBuffer(samples, sampleCount)));
  }

  //************************************************************************************************
  unsigned InputNode::GetMinimumBufferSize()
  {
    return MinimumBufferSize;
  }

  //************************************************************************************************
  void InputNode::SetNumberOfChannels(const unsigned channels)
  {
    Channels = channels;
    SetMinimumBufferSize();

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&InputNode::SetNumberOfChannels, (InputNode*)GetSiblingNode(), channels));
    else if (Threaded)
      SamplesThisFrame.Resize(channels);
  }

  //************************************************************************************************
  unsigned InputNode::GetSystemSampleRate()
  {
    return AudioSystemInternal::SampleRate;
  }

  //************************************************************************************************
  unsigned InputNode::GetNumberOfChannels()
  {
    return Channels;
  }

  //************************************************************************************************
  void InputNode::AddBufferToList(InputNode::SampleBuffer* newBuffer)
  {
    if (!BufferList.Empty())
      SamplesInExtraBuffers += newBuffer->BufferSize;
    BufferList.PushBack(newBuffer);
    TotalSamplesInBuffers += newBuffer->BufferSize;

    if (TotalSamplesInBuffers > MinimumSamplesNeededInBuffers)
    {
      WaitingForSamples = false;
    }
    else if (GetSiblingNode())
    {
      unsigned samplesNeeded = MinimumSamplesNeededInBuffers - TotalSamplesInBuffers + MinimumSamplesNeededInBuffers;
      samplesNeeded -= samplesNeeded % Channels;
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
        GetSiblingNode(), Notify_NeedInputSamples, (void*)new InputNodeSampleRequest(samplesNeeded)));
    }
  }

  //************************************************************************************************
  void InputNode::SetMinimumBufferSize()
  {
    // System mix buffer size accounts for number of channels. 
    // Channels/SystemChannels accounts for differences in channels.
    MinimumBufferSize = gAudioSystem->MixBufferSizeThreaded * Channels 
      / gAudioSystem->SystemChannelsThreaded * 4;

    // Make sure the size is a multiple of the number of channels
    MinimumBufferSize -= MinimumBufferSize % Channels;

    MinimumSamplesNeededInBuffers = MinimumBufferSize * 3;
  }

  //************************************************************************************************
  bool InputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // If no sample data, just reset buffer to zero and return
    if (BufferList.Empty())
    {
      memset(outputBuffer->Data(), 0, sizeof(float) * outputBuffer->Size());
      if (!WaitingForSamples)
      {
        WaitingForSamples = true;
        if (GetSiblingNode())
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
            GetSiblingNode(), Notify_NeedInputSamples, 
            (void*)new InputNodeSampleRequest(MinimumSamplesNeededInBuffers * 2)));
      }
      return false;
    }

    SampleBuffer* samples = &BufferList.Front();
    unsigned outputBufferSize = outputBuffer->Size();
    float* outputBufferPosition = outputBuffer->Data();

    for (unsigned i = 0; i < outputBufferSize; i += numberOfChannels, outputBufferPosition += numberOfChannels)
    {
      memcpy(SamplesThisFrame.Data(), samples->Buffer + (samples->FrameIndex * Channels), sizeof(float) * Channels);

      // Reduce number of samples available
      TotalSamplesInBuffers -= Channels;

      // Advance sample buffer index by the number of samples in this frame
      ++samples->FrameIndex;
        

      // Check if we've reached the end of this buffer
      if (samples->FrameIndex * Channels >= samples->BufferSize)
      {
        // Remove buffer from list and delete
        BufferList.PopFront();
        Zero::SafeDelete(samples);

        //ResampleFrameIndex = 0;

        if (!BufferList.Empty())
          SamplesInExtraBuffers -= BufferList.Front().BufferSize;

        // Only need to do the following steps if more samples are needed
        if (i + numberOfChannels < outputBufferSize)
        {
          // Check if the list is empty
          if (BufferList.Empty())
          {
            // Set the rest of the output buffer to zero
            memset(outputBufferPosition + numberOfChannels, 0, sizeof(float) * (outputBufferSize - (i + numberOfChannels)));
            TotalSamplesInBuffers = 0;

            // Stop copying samples
            break;
          }
          else
          {
            // Get the next buffer
            samples = &BufferList.Front();
          }
        }
      }

      // Channels match, can just copy
      if (Channels == numberOfChannels)
        memcpy(outputBufferPosition, SamplesThisFrame.Data(), sizeof(float) * Channels);
      // Copy samples into output buffer, adjusting for channel differences
      else
      {
        AudioFrame frame(SamplesThisFrame.Data(), Channels);
        frame.TranslateChannels(numberOfChannels);
        memcpy(outputBufferPosition, frame.Samples, sizeof(float) * numberOfChannels);
      }
    }

    if (!WaitingForSamples && TotalSamplesInBuffers <= MinimumSamplesNeededInBuffers)
    {
      WaitingForSamples = true;
      unsigned samplesNeeded = MinimumSamplesNeededInBuffers - TotalSamplesInBuffers + MinimumSamplesNeededInBuffers;
      samplesNeeded -= samplesNeeded % Channels;
      if (GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
          GetSiblingNode(), Notify_NeedInputSamples, (void*)new InputNodeSampleRequest(samplesNeeded)));
    }

    return true;
  }



}