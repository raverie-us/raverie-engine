///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

#define MAXNODES 10000

namespace Audio
{
  AudioSystemInternal *gAudioSystem;

  //-------------------------------------------------------------------------- Audio System Internal

  //************************************************************************************************
  AudioSystemInternal::AudioSystemInternal(ExternalSystemInterface* extInterface) : 
    ExternalInterface(extInterface), 
    MinimumVolumeThresholdThreaded(0.015f),
    ShuttingDownThreaded(false), 
    SystemVolumeThreaded(1.0f),
    SystemChannelsThreaded(2),
    Volume(1.0f), 
    ResetPA(false), 
    MixVersionNumber(0), 
    FinalOutputNode(nullptr), 
    FinalOutputNodeThreaded(nullptr),
    PeakVolumeLastMix(0.0f),
    RmsVolumeLastMix(0.0f),
    NodeCount(0),
    LowPass(nullptr),
    PreviousPeakVolumeThreaded(0),
    PreviousRMSVolumeThreaded(0),
    Resampling(false),
    SendMicrophoneInputData(false),
    Muted(false),
    MutedThreaded(false),
    MutingThreaded(false)
  {
    gAudioSystem = this;

    AudioIO = new AudioIOWindows();
  }

  //************************************************************************************************
  AudioSystemInternal::~AudioSystemInternal()
  {
    delete AudioIO;
  }

  //************************************************************************************************
  Zero::OsInt StartMix(void *system)
  {
    ((Audio::AudioSystemInternal*)system)->MixLoopThreaded();
    return 0;
  }

  //************************************************************************************************
  void AudioSystemInternal::StartSystem(Zero::Status &status)
  {
    // Initialize the audio API
    StreamStatus::Enum ioStatus = AudioIO->InitializeAPI();
    // If initialization was not successful, set the message on the status object
    if (ioStatus != StreamStatus::Initialized)
      status.SetFailed(AudioIO->LastErrorMessage);

    // Initialize audio output
    StreamStatus::Enum outputStatus = AudioIO->InitializeStream(StreamTypes::Output);
    // If initializing stream failed but initializing API succeeded, set the message on the status
    if (outputStatus != StreamStatus::Initialized && status.Succeeded())
      status.SetFailed(AudioIO->GetStreamInfo(StreamTypes::Output).ErrorMessage);

    CheckForResampling();

    // Create output nodes
    FinalOutputNode = new OutputNode("FinalOutputNode", this, false);
    FinalOutputNodeThreaded = (OutputNode*)FinalOutputNode->GetSiblingNode();

    // For low frequency channel (uses audio system in constructor)
    LowPass = new LowPassFilter();
    LowPass->SetCutoffFrequency(120.0f);

    AudioIO->OutputRingBuffer.ResetBuffer();

    // Start up the mix thread
    if (Zero::ThreadingEnabled)
    {
      MixThread.Initialize(StartMix, this, "Audio mix");
      MixThread.Resume();
      if (!MixThread.IsValid())
      {
        ZPrint("Error creating audio mix thread\n");
        status.SetFailed("Error creating audio mix thread");
        return;
      }

      ZPrint("Audio mix thread initialized\n");
    }

    AudioIO->StartStream(StreamTypes::Output);

    ZPrint("Audio initialization completed\n");
  }

  //************************************************************************************************
  void AudioSystemInternal::StopSystem(Zero::Status &status)
  {
    Zero::TimerBlock block("Stopping Audio System.");

    if (MixThread.IsValid())
    {
      // Tells the mix thread to shut down
      AddTask(Zero::CreateFunctor(&AudioSystemInternal::ShutDownThreaded, this));

      // Wait for the mix thread to finish shutting down
      MixThread.WaitForCompletion();
      MixThread.Close();
    }

    // Shut down audio output, input, and API
    AudioIO->ShutDownStream(StreamTypes::Input);
    AudioIO->ShutDownStream(StreamTypes::Output);
    AudioIO->ShutDownAPI();

    while (!TagsToDelete.Empty())
    {
      delete TagsToDelete.Back();
      TagsToDelete.PopBack();
    }
    while (!TagsToDeleteThreaded.Empty())
    {
      delete TagsToDeleteThreaded.Back();
      TagsToDeleteThreaded.PopBack();
    }

    // Delete all existing sound nodes 
    while (!NodeListThreaded.Empty())
      delete &NodeListThreaded.Front();
    while (!NodeList.Empty())
      delete &NodeList.Front();

    // Delete all existing sound assets
    while (!AssetList.Empty())
      delete &AssetList.Front();

    // Delete all current tags
    while (!TagList.Empty())
      delete &TagList.Front();
    while (!TagListThreaded.Empty())
      delete &TagListThreaded.Front();

    // Delete any unprocessed tasks
    Zero::Functor* function;
    while (TasksForGameThread.Read(function))
      delete function;
    while (TasksForMixThread.Read(function))
      delete function;
    
    if (LowPass)
      delete LowPass;
  }

  //************************************************************************************************
  void AudioSystemInternal::MixLoopThreaded()
  {
#ifdef TRACK_TIME 
    double maxTime = (double)AudioIO->OutputBufferSizeThreaded / (double)AudioIO->GetOutputChannels() 
      / (double)SystemSampleRate;
#endif
    
    bool running = true;
    do
    {
#ifdef TRACK_TIME
      clock_t time = clock();
#endif

      // Execute tasks
      HandleTasksThreaded();

      // Mix current sounds to output buffer
      // Will return false when it's okay to shut down
      running = MixCurrentInstancesThreaded();

      // Delete objects on the to-be-deleted list
      forRange(TagObject* object, TagsToDeleteThreaded.All())
        delete object;
      TagsToDeleteThreaded.Clear();

#ifdef TRACK_TIME
      double timeDiff = (double)(clock() - time) / CLOCKS_PER_SEC;
      if (timeDiff > maxTime)
      {
        AddTaskThreaded(Zero::CreateFunctor(&ExternalSystemInterface::SendAudioError, 
          ExternalInterface, Zero::String("Mix took too long (informational message, not an error)")));
      }
#endif

      // Wait until more data is needed
      if (running && Zero::ThreadingEnabled)
        AudioIO->WaitUntilOutputNeededThreaded();

    } while (running && Zero::ThreadingEnabled);

  }

  //************************************************************************************************
  bool AudioSystemInternal::MixCurrentInstancesThreaded()
  {
    if (!FinalOutputNodeThreaded)
    {
      if (!ShuttingDownThreaded)
        return true;
      else
        return false;
    }

    // Find out how many samples we can write to the ring buffer
    unsigned samplesNeeded = AudioIO->OutputRingBuffer.GetWriteAvailable();

    // Check to make sure there is available write space
    if (samplesNeeded == 0)
      return true;

    // Save the number of channels in the audio output
    unsigned outputChannels = AudioIO->GetStreamChannels(StreamTypes::Output);
    // Number of frames in the output
    unsigned outputFrames = samplesNeeded / outputChannels;

    // Check for resampling and update resample factor if necessary
    // (this can change at any time if the audio output device changes)
    CheckForResampling();

    // Number of frames in the mix (will be different if resampling)
    unsigned mixFrames = outputFrames;
    if (Resampling)
      mixFrames = OutputResampling.GetOutputFrameCount(outputFrames);

    // Get the audio input data
    GetAudioInputDataThreaded(samplesNeeded);

    // If sending microphone input to the external system, add the input buffer to the queue
    if (SendMicrophoneInputData)
      InputDataQueue.Write(new Zero::Array<float>(InputBuffer));

    // Resize BufferForOutput to match samplesNeeded
    BufferForOutput.Resize(mixFrames * SystemChannelsThreaded);

    // Get samples from output node
    bool isThereData = FinalOutputNodeThreaded->GetOutputSamples(&BufferForOutput, 
      SystemChannelsThreaded, nullptr, true);
    ++MixVersionNumber;

    // Set the size of the MixedOutput buffer
    MixedOutput.Resize(samplesNeeded);

    float peakVolume(0.0f);
    unsigned rmsVolume(0);

    // If there is no real data, just reset output buffer to 0
    if (!isThereData)
      memset(MixedOutput.Data(), 0, MixedOutput.Size() * sizeof(float));
    else
    {
      // Frame object for this set of samples
      AudioFrame frame;

      if (Resampling)
        OutputResampling.SetInputBuffer(BufferForOutput.Data(), mixFrames, SystemChannelsThreaded);

      // Step through each frame in the output buffer
      for (unsigned frameIndex = 0; frameIndex < outputFrames; ++frameIndex)
      {
        // If not resampling, set the samples on the frame object from this frame in the mix
        if (!Resampling)
          frame.SetSamples(BufferForOutput.Data() + (frameIndex * SystemChannelsThreaded), 
            SystemChannelsThreaded);
        // Otherwise, interpolate between two mix frames
        else
        {
          Zero::Array<float> samples(SystemChannelsThreaded);
          OutputResampling.GetNextFrame(samples.Data());
          frame.SetSamples(samples.Data(), SystemChannelsThreaded);
        }

        // Apply the system volume
        frame *= SystemVolumeThreaded;
        // Keep the samples between -1.0 and 1.0
        frame.Clamp();

        // If the peak volume of this frame is the loudest so far, save it
        float framePeak = frame.GetMaxValue();
        if (framePeak > peakVolume)
          peakVolume = framePeak;

        // Use 16 bit int for RMS volume
        unsigned value = (unsigned)(Math::Abs(frame.GetMonoValue()) * ((1 << 15) - 1));
        rmsVolume += value * value;

        float* frameSamples = frame.GetSamples(outputChannels);

        // If 5.1 or 7.1, handle low frequency channel
        if (outputChannels == 6 || outputChannels == 8)
        {
          float monoSample = frame.GetMonoValue();
          LowPass->ProcessFrame(&monoSample, frameSamples + 3, 1);
        }

        // Copy this frame of samples to the output buffer
        memcpy(MixedOutput.Data() + (frameIndex * outputChannels), frameSamples,
          sizeof(float) * outputChannels);
      }
    }

    // Update the output volumes
    if (peakVolume != PreviousPeakVolumeThreaded || rmsVolume != PreviousRMSVolumeThreaded)
    {
      PreviousPeakVolumeThreaded = peakVolume;
      PreviousRMSVolumeThreaded = rmsVolume;
      rmsVolume /= outputFrames;
      AddTaskThreaded(Zero::CreateFunctor(&AudioSystemInternal::SetVolumes, this, peakVolume, rmsVolume));
    }

    // Check if there is a volume adjustment to apply
    if (!VolumeInterpolatorThreaded.Finished())
    {
      // Apply the interpolated volume to each frame
      BufferRange outputRange = MixedOutput.All();
      while (!outputRange.Empty())
      {
        float volume = VolumeInterpolatorThreaded.NextValue();

        for (unsigned j = 0; j < outputChannels; ++j, outputRange.PopFront())
          outputRange.Front() *= volume;
      }
    }

    // If shutting down, wait for volume to ramp down to zero
    if (ShuttingDownThreaded)
    {
      // Ramp the volume down to zero
      VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, outputFrames);
      BufferRange outputRange = MixedOutput.All();
      while (!outputRange.Empty())
      {
        float volume = VolumeInterpolatorThreaded.NextValue();

        for (unsigned j = 0; j < outputChannels; ++j, outputRange.PopFront())
          outputRange.Front() *= volume;
      }

      // Copy the data to the ring buffer
      AudioIO->OutputRingBuffer.Write(MixedOutput.Data(), MixedOutput.Size());

      return false;
    }

    // If muted, don't actually output any audio data, set everything to zero
    if (MutedThreaded)
      memset(MixedOutput.Data(), 0, sizeof(float) * MixedOutput.Size());

    // Check if we are switching to muted
    if (MutingThreaded && VolumeInterpolatorThreaded.Finished())
    {
      MutingThreaded = false;
      MutedThreaded = true;
    }

    // Copy the data to the ring buffer
    AudioIO->OutputRingBuffer.Write(MixedOutput.Data(), MixedOutput.Size());

    // Still running, return true
    return true;
  }

  //************************************************************************************************
  void AudioSystemInternal::AddTask(Zero::Functor* function)
  {
    TasksForMixThread.Write(function);
    return;
  }

  //************************************************************************************************
  void AudioSystemInternal::AddTaskThreaded(Zero::Functor* function)
  {
    TasksForGameThread.Write(function);
  }

  //************************************************************************************************
  void AudioSystemInternal::AddTag(TagObject* tag, bool threaded)
  {
    if (threaded)
      TagListThreaded.PushBack(tag);
    else
      TagList.PushBack(tag);
  }

  //************************************************************************************************
  void AudioSystemInternal::RemoveTag(TagObject* tag, bool threaded)
  {
    if (threaded)
      TagListThreaded.Erase(tag);
    else
      TagList.Erase(tag);
  }

  //************************************************************************************************
  void AudioSystemInternal::DelayDeleteTag(TagObject* tag, bool threaded)
  {
    if (threaded)
      TagsToDeleteThreaded.PushBack(tag);
    else
      TagsToDelete.PushBack(tag);
  }

  //************************************************************************************************
  void AudioSystemInternal::AddAsset(SoundAsset* asset)
  {
    AssetList.PushBack(asset);
  }

  //************************************************************************************************
  void AudioSystemInternal::RemoveAsset(SoundAsset* asset)
  {
    AssetList.Erase(asset);
  }

  //************************************************************************************************
  bool AudioSystemInternal::AddSoundNode(SoundNode* node, const bool threaded)
  {
    if (!threaded)
    {
      // Need to add the node to the list even if over the max, since it will 
      // remove itself in its destructor
      ++NodeCount;
      NodeList.PushBack(node);

#ifdef _DEBUG  
      if (NodeCount >= MAXNODES)
        ExternalInterface->SendAudioError("Number of SoundNodes over limit");
#endif
    }
    else
    {
      NodeListThreaded.PushBack(node);
    }

    return true;
  }

  //************************************************************************************************
  void AudioSystemInternal::RemoveSoundNode(SoundNode* node, const bool threaded)
  {
    if (!threaded)
    {
      --NodeCount;
      NodeList.Erase(node);
    }
    else
      NodeListThreaded.Erase(node);
  }

  //************************************************************************************************
  void AudioSystemInternal::SetMinVolumeThresholdThreaded(const float volume)
  {
    MinimumVolumeThresholdThreaded = volume;
  }

  //************************************************************************************************
  void AudioSystemInternal::SetMutedThreaded(bool muteAudio)
  {
    if (muteAudio && !MutedThreaded && !MutingThreaded)
    {
      MutingThreaded = true;
      VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, PropertyChangeFrames);
    }
    else if (!muteAudio && MutedThreaded)
    {
      MutedThreaded = false;
      VolumeInterpolatorThreaded.SetValues(0.0f, 1.0f, PropertyChangeFrames);
    }
  }

  //************************************************************************************************
  bool AudioSystemInternal::StartInput()
  {
    if (AudioIO->GetStreamInfo(StreamTypes::Input).Status == StreamStatus::Started)
      return true;
   
    StreamStatus::Enum inputStatus = AudioIO->InitializeStream(StreamTypes::Input);
    if (inputStatus != StreamStatus::Initialized)
      return false;

    inputStatus = AudioIO->StartStream(StreamTypes::Input);
    if (inputStatus == StreamStatus::Started)
      return true;
    else
      return false;
  }

  //************************************************************************************************
  void AudioSystemInternal::HandleTasksThreaded()
  {
    int counter(0);
    Zero::Functor* function;
    while (counter < MaxTasksToRead && TasksForMixThread.Read(function))
    {
      function->Execute();
      delete function;
      ++counter;
    }
  }

  //************************************************************************************************
  void AudioSystemInternal::HandleTasks()
  {
    int counter(0);
    Zero::Functor* function;
    while (counter < MaxTasksToRead && TasksForGameThread.Read(function))
    {
      function->Execute();
      delete function;
      ++counter;
    }
  }

  //************************************************************************************************
  void AudioSystemInternal::ShutDownThreaded()
  {
    ShuttingDownThreaded = true;
  }

  //************************************************************************************************
  void AudioSystemInternal::SetSystemChannelsThreaded(const unsigned channels)
  {
    SystemChannelsThreaded = channels;
  }

  //************************************************************************************************
  void AudioSystemInternal::SetVolumes(const float peak, const unsigned rms)
  {
    PeakVolumeLastMix = peak;
    RmsVolumeLastMix = Math::Sqrt((float)rms) / (float)((1 << 15) - 1);
  }

  //************************************************************************************************
  void AudioSystemInternal::SetLatencyThreaded(const bool useHighLatency)
  {
    if (!useHighLatency)
      AudioIO->SetOutputLatency(LatencyValues::LowLatency);
    else
      AudioIO->SetOutputLatency(LatencyValues::HighLatency);
  }

  //************************************************************************************************
  void AudioSystemInternal::CheckForResampling()
  {
    unsigned outputSampleRate = AudioIO->GetStreamSampleRate(StreamTypes::Output);

    if (SystemSampleRate != outputSampleRate)
    {
      Resampling = true;
      OutputResampling.SetFactor((double)SystemSampleRate / (double)outputSampleRate);
    }
    else if (SystemSampleRate == outputSampleRate)
    {
      Resampling = false;
    }
  }

  //************************************************************************************************
  void AudioSystemInternal::GetAudioInputDataThreaded(unsigned howManySamples)
  {
    unsigned inputChannels = AudioIO->GetStreamChannels(StreamTypes::Input);
    unsigned inputRate = AudioIO->GetStreamSampleRate(StreamTypes::Input);

    if (inputChannels == 0 || inputRate == 0)
      return;

    // Channels and sample rates match, don't need to process anything
    if (inputChannels == SystemChannelsThreaded && inputRate == SystemSampleRate)
    {
      AudioIO->GetInputDataThreaded(InputBuffer, howManySamples);
    }
    else
    {
      // Save the number of frames of input to get, adjusting for resampling if necessary
      unsigned inputFrames = howManySamples / SystemChannelsThreaded;
      if (inputRate != SystemSampleRate)
        inputFrames = (unsigned)(inputFrames * (float)inputRate / (float)SystemSampleRate);

      // Need to adjust channels
      if (inputChannels != SystemChannelsThreaded)
      {
        // Get input data in a temporary array
        Zero::Array<float> inputSamples;
        AudioIO->GetInputDataThreaded(inputSamples, inputFrames * inputChannels);

        // Reset the InputBuffer
        InputBuffer.Clear();

        // Translate the channels for each audio frame and add samples to InputBuffer
        for (unsigned i = 0; i < inputSamples.Size(); i += inputChannels)
        {
          AudioFrame frame(inputSamples.Data() + i, inputChannels);
          float* samples = frame.GetSamples(SystemChannelsThreaded);
          for (unsigned j = 0; j < SystemChannelsThreaded; ++j)
            InputBuffer.PushBack(samples[j]);
        }
      }
      // Not adjusting channels, just get input
      else
        AudioIO->GetInputDataThreaded(InputBuffer, inputFrames * SystemChannelsThreaded);

      // Need to resample
      if (inputRate != SystemSampleRate)
      {
        // Temporary array for resampled data
        Zero::Array<float> resampledInput;
        // Set the resampling factor on the resampler object
        InputResampling.SetFactor((double)inputRate / (double)SystemSampleRate);
        // Set the buffer on the resampler
        InputResampling.SetInputBuffer(InputBuffer.Data(), InputBuffer.Size() 
          / SystemChannelsThreaded, SystemChannelsThreaded);
        // Array to get a frame of samples from the resampler
        Zero::Array<float> frame(SystemChannelsThreaded);

        bool working(true);
        while (working)
        {
          // Get the next frame of resampled data
          working = InputResampling.GetNextFrame(frame.Data());
          // Add it to the array
          resampledInput.Append(frame.All());
        }

        // Swap the resampled data into the InputBuffer
        InputBuffer.Swap(resampledInput);
      }
    }
  }

  //************************************************************************************************
  void AudioSystemInternal::SetSendMicInputData(bool sendData)
  {
    if (sendData && !SendMicrophoneInputData)
    {
      if (!StartInput())
        return;

      AddTask(Zero::CreateFunctor(&AudioSystemInternal::SendMicrophoneInputData,
        this, true));
    }
    else if (!sendData && SendMicrophoneInputData)
    {
      AddTask(Zero::CreateFunctor(&AudioSystemInternal::SendMicrophoneInputData,
        this, false));
    }
  }

  //------------------------------------------------------------------------------------ Audio Frame

  //************************************************************************************************
  AudioFrame::AudioFrame(float* samples, unsigned channels)
  {
    SetSamples(samples, channels);

    Matrices[0] = nullptr;
    Matrices[1] = ChannelMatrix1;
    Matrices[2] = ChannelMatrix2;
    Matrices[3] = ChannelMatrix3;
    Matrices[4] = ChannelMatrix4;
    Matrices[5] = ChannelMatrix5;
    Matrices[6] = ChannelMatrix6;
    Matrices[7] = ChannelMatrix7;
    Matrices[8] = ChannelMatrix8;
  }

  //************************************************************************************************
  AudioFrame::AudioFrame() :
    mStoredChannels(1)
  {
    memset(mSamples, 0, sizeof(float) * MaxChannels);

    Matrices[0] = nullptr;
    Matrices[1] = ChannelMatrix1;
    Matrices[2] = ChannelMatrix2;
    Matrices[3] = ChannelMatrix3;
    Matrices[4] = ChannelMatrix4;
    Matrices[5] = ChannelMatrix5;
    Matrices[6] = ChannelMatrix6;
    Matrices[7] = ChannelMatrix7;
    Matrices[8] = ChannelMatrix8;
  }

  //************************************************************************************************
  AudioFrame::AudioFrame(const AudioFrame& copy) :
    mStoredChannels(copy.mStoredChannels)
  {
    memcpy(mSamples, copy.mSamples, sizeof(float) * MaxChannels);

    Matrices[0] = nullptr;
    Matrices[1] = ChannelMatrix1;
    Matrices[2] = ChannelMatrix2;
    Matrices[3] = ChannelMatrix3;
    Matrices[4] = ChannelMatrix4;
    Matrices[5] = ChannelMatrix5;
    Matrices[6] = ChannelMatrix6;
    Matrices[7] = ChannelMatrix7;
    Matrices[8] = ChannelMatrix8;
  }

  //************************************************************************************************
  float* AudioFrame::GetSamples(const unsigned outputChannels)
  {
    if (outputChannels != mStoredChannels)
    {
      float output[MaxChannels] = { 0 };

      // Down-mixing
      if (outputChannels < mStoredChannels)
      {
        for (unsigned outChannel = 0; outChannel < outputChannels; ++outChannel)
        {
          for (unsigned i = 0; i < MaxChannels; ++i)
          {
            output[outChannel] += mSamples[i] * Matrices[outputChannels][i + (outChannel * MaxChannels)];
          }
        }
      }
      // Up-mixing
      else
      {
        if (mStoredChannels == 1)
        {
          for (unsigned i = 0; i < outputChannels; ++i)
            output[i] = mSamples[0];

          output[LowFreq] = 0.0f;
        }
        else
        {
          if (mStoredChannels < 5)
          {
            output[0] = mSamples[0];
            output[1] = mSamples[1];

            if (mStoredChannels == 3)
              output[Center] = mSamples[Center];
            else
              output[Center] = (mSamples[0] + mSamples[1]) * Sqrt2Inv;

            output[BackLeft] = output[BackRight] = output[SideLeft] = output[SideRight] = (mSamples[0] - mSamples[1]) * Sqrt2Inv;
          }
          else
          {
            memcpy(output, mSamples, sizeof(float) * 6);
            memcpy(output + BackLeft, mSamples + SideLeft, sizeof(float) * 2);
          }
        }
      }

      CopySamples(output, mCopiedSamples, outputChannels);
    }
    else
      CopySamples(mSamples, mCopiedSamples, outputChannels);

    return mCopiedSamples;
  }

  //************************************************************************************************
  void AudioFrame::SetSamples(const float* samples, unsigned channels)
  {
    mStoredChannels = Math::Max(channels, (unsigned)1);

    memset(mSamples, 0, sizeof(float) * MaxChannels);

    switch (channels)
    {
    case 1:
      mSamples[0] = samples[0];
      break;
    case 2:
      memcpy(mSamples, samples, sizeof(float) * 2);
      break;
    case 3:
      memcpy(mSamples, samples, sizeof(float) * 3);
      break;
    case 4:
      memcpy(mSamples, samples, sizeof(float) * 2);
      memcpy(mSamples + BackLeft, samples + 2, sizeof(float) * 2);
      break;
    case 5:
      memcpy(mSamples, samples, sizeof(float) * 3);
      memcpy(mSamples + SideLeft, samples + 3, sizeof(float) * 2);
      break;
    case 6:
      memcpy(mSamples, samples, sizeof(float) * 6);
      break;
    case 7:
      memcpy(mSamples, samples, sizeof(float) * 3);
      memcpy(mSamples + SideLeft, samples + 3, sizeof(float) * 4);
      break;
    case 8:
      memcpy(mSamples, samples, sizeof(float) * 8);
      break;
    default:
      break;
    }
  }

  //************************************************************************************************
  void AudioFrame::Clamp()
  {
    for (unsigned i = 0; i < MaxChannels; ++i)
      mSamples[i] = Math::Clamp(mSamples[i], -1.0f, 1.0f);
  }

  //************************************************************************************************
  float AudioFrame::GetMaxValue()
  {
    float value = Math::Abs(mSamples[0]);

    for (unsigned i = 1; i < MaxChannels; ++i)
    {
      value = Math::Max(value, Math::Abs(mSamples[i]));
    }

    return value;
  }

  //************************************************************************************************
  float AudioFrame::GetMonoValue()
  {
    if (mStoredChannels == 1)
      return mSamples[0];

    float value = mSamples[0];

    // Need to add all values together because samples are not necessarily stored sequentially
    for (unsigned i = 1; i < MaxChannels; ++i)
      value += mSamples[i];

    // Divide by number of channels stored because other values will be zero
    value /= mStoredChannels;

    return value;
  }

  //************************************************************************************************
  void AudioFrame::operator*=(float multiplier)
  {
    for (unsigned i = 0; i < MaxChannels; ++i)
      mSamples[i] *= multiplier;
  }

  //************************************************************************************************
  void AudioFrame::operator=(const AudioFrame& copy)
  {
    mStoredChannels = copy.mStoredChannels;
    memcpy(mSamples, copy.mSamples, sizeof(float) * MaxChannels);
  }

  //************************************************************************************************
  void AudioFrame::CopySamples(const float* source, float* destination, const unsigned channels)
  {
    switch (channels)
    {
    case 1:
      destination[0] = source[0];
      break;
    case 2:
      memcpy(destination, source, sizeof(float) * 2);
      break;
    case 3:
      memcpy(destination, source, sizeof(float) * 3);
      break;
    case 4:
      memcpy(destination, source, sizeof(float) * 2);
      memcpy(destination + 2, source + BackLeft, sizeof(float) * 2);
      break;
    case 5:
      memcpy(destination, source, sizeof(float) * 3);
      memcpy(destination + 3, source + SideLeft, sizeof(float) * 2);
      break;
    case 6:
      memcpy(destination, source, sizeof(float) * 6);
      break;
    case 7:
      memcpy(destination, source, sizeof(float) * 3);
      memcpy(destination + 3, source + SideLeft, sizeof(float) * 4);
      break;
    case 8:
      memcpy(destination, source, sizeof(float) * 8);
      break;
    default:
      break;
    }
  }

}

