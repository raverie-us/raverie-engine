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
  RecordNode::RecordNode(Zero::Status& status, Zero::StringParam name, unsigned ID,
    ExternalNodeInterface* extInt, bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, false, isThreaded),
    MaxValue((float)((1 << 15) - 1)),
    FileName("RecordedOutput.wav"),
    Recording(false),
    Channels(0),
    Streaming(true),
    Paused(false)
  {
    if (!Threaded)
      SetSiblingNodes(new RecordNode(status, name, ID, nullptr, true), status);
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

    // Move input to output buffer
    InputSamples.Swap(*outputBuffer);

    if (Recording && !Paused && firstRequest && GetSiblingNode())
    {
      Zero::Array<float>* buffer = new Zero::Array<float>(*outputBuffer);
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&RecordNode::WriteBuffer,
        (RecordNode*)GetSiblingNode(), buffer, numberOfChannels));
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

          gAudioSystem->AddTask(Zero::CreateFunctor(&RecordNode::StartRecording,
            (RecordNode*)GetSiblingNode()));
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
            gAudioSystem->SystemSampleRate, // sampling rate
            gAudioSystem->SystemSampleRate * Channels * 16 / 8, // bytes per second
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
            for (unsigned i = 0; i < SavedSamples.Size(); ++i)
            {
              // Convert from float to short
              short shortValue = (short)(SavedSamples[i] * MaxValue);

              FileStream.Write(reinterpret_cast<byte*>(&shortValue), sizeof(short));
            }

            SavedSamples.Clear();
          }

          FileStream.Close();
        }

        if (GetSiblingNode())
          gAudioSystem->AddTask(Zero::CreateFunctor(&RecordNode::StopRecording,
          (RecordNode*)GetSiblingNode()));
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

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&RecordNode::SetPaused,
      (RecordNode*)GetSiblingNode(), paused));
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

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&RecordNode::SetStreamToDisk,
      (RecordNode*)GetSiblingNode(), streamToDisk));
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
        unsigned bufferSize = buffer->Size();
        for (unsigned i = 0; i < bufferSize; ++i)
        {
          short shortValue = (short)((*buffer)[i] * MaxValue);

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

}