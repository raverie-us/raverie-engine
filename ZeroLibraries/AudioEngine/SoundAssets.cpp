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

  struct FileHeader
  {
    const char Name[4] = { 'Z','E','R','O' };
    short Channels;
    unsigned SamplesPerChannel;
  };

  struct PacketHeader
  {
    const char Name[4] = { 'p','a','c','k' };
    short Channel;
    unsigned Size;
  };

  struct DecodedPacket
  {
    unsigned FrameCount;
    float* Samples;
  };

  //************************************************************************************************
  SoundAssetFromFile::SoundAssetFromFile(Zero::Status& status, const Zero::String& fileName, 
    const bool streaming, ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAssetNode(extInt, isThreaded), 
    HasStreamingInstance(false), 
    Streaming(streaming),
    FileLength(0),
    Channels(0),
    //FileData(nullptr),
    //SampleRate(0),
    FrameCount(0),
    //Type(Other_Type),
    DecodingCheck(this),
    UndecodedIndex(0),
    Samples(nullptr)
  {
    if (!Threaded)
    {
      ThreadedAsset = new SoundAssetFromFile(status, fileName, streaming, extInt, true);
      if (!status.Failed())
      {
        FileLength = ((SoundAssetFromFile*)ThreadedAsset)->FileLength;
        Channels = ((SoundAssetFromFile*)ThreadedAsset)->Channels;
        //SampleRate = ((SoundAssetFromFile*)ThreadedAsset)->SampleRate;
        FrameCount = ((SoundAssetFromFile*)ThreadedAsset)->FrameCount;
        //Type = ((SoundAssetFromFile*)ThreadedAsset)->Type;
      }
    }
    else
    {
//       if (!streaming)
//         FileData = FileAccess::LoadAudioFile(status, fileName);
//       else
//         FileData = FileAccess::StartStream(status, fileName);
// 
//       if (status.Failed())
//         return;
// 
//       FileLength = (float)(FileData->DecodingData->TotalSamples / FileData->DecodingData->Channels)
//         / FileData->DecodingData->SampleRate;
//       Channels = FileData->DecodingData->Channels;
//       SampleRate = FileData->DecodingData->SampleRate;
//       FrameCount = FileData->DecodingData->FrameCount;
//       Type = FileData->GetFileType();


      // Remember that this constructor happens on the game thread

      // TODO read from buffer as well as from file

      InputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
      if (!InputFile.IsOpen())
      {
        status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
        return;
      }

      FileHeader header;
      InputFile.Read(status, (byte*)&header, sizeof(header));
      if (status.Failed())
        return;

      ErrorIf(header.Name[0] != 'Z');

      int error;
      for (short i = 0; i < header.Channels; ++i)
      {
        Decoders[i] = opus_decoder_create(48000, 1, &error);
        if (error < 0)
        {
          status.SetFailed(Zero::String::Format("Error creating audio decoder: %s", opus_strerror(error)));
        }
      }

      FileLength = (float)header.SamplesPerChannel / gAudioSystem->SampleRate;
      Channels = header.Channels;
      FrameCount = header.SamplesPerChannel;

      Samples = new float[FrameCount * Channels];

      // TODO Need to not do this on the game thread!!
      gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&SoundAssetFromFile::DecodeNextPacket, this));
    }
  }

  //************************************************************************************************
  SoundAssetFromFile::~SoundAssetFromFile()
  {
//     if (FileData)
//       delete FileData;


    if (Samples)
      delete[] Samples;

    // TODO Need to destroy encoders when done with them
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

//     for (unsigned i = 0; i < data.HowManyChannels; ++i)
//       data.Samples[i] = Samples[sampleIndex + i];

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

//     for (unsigned i = 0; i < numberOfSamples && sampleIndex <= FileData->DecodingData->TotalSamples; 
//       ++i, ++sampleIndex)
//     {
//       buffer[i] = (*FileData)[sampleIndex];
//     }
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetSampleRate()
  {
    //return SampleRate;
    return 0;
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
  Audio::AudioFileTypes SoundAssetFromFile::GetFileType()
  {
    return AudioFileTypes::Other_Type;
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

  void SoundAssetFromFile::DecodeNextPacket()
  {
    // Remember that this function happens on the decoding thread

    if (!InputFile.IsOpen())
      return;

    Zero::Status status;

    PacketHeader header;
    int frames;

    for (unsigned i = 0; i < Channels; ++i)
    {
      InputFile.Read(status, (byte*)&header, sizeof(header));
      if (status.Failed())
      {
        if (!Streaming)
        {
          InputFile.Close();
          return;
        }
        else
        {
          InputFile.Seek(sizeof(FileHeader), Zero::FileOrigin::Begin);
          InputFile.Read(status, (byte*)&header, sizeof(header));
          if (status.Failed())
          {
            InputFile.Close();
            return;
          }
        }
      }

      ErrorIf(header.Name[0] != 'p' || header.Name[1] != 'a');
      ErrorIf(header.Size > MaxPacketSize);
      ErrorIf(header.Channel != i);

      InputFile.Read(status, PacketBuffer, header.Size);

      frames = opus_decode_float(Decoders[header.Channel], PacketBuffer, header.Size, DecodedPackets[i], FrameSize, 0);

      ErrorIf(frames < 0, Zero::String::Format("Opus error: %s", opus_strerror(frames)).c_str());
    }

    DecodedPacket* newPacket = new DecodedPacket();
    newPacket->FrameCount = frames;
    newPacket->Samples = new float[newPacket->FrameCount * Channels];

    for (unsigned frame = 0; frame < newPacket->FrameCount; ++frame)
    {
      for (unsigned channel = 0; channel < Channels; ++channel)
        newPacket->Samples[(frame * channel) + channel] = DecodedPackets[channel][frame];
    }

    DecodedPacketQueue.Write(newPacket);

    if (AtomicCheckEqualityPointer(DecodingCheck, nullptr))
      delete this;
  }

  void SoundAssetFromFile::CheckForDecodedPacket()
  {
    if (AtomicCheckEqualityPointer(DecodingCheck, this))
    {
      DecodedPacket* packet;
      if (DecodedPacketQueue.Read(packet))
      {
        // TODO Need to handle streaming

        memcpy(Samples + UndecodedIndex, packet->Samples, sizeof(float) * packet->FrameCount * Channels);
        UndecodedIndex += packet->FrameCount * Channels;

        if (UndecodedIndex < FrameCount * Channels)
        {
          gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&SoundAssetFromFile::DecodeNextPacket, this));
        }
        else
          AtomicSetPointer((void**)&DecodingCheck, nullptr);
      }
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
  unsigned GeneratedWaveSoundAsset::GetSampleRate()
  {
    return gAudioSystem->SystemSampleRate;
  }

  //************************************************************************************************
  unsigned GeneratedWaveSoundAsset::GetNumberOfSamples()
  {
    return gAudioSystem->SystemSampleRate;
  }
  //************************************************************************************************

  unsigned GeneratedWaveSoundAsset::GetNumberOfFrames()
  {
    return gAudioSystem->SystemSampleRate;
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
          (unsigned)(time * gAudioSystem->SystemSampleRate));
    }
  }

}