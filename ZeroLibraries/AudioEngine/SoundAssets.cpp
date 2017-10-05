///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"


namespace Audio
{
  //------------------------------------------------------------------------------- Sound Asset Node

  //************************************************************************************************
  SoundAssetNode::SoundAssetNode(ExternalNodeInterface* externalInterface, const bool threaded) :
    ThreadedAsset(nullptr),
    ReferenceCount(0), 
    Threaded(threaded), 
    ExternalData(externalInterface)
  {
    // If not threaded, add to the system's list
    if (!Threaded)
      gAudioSystem->AddAsset(this);
  }

  //************************************************************************************************
  SoundAssetNode::~SoundAssetNode()
  {
    if (!Threaded)
    {
      // Delete the threaded asset (not tracked anywhere else)
      if (ThreadedAsset)
        delete ThreadedAsset;
      
      // Remove from the system's list
      gAudioSystem->RemoveAsset(this);
    }
  }

  //************************************************************************************************
  void SoundAssetNode::SetExternalInterface(ExternalNodeInterface* extInt)
  {
    if (Threaded)
      return;

    ExternalData = extInt;

    // If interface is now null and is not referenced, can delete now
    if (!extInt && ReferenceCount == 0)
      delete this;
  }

  //************************************************************************************************
  bool SoundAssetNode::IsPlaying()
  {
    if (Threaded)
      return false;

    return ReferenceCount > 0;
  }

  //************************************************************************************************
  bool SoundAssetNode::AddReference()
  {
    if (Threaded)
      return false;

    // Check if it's okay to add another instance to this asset
    if (OkayToAddInstance())
    {
      ++ReferenceCount;
      return true;
    }
    else
      return false;
  }

  //************************************************************************************************
  void SoundAssetNode::ReleaseReference()
  {
    if (Threaded)
      return;

    if (ReferenceCount == 0)
      Error("Trying to release reference on an unreferenced sound asset");

    --ReferenceCount;
    RemoveInstance();

    if (ReferenceCount == 0)
    {
      // If there is no external interface, delete now
      if (!ExternalData)
        delete this;
      else
        ExternalData->SendAudioEvent(Notify_AssetUnreferenced, (void*)nullptr);
    }
  }

  //-------------------------------------------------------------------------- Sound Asset From File

  //************************************************************************************************
  SoundAssetFromFile::SoundAssetFromFile(Zero::Status& status, const Zero::String& fileName, 
    const bool streaming, ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAssetNode(extInt, isThreaded), 
    HasStreamingInstance(false), 
    Streaming(streaming),
    FileLength(0),
    Channels(0),
    FrameCount(0),
    UndecodedSamplesIndex(0),
    Samples(nullptr),
    Decoder(nullptr),
    PreviousBufferSamples(0),
    NeedSecondBuffer(true),
    IndexCheck(0)
  {
    if (!Threaded)
    {
      ThreadedAsset = new SoundAssetFromFile(status, fileName, streaming, extInt, true);
      if (!status.Failed())
      {
        FileLength = ((SoundAssetFromFile*)ThreadedAsset)->FileLength;
        Channels = ((SoundAssetFromFile*)ThreadedAsset)->Channels;
        FrameCount = ((SoundAssetFromFile*)ThreadedAsset)->FrameCount;
      }
    }
    else
    {
      // Remember that this constructor happens on the game thread

      // Create the decoder object
      Decoder = new FileDecoder(status, fileName, streaming, this);

      // Make sure it was successful
      if (!status.Failed())
      {
        // Set the variables
        FileLength = (float)Decoder->SamplesPerChannel / AudioSystemInternal::SystemSampleRate;
        Channels = Decoder->Channels;
        FrameCount = Decoder->SamplesPerChannel;
        IndexCheck = AudioSystemInternal::SystemSampleRate * Channels / 2;

        // If not streaming, make the Samples buffer big enough to hold all the audio samples,
        // and decode one buffer so it's ready to go
        if (!Streaming)
        {
          Samples = new float[FrameCount * Channels];

          gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::AddDecodingTask, Decoder));
        }
        // Otherwise, make the Samples buffer the size of one packet
        else
        {
          unsigned sampleCount = FileEncoder::PacketFrames * Channels;
          Samples = new float[sampleCount];
          memset(Samples, 0, sizeof(float) * sampleCount);

          NextStreamedSamples = new float[sampleCount];
          memset(NextStreamedSamples, 0, sizeof(float) * sampleCount);
        }
      }
      // If not successful, delete the decoder object
      else
      {
        delete Decoder;
        Decoder = nullptr;
      }
    }
  }

  //************************************************************************************************
  SoundAssetFromFile::~SoundAssetFromFile()
  {
    if (Threaded)
    {
      if (Samples)
        delete[] Samples;

      if (Decoder)
      {
        // If the decoder isn't executing any tasks, go ahead and delete it
        if (AtomicCompareExchange32(&Decoder->DecodingTaskCount, 0, 0) == 0)
          delete Decoder;
        else
        {
          // Otherwise set the Asset pointer to null so it will delete itself
          AtomicSetPointer((void**)Decoder->ParentAlive, (void*)nullptr);
        }
      }
    }
  }

  //************************************************************************************************
  FrameData SoundAssetFromFile::GetFrame(const unsigned frameIndex)
  {
    if (!Threaded)
      return FrameData();

    FrameData data;
    data.HowManyChannels = Channels;

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * Channels;

    if (!Streaming)
    {
      if (Decoder)
        CheckForDecodedPacket();

      // Past end of file, return zeros
      if (frameIndex >= FrameCount || sampleIndex >= UndecodedSamplesIndex)
      {
        memset(data.Samples, 0, data.HowManyChannels * sizeof(float));
        return data;
      }
    }
    else
    {
      // Adjust the sample index
      sampleIndex -= PreviousBufferSamples;

      unsigned bufferSize = FileEncoder::PacketFrames * Channels;

      // If we've reached the end of the buffer, swap in the NextStreamedSamples buffer
      if (sampleIndex >= bufferSize)
      {
        // Copy the samples into the buffer
        memcpy(Samples, NextStreamedSamples, sizeof(float) * bufferSize);

        // Increase the previous samples count
        PreviousBufferSamples += bufferSize;
        // Decrease the sample index
        sampleIndex -= bufferSize;

        // Mark that we need a new second buffer
        NeedSecondBuffer = true;

        // If we're still past the end of the buffer, return zeros
        if (sampleIndex >= bufferSize)
        {
          memset(data.Samples, 0, data.HowManyChannels * sizeof(float));
          return data;
        }
      }

      if (NeedSecondBuffer)
        CheckForDecodedPacket();
    }

    // Get samples for all channels 
    memcpy(data.Samples, Samples + sampleIndex, Channels * sizeof(float));

    return data;
  }

  //************************************************************************************************
  void SoundAssetFromFile::GetBuffer(float* buffer, const unsigned frameIndex, 
    const unsigned numberOfSamples)
  {
    if (!Threaded)
      return;

    if (Streaming)
    {
      // TODO move decoded buffers forward

      memset(buffer, 0, sizeof(float) * numberOfSamples);
      return;
    }

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * Channels;

    if (Decoder)
      CheckForDecodedPacket();

    // If the number of samples would go past the available decoded samples, reduce the number
    unsigned samples = numberOfSamples;
    if (sampleIndex + numberOfSamples >= UndecodedSamplesIndex)
    {
      if (sampleIndex < UndecodedSamplesIndex)
        samples = UndecodedSamplesIndex - sampleIndex;
      else
        samples = 0;
    }

    // Copy the appropriate samples into the buffer
    memcpy(buffer, Samples + sampleIndex, samples * sizeof(float));

    // If the number of samples provided is less than what's requested, set the rest to zero
    if (samples < numberOfSamples)
      memset(buffer + samples, 0, (numberOfSamples - samples) * sizeof(float));
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetNumberOfFrames()
  {
    return FrameCount;
  }

  //************************************************************************************************
  bool SoundAssetFromFile::GetStreaming()
  {
    return Streaming;
  }

  //************************************************************************************************
  void SoundAssetFromFile::ResetStreamingFile()
  {
    if (!Threaded)
      return;

    DecodedPacket packet;
    while (Decoder->DecodedPacketQueue.Read(packet))
    {

    }

    Decoder->ResetStream();

    PreviousBufferSamples = 0;
    NeedSecondBuffer = true;
    memset(Samples, 0, sizeof(float) * FileEncoder::PacketFrames * Channels);
    memset(NextStreamedSamples, 0, sizeof(float) * FileEncoder::PacketFrames * Channels);
  }

  //************************************************************************************************
  float SoundAssetFromFile::GetLengthOfFile()
  {
    return FileLength;
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetChannels()
  {
    return Channels;
  }

  //************************************************************************************************
  bool SoundAssetFromFile::OkayToAddInstance()
  {
    if (Threaded)
      return false;

    // Not streaming, can play multiple instances
    if (!Streaming)
      return true;
    else
    {
      // Already streaming one instance, don't play another
      if (HasStreamingInstance)
        return false;
      else
      {
        HasStreamingInstance = true;

        ((SoundAssetFromFile*)ThreadedAsset)->Decoder->OpenStream();
        gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::AddDecodingTask, 
          ((SoundAssetFromFile*)ThreadedAsset)->Decoder));
        return true;
      }
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::RemoveInstance()
  {
    if (Threaded)
      return;

    if (HasStreamingInstance)
    {
      HasStreamingInstance = false;

      SoundAssetFromFile* threadedSibling = (SoundAssetFromFile*)ThreadedAsset;

      gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::CloseStream, threadedSibling->Decoder));

      threadedSibling->PreviousBufferSamples = 0;
      threadedSibling->NeedSecondBuffer = true;
      memset(threadedSibling->Samples, 0, sizeof(float) * FileEncoder::PacketFrames * Channels);
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::CheckForDecodedPacket()
  {
    if (!Threaded)
      return;

    if (!Decoder)
      return;

    if (Streaming && !NeedSecondBuffer)
      return;

    DecodedPacket packet;
    if (Decoder->DecodedPacketQueue.Read(packet))
    {
      if (!Streaming)
      {
        // Save the number of frames
        unsigned undecodedFrames = UndecodedSamplesIndex / Channels;

        // If necessary, adjust the frames to not go past the end
        if (packet.FrameCount + undecodedFrames >= FrameCount)
          packet.FrameCount = FrameCount - undecodedFrames;

        // Copy the samples into the buffer
        memcpy(Samples + UndecodedSamplesIndex, packet.Samples, sizeof(float) * packet.FrameCount * Channels);

        // Advance the undecoded index
        UndecodedSamplesIndex += packet.FrameCount * Channels;

        // If the index hasn't reached the end, decode another packet
        if (UndecodedSamplesIndex < FrameCount * Channels)
          Decoder->AddDecodingTask();
        // If this is the end, don't need the decoder any more (it won't be processing tasks)
        else
        {
          delete Decoder;
          Decoder = nullptr;
        }
      }
      else
      {
        // Copy the samples into the NextStreamedSamples buffer
        memcpy(NextStreamedSamples, packet.Samples, sizeof(float) * packet.FrameCount * Channels);

        // If the index hasn't reached the end, decode another packet
        if (PreviousBufferSamples + (FileEncoder::PacketFrames * Channels) < FrameCount * Channels)
          Decoder->AddDecodingTask();

        // Mark that the second buffer is filled
        NeedSecondBuffer = false;
      }

      // Delete the buffer on the DecodedPacket object
      packet.ReleaseSamples();
    }
  }

  //--------------------------------------------------------------------- Generated Wave Sound Asset

  //************************************************************************************************
  GeneratedWaveSoundAsset::GeneratedWaveSoundAsset(const OscillatorTypes waveType, const float frequency, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAssetNode(extInt, isThreaded),
    Frequency(frequency), 
    WaveData(nullptr),
    FrequencyInterpolator(nullptr)
  {
    if (!Threaded)
      ThreadedAsset = new GeneratedWaveSoundAsset(waveType, frequency, extInt, true);
    else
    {
      WaveData = new Oscillator();
      WaveData->SetType((Oscillator::Types)waveType);
      WaveData->SetFrequency(frequency);
      WaveData->SetNoteOn(true);

      FrequencyInterpolator = gAudioSystem->GetInterpolatorThreaded();
    }
  }

  //************************************************************************************************
  GeneratedWaveSoundAsset::~GeneratedWaveSoundAsset()
  {
    if (Threaded)
    {
      delete WaveData;
      gAudioSystem->ReleaseInterpolatorThreaded(FrequencyInterpolator);
    }
  }

  //************************************************************************************************
  FrameData GeneratedWaveSoundAsset::GetFrame(const unsigned frameIndex)
  {
    if (!Threaded)
      return FrameData();

    FrameData data;
    data.HowManyChannels = 1;

    data.Samples[0] = WaveData->GetNextSample() * WAVE_VOLUME;
    
    if (!FrequencyInterpolator->Finished())
    {
      Frequency = FrequencyInterpolator->NextValue();
      WaveData->SetFrequency(Frequency);
    }

    return data;
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::GetBuffer(float* buffer, const unsigned frameIndex, 
    const unsigned numberOfSamples)
  {
    if (!Threaded)
      return;

    for (unsigned i = 0; i < numberOfSamples; ++i)
      buffer[i] = GetFrame(i).Samples[0];
  }

  //************************************************************************************************
  unsigned GeneratedWaveSoundAsset::GetNumberOfFrames()
  {
    return AudioSystemInternal::SystemSampleRate;
  }

  //************************************************************************************************
  bool GeneratedWaveSoundAsset::GetStreaming()
  {
    return false;
  }

  //************************************************************************************************
  float GeneratedWaveSoundAsset::GetFrequency()
  {
    return Frequency;
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::SetFrequency(const float newFrequency, const float time)
  {
    if (!Threaded)
    {
      Frequency = newFrequency;
      if (ThreadedAsset)
        gAudioSystem->AddTask(Zero::CreateFunctor(&GeneratedWaveSoundAsset::SetFrequency,
          (GeneratedWaveSoundAsset*)ThreadedAsset, newFrequency, time));
    }
    else
    {
      if (time == 0)
      {
        Frequency = newFrequency;
        WaveData->SetFrequency(Frequency);
      }
      else
        FrequencyInterpolator->SetValues(Frequency, newFrequency,
          (unsigned)(time * AudioSystemInternal::SystemSampleRate));
    }
  }

}