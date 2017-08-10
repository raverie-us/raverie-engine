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
    NeedSecondBuffer(true)
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

      Decoder = new FileDecoder(status, fileName, streaming, this);

      if (status.Failed())
      {
        // TODO failed state
      }

      FileLength = (float)Decoder->SamplesPerChannel / AudioSystemInternal::SampleRate;
      Channels = Decoder->Channels;
      FrameCount = Decoder->SamplesPerChannel;

      if (!Streaming)
      {
        Samples = new float[FrameCount * Channels];

        // TODO Need to not do this on the game thread!!
        gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, Decoder));
      }
      else
      {
        Samples = new float[FrameSize * Channels];
        memset(Samples, 0, sizeof(float) * FrameSize * Channels);

        NextStreamedSamples = new float[FrameSize * Channels];
      }
    }
  }

  //************************************************************************************************
  SoundAssetFromFile::~SoundAssetFromFile()
  {
    if (!Threaded)
    {
      if (Samples)
        delete[] Samples;

      if (Decoder)
      {
        AtomicSetPointer((void**)Decoder->Asset, (void*)nullptr);

        if (AtomicCheckEqualityPointer(Decoder->Decoding, nullptr))
          delete Decoder;
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
      if (sampleIndex > UndecodedSamplesIndex || UndecodedSamplesIndex - sampleIndex < AudioSystemInternal::SampleRate * Channels / 2)
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
      sampleIndex -= PreviousBufferSamples;

      if (sampleIndex >= FrameSize * Channels)
      {
        memcpy(Samples, NextStreamedSamples, sizeof(float) * FrameSize * Channels);

        PreviousBufferSamples += FrameSize * Channels;
        sampleIndex -= FrameSize * Channels;

        NeedSecondBuffer = true;

        if (sampleIndex >= FrameSize * Channels)
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

    ErrorIf(data.Samples[0] < -1.0f || data.Samples[0] > 1.0f);

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
      memset(buffer, 0, numberOfSamples * sizeof(float));
      return;
    }

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * Channels;

    if (sampleIndex > UndecodedSamplesIndex || UndecodedSamplesIndex - sampleIndex < AudioSystemInternal::SampleRate * Channels / 2)
      CheckForDecodedPacket();

    unsigned samples = numberOfSamples;
    if (sampleIndex + numberOfSamples >= UndecodedSamplesIndex)
    {
      if (sampleIndex < UndecodedSamplesIndex)
        samples = UndecodedSamplesIndex - sampleIndex;
      else
        samples = 0;
    }

    memcpy(buffer, Samples + sampleIndex, samples * sizeof(float));

    if (samples < numberOfSamples)
      memset(buffer + samples, 0, (numberOfSamples - samples) * sizeof(float));

    for (unsigned i = 0; i < numberOfSamples; ++i)
    {
      ErrorIf(buffer[i] < -1.0f || buffer[i] > 1.0f);
    }
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetNumberOfSamples()
  {
    return FrameCount * Channels;
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

    Decoder->ResetStream();

    PreviousBufferSamples = 0;
    NeedSecondBuffer = true;
    memset(Samples, 0, sizeof(float) * FrameSize * Channels);
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

        // TODO shouldn't do this on game thread
        ((SoundAssetFromFile*)ThreadedAsset)->Decoder->OpenStream();
        gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, ((SoundAssetFromFile*)ThreadedAsset)->Decoder));
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
      gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::CloseStream,
        ((SoundAssetFromFile*)ThreadedAsset)->Decoder));

      ((SoundAssetFromFile*)ThreadedAsset)->PreviousBufferSamples = 0;
      ((SoundAssetFromFile*)ThreadedAsset)->NeedSecondBuffer = true;
      memset(((SoundAssetFromFile*)ThreadedAsset)->Samples, 0, sizeof(float) * FrameSize * Channels);
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

    DecodedPacket* packet;
    if (Decoder->DecodedPacketQueue.Read(packet))
    {
      if (!Streaming)
      {
        unsigned undecodedFrames = UndecodedSamplesIndex / Channels;

        if (packet->FrameCount + undecodedFrames >= FrameCount)
          packet->FrameCount = FrameCount - undecodedFrames;

        memcpy(Samples + UndecodedSamplesIndex, packet->Samples, sizeof(float) * packet->FrameCount * Channels);

        for (unsigned i = 0; i < packet->FrameCount * Channels; ++i)
        {
          ErrorIf(Samples[UndecodedSamplesIndex + i] < -1.0f || Samples[UndecodedSamplesIndex + i] > 1.0f);
        }

        UndecodedSamplesIndex += packet->FrameCount * Channels;

        if (UndecodedSamplesIndex < FrameCount * Channels)
        {
          gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, Decoder));
        }
      }
      else
      {
        memcpy(NextStreamedSamples, packet->Samples, sizeof(float) * packet->FrameCount * Channels);

        if (PreviousBufferSamples + (FrameSize * Channels) < FrameCount * Channels)
          gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, Decoder));

        NeedSecondBuffer = false;
      }

      delete packet;
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
  unsigned GeneratedWaveSoundAsset::GetNumberOfSamples()
  {
    return AudioSystemInternal::SampleRate;
  }
  //************************************************************************************************

  unsigned GeneratedWaveSoundAsset::GetNumberOfFrames()
  {
    return AudioSystemInternal::SampleRate;
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
          (unsigned)(time * AudioSystemInternal::SampleRate));
    }
  }

}