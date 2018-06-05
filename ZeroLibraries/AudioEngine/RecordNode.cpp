///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------ Record Node

  //************************************************************************************************
  RecordNode::RecordNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
      bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    FileName("RecordedOutput.wav"),
    MaxValue((float)((1 << 15) - 1)),
    Recording(false),
    Paused(false),
    Channels(0),
    Streaming(true)
  {
    if (!Threaded)
      SetSiblingNodes(new RecordNode(name, ID, nullptr, true));
  }

  //************************************************************************************************
  RecordNode::~RecordNode()
  {
    if (Recording)
      StopRecording();
  }

  //************************************************************************************************
  bool RecordNode::GetOutputSamples(BufferType* outputBuffer, unsigned const numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // Get input
    bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

    // If there is input data, move input to output buffer
    if (isThereOutput)
      InputSamples.Swap(*outputBuffer);

    // If we are recording, not paused, this is the first time input was requested, and we still
    // have a sibling node, create a task to write the data to the file
    if (Recording && !Paused && firstRequest && GetSiblingNode())
    {
      // If there was no input data, set the buffer to zero
      if (!isThereOutput)
        memset(outputBuffer->Data(), 0, sizeof(float) * outputBuffer->Size());

      Zero::Array<float>* buffer = new Zero::Array<float>(*outputBuffer);
      AddTaskForSiblingThreaded(&RecordNode::WriteBuffer, buffer, numberOfChannels);
    }

    return isThereOutput;
  }

  //************************************************************************************************
  void RecordNode::SetFileName(Zero::StringParam newFileName)
  {
    if (Threaded)
      return;

    FileName = Zero::String::Format("%s.wav", newFileName.c_str());
  }

  //************************************************************************************************
  void RecordNode::StartRecording()
  {
    if (!Threaded)
    {
      if (!Recording && GetSiblingNode())
      {
        FileStream.Open(FileName.c_str(), Zero::FileMode::Write, Zero::FileAccessPattern::Sequential);
        if (FileStream.IsOpen())
        {
          Recording = true;
          SamplesRecorded = 0;

          WavHeader header;
          FileStream.Write(reinterpret_cast<byte*>(&header), sizeof(header));

          AddTaskForSibling(&RecordNode::StartRecording);
        }
      }
    }
    else
      Recording = true;
  }

  //************************************************************************************************
  void RecordNode::StopRecording()
  {
    if (!Threaded)
    {
      if (Recording)
      {
        Recording = false;

        if (FileStream.IsOpen())
        {
          WavHeader header =
          {
            { 'R', 'I', 'F', 'F' },
            36 + (SamplesRecorded * 2),
            { 'W', 'A', 'V', 'E' },
            { 'f', 'm', 't', ' ' },
            16, // fmt chunk size
            1, // audio format
            (unsigned short)Channels, // number of channels
            cSystemSampleRate, // sampling rate
            cSystemSampleRate * Channels * 16 / 8, // bytes per second
            2 * 16 / 8, // bytes per sample
            16, // bits per sample
            { 'd', 'a', 't', 'a' },
            SamplesRecorded * 2
          };

          FileStream.Seek(Zero::FilePosition(0));
          FileStream.Write(reinterpret_cast<byte*>(&header), sizeof(header));

          // If samples are being saved, write them to the file
          if (!Streaming)
          {
            forRange(float& sample, SavedSamples.All())
            {
              // Convert from float to short
              short shortValue = (short)(sample * MaxValue);

              FileStream.Write(reinterpret_cast<byte*>(&shortValue), sizeof(short));
            }

            SavedSamples.Clear();
          }

          FileStream.Close();
        }

        AddTaskForSibling(&RecordNode::StopRecording);
      }
    }
    else
      Recording = false;
  }

  //************************************************************************************************
  bool RecordNode::GetPaused()
  {
    return Paused;
  }

  //************************************************************************************************
  void RecordNode::SetPaused(const bool paused)
  {
    Paused = paused;

    if (!Threaded)
      AddTaskForSibling(&RecordNode::SetPaused, paused);
  }

  //************************************************************************************************
  bool RecordNode::GetStreamToDisk()
  {
    return Streaming;
  }

  //************************************************************************************************
  void RecordNode::SetStreamToDisk(const bool streamToDisk)
  {
    Streaming = streamToDisk;

    if (!Threaded)
      AddTaskForSibling(&RecordNode::SetStreamToDisk, streamToDisk);
  }

  //************************************************************************************************
  void RecordNode::WriteBuffer(Zero::Array<float>* buffer, unsigned numberOfChannels)
  {
    if (Threaded)
      return;

    Channels = numberOfChannels;

    if (FileStream.IsOpen())
    {
      if (Streaming)
      {
        forRange(float& sample, buffer->All())
        {
          short shortValue = (short)(sample * MaxValue);

          FileStream.Write(reinterpret_cast<byte*>(&shortValue), sizeof(short));
          ++SamplesRecorded;
        }
      }
      else
      {
        SavedSamples.Append(buffer->All());

        SamplesRecorded += buffer->Size();
      }
    }

    delete buffer;
  }


  //-------------------------------------------------------------------------------- Save Audio Node

  //************************************************************************************************
  SaveAudioNode::SaveAudioNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
      bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    mSaveData(false),
    mPlayData(false),
    mPlaybackIndex(0)
  {
    if (!Threaded)
      SetSiblingNodes(new SaveAudioNode(name, ID, nullptr, true));
  }

  //************************************************************************************************
  bool SaveAudioNode::GetSaveAudio()
  {
    return mSaveData;
  }

  //************************************************************************************************
  void SaveAudioNode::SetSaveAudio(bool save)
  {
    if (!Threaded)
      AddTaskForSibling(&SaveAudioNode::SetSaveAudio, save);
    else if (save)
      ClearSavedAudio();

    mSaveData = save;
  }

  //************************************************************************************************
  void SaveAudioNode::PlaySavedAudio()
  {
    if (!Threaded)
      AddTaskForSibling(&SaveAudioNode::PlaySavedAudio);

    mPlayData = true;
  }

  //************************************************************************************************
  void SaveAudioNode::StopPlaying()
  {
    if (!Threaded)
      AddTaskForSibling(&SaveAudioNode::StopPlaying);

    mPlayData = false;
  }

  //************************************************************************************************
  void SaveAudioNode::ClearSavedAudio()
  {
    if (!Threaded)
      AddTaskForSibling(&SaveAudioNode::ClearSavedAudio);
    else
    {
      mSavedSamples.Clear();
      mPlaybackIndex = 0;
    }
  }

  //************************************************************************************************
  bool SaveAudioNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // Get input data
    bool isInputData = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

    // If there is input data and we are saving, append the samples to the buffer
    if (mSaveData && isInputData)
      CopyIntoBuffer(&mSavedSamples, InputSamples, 0, outputBuffer->Size());

    // If there is input data, move it to the output buffer
    if (isInputData)
      outputBuffer->Swap(InputSamples);

    // Check if we are playing saved data
    if (mPlayData)
    {
      // The samples to copy can't be more than the samples available
      size_t samplesToCopy = Math::Min(outputBuffer->Size(), mSavedSamples.Size() - mPlaybackIndex);

      // If there's no input data, copy the saved samples into the output buffer
      if (!isInputData)
      {
        memcpy(outputBuffer->Data(), mSavedSamples.Data() + mPlaybackIndex, sizeof(float) * samplesToCopy);

        // If there are extra samples in the output buffer, set them all to zero
        if (samplesToCopy < outputBuffer->Size())
        {
          memset(outputBuffer->Data() + (outputBuffer->Size() - samplesToCopy), 0,
            outputBuffer->Size() - samplesToCopy);
        }
      }
      // If there is input data, add the saved samples to the existing data
      else
      {
        for (size_t i = 0; i < samplesToCopy; ++i)
          (*outputBuffer)[i] += mSavedSamples[mPlaybackIndex + i];
      }

      // Move the playback index forward
      mPlaybackIndex += samplesToCopy;
      // Check if we've reached the end of the saved data
      if (mPlaybackIndex >= mSavedSamples.Size())
      {
        mPlayData = false;
        mPlaybackIndex = 0;
      }

      // Mark that we do have valid data to return
      isInputData = true;
    }

    return isInputData;
  }

}
