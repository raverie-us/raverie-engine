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
  CustomDataNode::CustomDataNode(Zero::StringParam name, const unsigned ID,
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundNode(name, ID, extInt, false, true, isThreaded), 
    WaitingForSamples(false), 
    Channels(1), 
    TotalSamplesInBuffers(0), 
    SamplesInExtraBuffers(0)
  {
    SetMinimumBufferSize();

    if (!Threaded)
      SetSiblingNodes(new CustomDataNode(name, ID, nullptr, true));
    else
      SamplesThisFrame.Resize(1);
  }

  //************************************************************************************************
  CustomDataNode::~CustomDataNode()
  {
    if (!Threaded)
      return;

    while (!BufferList.Empty())
    {
      SampleBuffer& data = BufferList.Front();
      BufferList.PopFront();
      delete &data;
    }
  }

  //************************************************************************************************
  void CustomDataNode::AddSamples(float* samples, const unsigned sampleCount)
  {
    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&CustomDataNode::AddBufferToList, 
        (CustomDataNode*)GetSiblingNode(), new SampleBuffer(samples, sampleCount)));
  }

  //************************************************************************************************
  unsigned CustomDataNode::GetMinimumBufferSize()
  {
    return MinimumBufferSize;
  }

  //************************************************************************************************
  void CustomDataNode::SetNumberOfChannels(const unsigned channels)
  {
    Channels = channels;
    SetMinimumBufferSize();

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&CustomDataNode::SetNumberOfChannels, 
        (CustomDataNode*)GetSiblingNode(), channels));
    else if (Threaded)
      SamplesThisFrame.Resize(channels);
  }

  //************************************************************************************************
  unsigned CustomDataNode::GetSystemSampleRate()
  {
    return SystemSampleRate;
  }

  //************************************************************************************************
  unsigned CustomDataNode::GetNumberOfChannels()
  {
    return Channels;
  }

  //************************************************************************************************
  void CustomDataNode::AddBufferToList(CustomDataNode::SampleBuffer* newBuffer)
  {
    if (!Threaded)
      return;

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
      unsigned samplesNeeded = MinimumSamplesNeededInBuffers - TotalSamplesInBuffers 
        + MinimumSamplesNeededInBuffers;
      samplesNeeded -= samplesNeeded % Channels;
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventDataToExternalData, 
        GetSiblingNode(), (EventData*)(new EventData1<unsigned>(AudioEventTypes::NeedInputSamples, samplesNeeded))));
    }
  }

  //************************************************************************************************
  void CustomDataNode::SetMinimumBufferSize()
  {
    if (Channels > 0)
    {
      MinimumBufferSize = (unsigned)(SystemSampleRate * 0.01f * Channels * 4);

      MinimumSamplesNeededInBuffers = MinimumBufferSize * 3;
    }
    else
      MinimumBufferSize = 0;
  }

  //************************************************************************************************
  bool CustomDataNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // If no sample data, check if we need to request more samples and return
    if (BufferList.Empty())
    {
      if (!WaitingForSamples)
      {
        WaitingForSamples = true;
        if (GetSiblingNode())
        {
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventDataToExternalData,
            GetSiblingNode(), (EventData*)(new EventData1<unsigned>(AudioEventTypes::NeedInputSamples,
              MinimumSamplesNeededInBuffers * 2))));
        }
      }
      return false;
    }

    SampleBuffer* samples = &BufferList.Front();
    unsigned outputBufferSize = outputBuffer->Size();
    float* outputBufferPosition = outputBuffer->Data();

    for (unsigned i = 0; i < outputBufferSize; i += numberOfChannels, outputBufferPosition += numberOfChannels)
    {
      memcpy(SamplesThisFrame.Data(), samples->Buffer + (samples->FrameIndex * Channels), 
        sizeof(float) * Channels);

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
            memset(outputBufferPosition + numberOfChannels, 0, sizeof(float) * 
              (outputBufferSize - (i + numberOfChannels)));
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
      unsigned samplesNeeded = MinimumSamplesNeededInBuffers - TotalSamplesInBuffers 
        + MinimumSamplesNeededInBuffers;
      samplesNeeded -= samplesNeeded % Channels;
      if (GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventDataToExternalData,
          GetSiblingNode(), (EventData*)(new EventData1<unsigned>(AudioEventTypes::NeedInputSamples, samplesNeeded))));
    }

    return true;
  }



}