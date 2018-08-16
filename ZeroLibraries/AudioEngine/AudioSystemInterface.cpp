///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------- Audio System Interface

  //************************************************************************************************
  AudioSystemInterface::AudioSystemInterface(ExternalSystemInterface* extInterface) : 
    SendUncompressedInputData(false),
    SendCompressedInputData(false),
    PeakInputVolume(0.0f)
  {
    // Create the internal audio system
    System = new AudioSystemInternal(extInterface);
    SystemOutputChannels = System->SystemChannelsThreaded;
  }

  //************************************************************************************************
  AudioSystemInterface::~AudioSystemInterface()
  {
    // Delete the internal audio system
    delete System;
  }

  //************************************************************************************************
  void AudioSystemInterface::Update()
  {
    // If not threaded, run decoding tasks and mix loop
    if (!Zero::ThreadingEnabled)
    {
      for (unsigned i = 0; i < System->MaxDecodingTasksToRun && !System->DecodingTasks.Empty(); ++i)
      {
        System->DecodingTasks.Front()->RunDecodingTask();
        System->DecodingTasks.PopFront();
      }

      System->MixLoopThreaded();
    }

    // Run tasks from the mix thread
    System->HandleTasks();

    // Delete objects on the to-be-deleted list
    forRange(TagObject* object, System->TagsToDelete.All())
      delete object;
    System->TagsToDelete.Clear();

    // Check if we need to send microphone input to the external system
    if (SendCompressedInputData || SendUncompressedInputData)
    {
      Zero::Array<float> allInput;
      Zero::Array<float>* inputBuffer;

      // Pull all input buffers out of the queue
      while (System->InputDataQueue.Read(inputBuffer))
      {
        // Make sure that we don't send too many samples (will create latency if they are
        // using this data for audio output)
        if (allInput.Size() > MaxInputFrames * SystemOutputChannels)
          allInput = *inputBuffer;
        else
          allInput.Append(inputBuffer->All());

        delete inputBuffer;
      }

      // Save the peak volume from the input samples
      PeakInputVolume = 0.0f;
      forRange(float sample, allInput.All())
      {
        float absSample = Math::Abs(sample);
        if (absSample > PeakInputVolume)
          PeakInputVolume = absSample;
      }

      // If we're sending uncompressed input data, send the entire buffer
      if (SendUncompressedInputData)
      {
        System->ExternalInterface->SendAudioEventData((EventData*)
          (new EventData1<BufferType*>(AudioEventTypes::MicInputData, &allInput)));
      }

      // Check if we're sending compressed data and have an encoder
      if (SendCompressedInputData)
      {
        // Add the input samples to the end of the buffer
        PreviousInputSamples.Append(allInput.All());

        unsigned totalPacketSamples = PacketEncoder::PacketFrames * SystemOutputChannels;

        // While we have at least the number of samples for a packet, encode them
        while (PreviousInputSamples.Size() > totalPacketSamples)
        {
          Zero::Array<float> monoSamples;

          // If the system is in mono, just add samples
          if (SystemOutputChannels == 1)
            monoSamples.Append(PreviousInputSamples.SubRange(0, PacketEncoder::PacketFrames));
          else
          {
            // Translate samples to mono
            BufferRange sampleRange = PreviousInputSamples.All();
            for (unsigned i = 0; i < totalPacketSamples; i += SystemOutputChannels)
            {
              float monoValue(0.0f);
              for (unsigned j = 0; j < SystemOutputChannels; ++j, sampleRange.PopFront())
                monoValue += sampleRange.Front();
              monoValue /= SystemOutputChannels;
              monoSamples.PushBack(monoValue);
            }
          }

          // Remove the samples from the array
          PreviousInputSamples.Erase(PreviousInputSamples.SubRange(0, totalPacketSamples));

          // Encode the packet
          Zero::Array<byte> dataArray;
          System->Encoder.EncodePacket(monoSamples.Data(), PacketEncoder::PacketFrames, dataArray);

          // Send the event with the encoded data
          System->ExternalInterface->SendAudioEventData((EventData*)
            (new EventData1<Zero::Array<byte>*>(AudioEventTypes::CompressedMicInputData, &dataArray)));
        }
      }
    }
  }

  //************************************************************************************************
  void AudioSystemInterface::StartSystem(Zero::Status &status)
  {
    System->StartSystem(status);
  }

  //************************************************************************************************
  void AudioSystemInterface::StopSystem(Zero::Status &status)
  {
    System->StopSystem(status);
  }

  //************************************************************************************************
  float AudioSystemInterface::GetVolume()
  {
    // Return the non-threaded volume variable
    return System->Volume;
  }

  //************************************************************************************************
  void AudioSystemInterface::SetVolume(const float newVolume)
  {
    // Set the non-threaded volume variable
    System->Volume = newVolume;

    // Send an asynchronous task to the threaded system
    System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SystemVolumeThreaded, System, newVolume));
  }

  //************************************************************************************************
  bool AudioSystemInterface::GetMuteAllAudio()
  {
    return System->Muted;
  }

  //************************************************************************************************
  void AudioSystemInterface::SetMuteAllAudio(const bool muteAudio)
  {
    // Set the non-threaded variable
    System->Muted = muteAudio;

    // Send an asynchronous task to the threaded system
    System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetMutedThreaded, System, muteAudio));
  }

  //************************************************************************************************
  unsigned AudioSystemInterface::GetSampleRate()
  {
    return SystemSampleRate;
  }

  //************************************************************************************************
  unsigned AudioSystemInterface::GetOutputChannels()
  {
    return SystemOutputChannels;
  }

  //************************************************************************************************
  void AudioSystemInterface::SetOutputChannels(const unsigned channels)
  {
    // Can only mix channels up to 7.1
    if (channels != SystemOutputChannels && channels <= 8)
    {
      if (channels == 0)
      {
        SystemOutputChannels = System->AudioIO->GetStreamChannels(StreamTypes::Output);
        System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetSystemChannelsThreaded, System,
          SystemOutputChannels));
      }
      else
      {
        System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetSystemChannelsThreaded, System,
          channels));
        SystemOutputChannels = channels;
      }
    }
  }

  //************************************************************************************************
  void AudioSystemInterface::AddNodeToOutput(SoundNode* node)
  {
    if (System->FinalOutputNode)
      System->FinalOutputNode->AddInput(node);
  }

  //************************************************************************************************
  void AudioSystemInterface::RemoveNodeFromOutput(SoundNode* node)
  {
    if (System->FinalOutputNode)
      System->FinalOutputNode->RemoveInput(node);
  }

  //************************************************************************************************
  float AudioSystemInterface::GetPeakOutputVolume()
  {
    return System->PeakVolumeLastMix;
  }

  //************************************************************************************************
  float AudioSystemInterface::GetRMSOutputVolume()
  {
    return System->RmsVolumeLastMix;
  }

  //************************************************************************************************
  void AudioSystemInterface::SetMinimumVolumeThreshold(const float volume)
  {
    System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetMinVolumeThresholdThreaded, System, 
      volume));
  }

  //************************************************************************************************
  void AudioSystemInterface::UseHighLatency(const bool useHigh)
  {
    System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetLatencyThreaded, System, useHigh));
  }

  //************************************************************************************************
  void AudioSystemInterface::SetSendUncompressedMicInput(const bool sendInput)
  {
    if (sendInput != SendUncompressedInputData)
    {
      // Both modes now off - turn off sending input data
      if (!sendInput && !SendCompressedInputData)
        SetSendMicInput(false);
      // Both modes were off but this one is on - turn on sending input data
      else if (sendInput && !SendCompressedInputData)
        SetSendMicInput(true);

      SendUncompressedInputData = sendInput;
    }
  }

  //************************************************************************************************
  void AudioSystemInterface::SetSendCompressedMicInput(const bool sendInput)
  {
    if (sendInput != SendCompressedInputData)
    {
      // Both modes now off - turn off sending input data
      if (!sendInput && !SendUncompressedInputData)
        SetSendMicInput(false);
      // Both modes were off but this one is on - turn on sending input data
      // and initialize (or re-initialize) the encoder
      else if (sendInput && !SendUncompressedInputData)
      {
        System->Encoder.InitializeEncoder();
        SetSendMicInput(true);
      }

      SendCompressedInputData = sendInput;
    }
  }

  //************************************************************************************************
  float AudioSystemInterface::GetPeakInputVolume(Zero::Status& status)
  {
    if (SendUncompressedInputData || SendCompressedInputData)
      return PeakInputVolume;
    else
    {
      status.SetFailed("To get PeakInputVolume turn on sending microphone data, either uncompressed or compressed");
      return 0.0f;
    }
  }

  //************************************************************************************************
  void AudioSystemInterface::SetSendMicInput(const bool turnOn)
  {
    if (turnOn)
    {
      // Turn on sending input data
      System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetSendMicInputData,
        System, true));

      // Flush the input queue
      Zero::Array<float>* inputBuffer;
      while (System->InputDataQueue.Read(inputBuffer))
      {

      }
    }
    else
      System->AddTask(Zero::CreateFunctor(&AudioSystemInternal::SetSendMicInputData,
        System, false));
  }

  //------------------------------------------------------------------------------------- Audio File

  //************************************************************************************************
  AudioFile::AudioFile() :
    Channels(0),
    FileLength(0)
  {
    ZeroConstructPrivateData(AudioFileData);
  }

  //************************************************************************************************
  AudioFile::~AudioFile()
  {
    Close();
    ZeroDestructPrivateData(AudioFileData);
  }

  //************************************************************************************************
  void AudioFile::OpenFile(Zero::Status& status, Zero::StringParam fileName)
  {
    ZeroGetPrivateData(AudioFileData);

    if (self->BuffersPerChannel)
    {
      status.SetFailed("Already open, call Close before opening another file");
    }
    else
    {
      *self = FileEncoder::OpenFile(status, fileName);

      if (status.Succeeded())
      {
        Channels = self->Channels;
        FileLength = (float)self->SamplesPerChannel / (float)self->SampleRate;
      }
    }
  }

  //************************************************************************************************
  void AudioFile::WriteEncodedFile(Zero::Status& status, Zero::StringParam outputFileName, 
    bool normalize, float maxVolume)
  {
    ZeroGetPrivateData(AudioFileData);

    if (self->BuffersPerChannel)
      FileEncoder::WriteFile(status, outputFileName, *self, normalize, maxVolume);
    else
      status.SetFailed("No input file was opened");
  }

  //************************************************************************************************
  void AudioFile::Close()
  {
    ZeroGetPrivateData(AudioFileData);

    self->ReleaseData();
  }

  //--------------------------------------------------------------------------- Audio Stream Decoder

  //************************************************************************************************
  AudioStreamDecoder::AudioStreamDecoder()
  {
    Decoder = new SingleChannelPacketDecoder();
    Decoder->InitializeDecoder();
  }

  //************************************************************************************************
  void AudioStreamDecoder::DecodeCompressedPacket(const byte* encodedData, const unsigned dataSize, 
    float*& decodedSamples, unsigned& sampleCount)
  {
    Decoder->DecodePacket(encodedData, dataSize, decodedSamples, sampleCount);
  }

}