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
  //------------------------------------------------------------------------------------ Sound Asset

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
        // and start decoding the audio data
        if (!mStreaming)
        {
          Samples.Reserve(mFrameCount * mChannels);

          gAudioSystem->AddTask(Zero::CreateFunctor(&FileDecoder::AddDecodingTask, Decoder));
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
      if (Zero::AtomicCompareExchangeBool(&Decoder->DecodingTaskCount, 0, 0))
        delete Decoder;
      else
      {
        // Otherwise set the Asset pointer to null so it will delete itself
        Zero::AtomicStore((void**)Decoder->ParentAlive, (void*)nullptr);
      }
    }
  }

  //************************************************************************************************
  void SoundAssetFromFile::AppendSamples(BufferType* buffer, const unsigned frameIndex,
    unsigned numberOfSamples)
  {
    if (!Threaded)
      return;

    // Translate from frames to sample location
    unsigned sampleIndex = frameIndex * mChannels;

    // Keep the original size of the buffer
    unsigned originalBufferSize = buffer->Size();
    // Resize the buffer to hold the new samples
    buffer->Resize(originalBufferSize + numberOfSamples);
    float* bufferStart = buffer->Data() + originalBufferSize;

    // Handle samples when not streaming
    if (!mStreaming)
    {
      if (Decoder)
        ProcessAvailableDecodedPacket();
      
      // Check if the number of samples would go past the available decoded samples
      if (sampleIndex + numberOfSamples >= Samples.Size())
      {
        // Keep getting decoded packets while it's successful
        while (ProcessAvailableDecodedPacket())
        {
          // Stop when we have enough decoded samples
          if (sampleIndex + numberOfSamples < Samples.Size())
            break;
        }

        int samplesToCopy = Samples.Size() - sampleIndex;

        // Check if there are samples available to copy
        if (samplesToCopy > 0)
        {
          // Copy the samples into the buffer
          memcpy(bufferStart, Samples.Data() + sampleIndex, sizeof(float) * samplesToCopy);
          // Set the remaining samples (if any) to zero
          memset(bufferStart + samplesToCopy, 0, sizeof(float) * (numberOfSamples - samplesToCopy));
        }
        // Otherwise, set all samples to zero
        else
          memset(bufferStart + samplesToCopy, 0, sizeof(float) * numberOfSamples);
      }
      // Doesn't go past end of decoded data, so copy all samples
      else
        memcpy(bufferStart, Samples.Data() + sampleIndex, sizeof(float) * numberOfSamples);
    }
    // Handle samples when streaming
    else
    {
      // Adjust the sample index to be within the current buffer
      sampleIndex -= mPreviousBufferSamples;

      // Get streamed data for the buffer as long as there are still samples to get
      while (numberOfSamples > 0)
        FillStreamingBuffer(&bufferStart, &sampleIndex, &numberOfSamples);
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

    }

    // Reset the decoder
    Decoder->ResetStream();

    // Set variables
    mPreviousBufferSamples = 0;
    mNeedSecondBuffer = true;

    // Reset the buffers
    Samples.Clear();
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
  bool SoundAssetFromFile::ProcessAvailableDecodedPacket()
  {
    if (!Threaded || !Decoder)
      return false;

    if (mStreaming && !mNeedSecondBuffer)
      return false;

    DecodedPacket packet;
    // Check if there is an available packet on the queue
    if (Decoder->DecodedPacketQueue.Read(packet))
    {
      if (!mStreaming)
      {
        // Copy the decoded samples into the asset's array
        CopyIntoBuffer(&Samples, packet.Samples, 0, packet.Samples.Size());

        // If this is the end, don't need the decoder any more (it won't be processing tasks)
        if (Samples.Size() >= mFrameCount * mChannels)
        {
          delete Decoder;
          Decoder = nullptr;
        }
      }
      else
      {
        // Move the samples into the NextStreamedSamples buffer
        NextStreamedSamples = Zero::MoveReference<BufferType>(packet.Samples);

        // If the index hasn't reached the end, decode another packet
        if (mPreviousBufferSamples + NextStreamedSamples.Size() < mFrameCount * mChannels)
          Decoder->AddDecodingTask();

        // Mark that the second buffer is filled
        mNeedSecondBuffer = false;
      }

      return true;
    }

    return false;
  }

  //************************************************************************************************
  bool SoundAssetFromFile::MoveBuffers()
  {
    // If there are no samples in the next buffer, check for a decoded packet
    if (NextStreamedSamples.Size() == 0)
    {
      mNeedSecondBuffer = true;
      ProcessAvailableDecodedPacket();

      // If there are still no samples, return
      if (NextStreamedSamples.Size() == 0)
        return false;
    }

    // Move the previous buffer samples counter forward
    mPreviousBufferSamples += Samples.Size();
    // Move the next buffer of samples into the Samples array
    Samples = Zero::MoveReference<BufferType>(NextStreamedSamples);
    // Mark that we need another buffer and check for decoded packets
    mNeedSecondBuffer = true;
    ProcessAvailableDecodedPacket();

    return true;
  }

  //************************************************************************************************
  void SoundAssetFromFile::FillStreamingBuffer(float** bufferPtr, unsigned* sampleIndexPtr,
    unsigned* samplesNeededPtr)
  {
    if (*samplesNeededPtr == 0)
      return;

    // Save the buffer size
    unsigned bufferSize = Samples.Size();

    // Save references for ease of use
    unsigned& sampleIndex = *sampleIndexPtr;
    unsigned& samplesNeeded = *samplesNeededPtr;

    // Keep looking for decoded samples while the index is larger than the current buffer
    while (sampleIndex >= bufferSize)
    {
      // If there are no more decoded samples available, set the buffer to zero and return
      if (!MoveBuffers())
      {
        memset(*bufferPtr, 0, sizeof(float) * samplesNeeded);
        samplesNeeded = 0;
        return;
      }

      // Adjust the sample index
      sampleIndex -= bufferSize;
      // Get the new buffer size
      bufferSize = Samples.Size();
    }

    // Copy either the samples needed or the samples available, whichever is smaller
    unsigned samplesCopied = Math::Min(samplesNeeded, bufferSize - sampleIndex);
    memcpy(*bufferPtr, Samples.Data() + sampleIndex, sizeof(float) * samplesCopied);

    // Move the sample index forward
    sampleIndex += samplesCopied;
    // Reduce the number of samples needed
    samplesNeeded -= samplesCopied;
    // Move the buffer pointer forward
    *bufferPtr = *bufferPtr + samplesCopied;
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
  void GeneratedWaveSoundAsset::AppendSamples(BufferType* buffer, const unsigned frameIndex,
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
