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
  SoundAsset::SoundAsset(ExternalNodeInterface* externalInterface, const bool threaded) :
    ThreadedAsset(nullptr),
    mReferenceCount(0), 
    Threaded(threaded), 
    ExternalData(externalInterface)
  {
    // If not threaded, add to the system's list
    if (!Threaded)
      gAudioSystem->AddAsset(this);
  }

  //************************************************************************************************
  SoundAsset::~SoundAsset()
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
  void SoundAsset::SetExternalInterface(ExternalNodeInterface* extInt)
  {
    if (Threaded)
      return;

    ExternalData = extInt;

    // If interface is now null and is not referenced, can delete now
    if (!extInt && mReferenceCount == 0)
      delete this;
  }

  //************************************************************************************************
  bool SoundAsset::IsPlaying()
  {
    if (Threaded)
      return false;

    return mReferenceCount > 0;
  }

  //************************************************************************************************
  bool SoundAsset::AddReference()
  {
    if (Threaded)
      return false;

    // Check if it's okay to add another instance to this asset
    if (OkayToAddInstance())
    {
      ++mReferenceCount;
      return true;
    }
    else
      return false;
  }

  //************************************************************************************************
  void SoundAsset::ReleaseReference()
  {
    if (Threaded)
      return;

    if (mReferenceCount == 0)
      Error("Trying to release reference on an unreferenced sound asset");

    --mReferenceCount;
    RemoveInstance();

    if (mReferenceCount == 0)
    {
      // If there is no external interface, delete now
      if (!ExternalData)
        delete this;
      else
        ExternalData->SendAudioEvent(AudioEventTypes::AssetUnreferenced, (void*)nullptr);
    }
  }

  //-------------------------------------------------------------------------- Sound Asset From File

  //************************************************************************************************
  SoundAssetFromFile::SoundAssetFromFile(Zero::Status& status, const Zero::String& fileName, 
    const bool streaming, ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAsset(extInt, isThreaded), 
    mHasStreamingInstance(false), 
    mStreaming(streaming),
    mFileLength(0),
    mChannels(0),
    mFrameCount(0),
    Decoder(nullptr),
    mPreviousBufferSamples(0),
    mNeedSecondBuffer(true)
  {
    if (!Threaded)
    {
      ThreadedAsset = new SoundAssetFromFile(status, fileName, streaming, extInt, true);
      if (!status.Failed())
      {
        mFileLength = ((SoundAssetFromFile*)ThreadedAsset)->mFileLength;
        mChannels = ((SoundAssetFromFile*)ThreadedAsset)->mChannels;
        mFrameCount = ((SoundAssetFromFile*)ThreadedAsset)->mFrameCount;
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
        mFileLength = (float)Decoder->SamplesPerChannel / AudioSystemInternal::SystemSampleRate;
        mChannels = Decoder->Channels;
        mFrameCount = Decoder->SamplesPerChannel;

        // If not streaming, make the Samples buffer big enough to hold all the audio samples,
        // and decode one buffer so it's ready to go
        if (!mStreaming)
        {
          Samples.Reserve(mFrameCount * mChannels);

          gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::AddDecodingTask, Decoder));
        }
        // Otherwise, make the Samples buffer the size of one packet
        else
        {
          Samples.Resize(FileEncoder::PacketFrames * mChannels, 0.0f);
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
    if (Threaded && Decoder)
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

  //************************************************************************************************
  void SoundAssetFromFile::AppendSamples(Zero::Array<float>* buffer, const unsigned frameIndex,
    unsigned numberOfSamples)
  {
    if (!Threaded)
      return;

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * mChannels;

    // Handle samples when not streaming
    if (!mStreaming)
    {
      if (Decoder)
        CheckForDecodedPacket();
      
      // Check if the number of samples would go past the available decoded samples
      if (sampleIndex + numberOfSamples >= Samples.Size())
      {
        // Copy available samples, if any
        if (sampleIndex < Samples.Size())
          buffer->Append(Samples.SubRange(sampleIndex, Samples.Size() - sampleIndex));
        // Set the remaining samples to zero
        buffer->Resize(buffer->Size() + numberOfSamples, 0.0f);
      }
      // Otherwise copy all samples
      else
        buffer->Append(Samples.SubRange(sampleIndex, numberOfSamples));
    }
    // Handle samples when streaming
    else
    {
      // Adjust the sample index to be within the current buffer
      sampleIndex -= mPreviousBufferSamples;
      // Save the buffer size
      unsigned bufferSize = FileEncoder::PacketFrames * mChannels;
      // Save the samples available in the current buffer
      int samplesAvailable = bufferSize - sampleIndex;

      // If there are more samples available than are needed, copy them over and return
      if ((int)numberOfSamples <= samplesAvailable)
      {
        buffer->Append(Samples.SubRange(sampleIndex, numberOfSamples));
        return;
      }

      // If there are samples available in the current buffer, copy them 
      if (samplesAvailable > 0)
      {
        buffer->Append(Samples.SubRange(sampleIndex, samplesAvailable));

        // Move the sample index forward
        sampleIndex += samplesAvailable;
        // Reduce the number of samples needed
        numberOfSamples -= samplesAvailable;
      }

      // If there are no samples in the next buffer, check for a decoded packet
      if (NextStreamedSamples.Size() == 0)
      {
        mNeedSecondBuffer = true;
        CheckForDecodedPacket();

        // If there are still no samples, set the rest of the buffer to zero and return
        if (NextStreamedSamples.Size() == 0)
        {
          buffer->Resize(buffer->Size() + numberOfSamples, 0.0f);
          return;
        }
      }

      // Move the next buffer of samples into the Samples array
      Samples.Swap(NextStreamedSamples);
      // Clear the next buffer
      NextStreamedSamples.Clear();
      // Move the previous buffer samples counter forward
      mPreviousBufferSamples += bufferSize;
      // Adjust the sample index
      sampleIndex -= bufferSize;
      // Mark that we need another buffer and check for decoded packets
      mNeedSecondBuffer = true;
      CheckForDecodedPacket();

      // If the sample index is not within this buffer, set the rest of the buffer to zero
      if (sampleIndex > bufferSize)
      {
        buffer->Resize(buffer->Size() + numberOfSamples, 0.0f);
      }
      // Check if there are additional samples needed
      else if (numberOfSamples > 0)
      {
        // Save how many samples are available now
        samplesAvailable = bufferSize - sampleIndex;

        // If there are more samples available than are needed, simply copy them over
        if ((int)numberOfSamples <= samplesAvailable)
          buffer->Append(Samples.SubRange(sampleIndex, numberOfSamples));
        else
        {
          // If there are samples available in the current buffer, copy them 
          if (samplesAvailable > 0)
            buffer->Append(Samples.SubRange(sampleIndex, samplesAvailable));

          // Set the remaining samples to zero
          buffer->Resize(buffer->Size() + numberOfSamples - samplesAvailable, 0.0f);
        }
      }
    }
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetNumberOfFrames()
  {
    return mFrameCount;
  }

  //************************************************************************************************
  bool SoundAssetFromFile::GetStreaming()
  {
    return mStreaming;
  }

  //************************************************************************************************
  void SoundAssetFromFile::ResetStreamingFile()
  {
    if (!Threaded)
      return;

    // Clear out any existing decoded packets
    DecodedPacket packet;
    while (Decoder->DecodedPacketQueue.Read(packet))
    {
      packet.ReleaseSamples();
    }

    // Reset the decoder
    Decoder->ResetStream();

    // Set variables
    mPreviousBufferSamples = 0;
    mNeedSecondBuffer = true;

    // Reset the buffers
    memset(Samples.Data(), 0, sizeof(float) * FileEncoder::PacketFrames * mChannels);
    NextStreamedSamples.Clear();
  }

  //************************************************************************************************
  float SoundAssetFromFile::GetLengthOfFile()
  {
    return mFileLength;
  }

  //************************************************************************************************
  unsigned SoundAssetFromFile::GetChannels()
  {
    return mChannels;
  }

  //************************************************************************************************
  bool SoundAssetFromFile::OkayToAddInstance()
  {
    if (Threaded)
      return false;

    // Not streaming, can play multiple instances
    if (!mStreaming)
      return true;
    else
    {
      // Already streaming one instance, don't play another
      if (mHasStreamingInstance)
        return false;
      else
      {
        mHasStreamingInstance = true;

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

    if (mHasStreamingInstance)
    {
      mHasStreamingInstance = false;

      SoundAssetFromFile* threadedSibling = (SoundAssetFromFile*)ThreadedAsset;

      gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::CloseStream, threadedSibling->Decoder));
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundAssetFromFile::ResetStreamingFile, threadedSibling));
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::CheckForDecodedPacket()
  {
    if (!Threaded || !Decoder)
      return;

    if (mStreaming && !mNeedSecondBuffer)
      return;

    DecodedPacket packet;
    if (Decoder->DecodedPacketQueue.Read(packet))
    {
      unsigned sampleCount = packet.FrameCount * mChannels;

      if (!mStreaming)
      {
        unsigned originalSize = Samples.Size();

        Samples.Resize(originalSize + sampleCount);
        memcpy(Samples.Data() + originalSize, packet.Samples, sampleCount * sizeof(float));

        packet.ReleaseSamples();

        // If we haven't reached the end, decode another packet
        if (Samples.Size() < mFrameCount * mChannels)
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
        // Move the samples into the NextStreamedSamples buffer
        NextStreamedSamples.Resize(sampleCount);
        memcpy(NextStreamedSamples.Data(), packet.Samples, sampleCount * sizeof(float));

        packet.ReleaseSamples();

        // If the index hasn't reached the end, decode another packet
        if (mPreviousBufferSamples + NextStreamedSamples.Size() < mFrameCount * mChannels)
          Decoder->AddDecodingTask();

        // Mark that the second buffer is filled
        mNeedSecondBuffer = false;
      }
    }
  }

  //--------------------------------------------------------------------- Generated Wave Sound Asset

  //************************************************************************************************
  GeneratedWaveSoundAsset::GeneratedWaveSoundAsset(const OscillatorTypes::Enum waveType, 
      const float frequency, ExternalNodeInterface* extInt, const bool isThreaded) :
    SoundAsset(extInt, isThreaded),
    mFrequency(frequency), 
    WaveData(nullptr)
  {
    if (!Threaded)
      ThreadedAsset = new GeneratedWaveSoundAsset(waveType, frequency, extInt, true);
    else
    {
      WaveData = new Oscillator();
      WaveData->SetType(waveType);
      WaveData->SetFrequency(frequency);
      WaveData->SetNoteOn(true);
    }
  }

  //************************************************************************************************
  GeneratedWaveSoundAsset::~GeneratedWaveSoundAsset()
  {
    if (Threaded)
    {
      delete WaveData;
    }
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::AppendSamples(Zero::Array<float>* buffer, const unsigned frameIndex,
    unsigned numberOfSamples)
  {
    if (!Threaded)
      return;

    unsigned startingIndex = buffer->Size();
    buffer->Resize(startingIndex + numberOfSamples);

    for (unsigned i = startingIndex; i < startingIndex + numberOfSamples; ++i)
    {
      (*buffer)[i] = WaveData->GetNextSample() * GeneratedWaveVolume;

      if (!FrequencyInterpolator.Finished())
      {
        mFrequency = FrequencyInterpolator.NextValue();
        WaveData->SetFrequency(mFrequency);
      }
    }
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
    return mFrequency;
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::SetFrequency(const float newFrequency, const float time)
  {
    if (!Threaded)
    {
      mFrequency = newFrequency;
      if (ThreadedAsset)
        gAudioSystem->AddTask(Zero::CreateFunctor(&GeneratedWaveSoundAsset::SetFrequency,
          (GeneratedWaveSoundAsset*)ThreadedAsset, newFrequency, time));
    }
    else
    {
      if (time == 0)
      {
        mFrequency = newFrequency;
        WaveData->SetFrequency(mFrequency);
      }
      else
        FrequencyInterpolator.SetValues(mFrequency, newFrequency,
          (unsigned)(time * AudioSystemInternal::SystemSampleRate));
    }
  }

  //************************************************************************************************
  void GeneratedWaveSoundAsset::SetSquareWavePositiveFraction(float positiveFraction)
  {
    if (!Threaded)
    {
      if (ThreadedAsset)
        gAudioSystem->AddTask(Zero::CreateFunctor(&GeneratedWaveSoundAsset::SetSquareWavePositiveFraction,
          (GeneratedWaveSoundAsset*)ThreadedAsset, positiveFraction));
    }
    else
    {
      WaveData->SetSquareWavePositiveFraction(positiveFraction);
    }
  }

}
