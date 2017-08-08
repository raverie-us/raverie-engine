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
    if (!Threaded)
    {
      ExternalData = extInt;

      // If interface is now null and is not referenced, can delete now
      if (!extInt && ReferenceCount == 0)
        delete this;
    }
  }

  //************************************************************************************************
  bool SoundAssetNode::IsPlaying()
  {
    return ReferenceCount > 0;
  }

  //************************************************************************************************
  bool SoundAssetNode::AddReference()
  {
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
    UndecodedIndex(0),
    Samples(nullptr),
    Decoder(nullptr)
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

      // TODO read from buffer as well as from file

      Decoder = new FileDecoder(status, fileName, streaming, this);

      if (status.Failed())
      {
        // TODO failed state
      }

      FileLength = (float)Decoder->SamplesPerChannel / AudioSystemInternal::SampleRate;
      Channels = Decoder->Channels;
      FrameCount = Decoder->SamplesPerChannel;

      Samples = new float[FrameCount * Channels];
    }
  }

  //************************************************************************************************
  SoundAssetFromFile::~SoundAssetFromFile()
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

  //************************************************************************************************
  FrameData SoundAssetFromFile::GetFrame(const unsigned frameIndex)
  {
    if (!Threaded)
      return FrameData();

    FrameData data;
    data.HowManyChannels = Channels;

    CheckForDecodedPacket();

    // Past end of file, return zeros
    if (frameIndex >= FrameCount || frameIndex >= UndecodedIndex)
    {
      memset(data.Samples, 0, data.HowManyChannels * sizeof(float));

      return data;
    }

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * Channels;

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

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * Channels;

    CheckForDecodedPacket();

    unsigned samples = numberOfSamples;
    if (sampleIndex + numberOfSamples >= UndecodedIndex)
      samples = UndecodedIndex - sampleIndex;

    memcpy(buffer, Samples + sampleIndex, samples * sizeof(float));

    if (samples < numberOfSamples)
      memset(buffer + samples, 0, (numberOfSamples - samples) * sizeof(float));
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
    // TODO
    //FileData->ResetStreamingFile();
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
        // TODO
//         gAudioSystem->AddTask(Zero::CreateFunctor(&SamplesFromFile::ReopenStreamingFile, 
//           ((SoundAssetFromFile*)ThreadedAsset)->FileData));
        return true;
      }
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::RemoveInstance()
  {
    if (HasStreamingInstance)
    {
      HasStreamingInstance = false;
      // TODO
//       gAudioSystem->AddTask(Zero::CreateFunctor(&SamplesFromFile::CloseStreamingFile, 
//         ((SoundAssetFromFile*)ThreadedAsset)->FileData));
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::CheckForDecodedPacket()
  {
    if (!Decoder)
      return;

    DecodedPacket* packet;
    if (Decoder->DecodedPacketQueue.Read(packet))
    {
      // TODO Need to handle streaming

      if ((packet->FrameCount * Channels) + UndecodedIndex >= FrameCount)
        packet->FrameCount = (FrameCount - UndecodedIndex) / Channels;

      memcpy(Samples + UndecodedIndex, packet->Samples, sizeof(float) * packet->FrameCount * Channels);

      UndecodedIndex += packet->FrameCount * Channels;

      if (UndecodedIndex < FrameCount * Channels)
      {
        gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, Decoder));
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
    memset(buffer, 0, sizeof(float) * numberOfSamples);
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