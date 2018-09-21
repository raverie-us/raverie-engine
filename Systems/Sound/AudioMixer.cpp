///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

//--------------------------------------------------------------------------------------- Audio Task

//**************************************************************************************************
AudioTask::AudioTask() :
  mFunction(nullptr),
  mObject(nullptr)
{

}

//**************************************************************************************************
AudioTask::AudioTask(Functor* function, HandleOf<SoundNode> node) :
  mFunction(function),
  mObject(node)
{

}

//**************************************************************************************************
AudioTask::AudioTask(const AudioTask& other) : 
  mFunction(other.mFunction),
  mObject(other.mObject)
{
  const_cast<AudioTask&>(other).mFunction = nullptr;
}

//**************************************************************************************************
AudioTask::~AudioTask()
{
  if (mFunction) 
    delete mFunction;
}

//-------------------------------------------------------------------------------------- Audio Mixer

//**************************************************************************************************
AudioMixer::AudioMixer() :
  mSystemChannels(2),
  mMixVersionThreaded(0),
  mMinimumVolumeThresholdThreaded(0.015f),
  mSendMicrophoneInputData(cFalse),
  FinalOutputNode(nullptr),
  mMixThreadTaskWriteIndex(0),
  mGameThreadTaskWriteIndex(0),
  mShuttingDown(cFalse),
  mVolume(1.0f),
  mPeakVolumeLastMix(0.0f),
  mRmsVolumeLastMix(0.0f),
  mPreviousPeakVolumeThreaded(0.0f),
  mPreviousRMSVolumeThreaded(0),
  mResamplingThreaded(false),
  mMuted(cFalse),
  mMutingThreaded(false),
  mPeakInputVolume(0.0f),
  mSendMicrophoneInputCompressed(false),
  mSendMicrophoneInputUncompressed(false)
{

}

//**************************************************************************************************
OsInt StartMix(void* mixer)
{
  ((AudioMixer*)mixer)->MixLoopThreaded();
  return 0;
}

//**************************************************************************************************
void AudioMixer::StartMixing(Status& status)
{
  // Initialize the audio API and the input & output streams
  // If initialization was not successful, set the message on the status object
  if (!AudioIO.InitializeAPI())
    status.SetFailed(AudioIO.GetSystemErrorMessage());

  // Initialize audio output (input will get initialized when needed)
  // If not successful, set the message on the status object
  if (!AudioIO.Initialize(true, false))
    status.SetFailed(AudioIO.GetStreamErrorMessage(StreamTypes::Output));

  CheckForResamplingThreaded();

  // Create output nodes
  FinalOutputNode = new OutputNode();

  // For low frequency channel (uses audio system in constructor)
  LowPass = new LowPassFilter();
  LowPass->SetCutoffFrequency(120.0f);

  AudioIO.OutputRingBuffer.ResetBuffer();

  // Start up the mix thread
  MixThread.Initialize(StartMix, this, "Audio mix");
  MixThread.Resume();
  if (!MixThread.IsValid())
  {
    ZPrint("Error creating audio mix thread\n");
    status.SetFailed("Error creating audio mix thread");
    return;
  }

  ZPrint("Audio mix thread initialized\n");

  // Start audio output stream
  AudioIO.StartStreams(true, false);

  ZPrint("Audio initialization completed\n");
}

//**************************************************************************************************
void AudioMixer::ShutDown()
{
  TimerBlock block("Stopping Audio System.");

  ZPrint("Shutting down audio\n");

  if (MixThread.IsValid())
  {
    // Tells the mix thread to shut down
    mShuttingDown.Set(cTrue);

    // Wait for the mix thread to finish shutting down
    MixThread.WaitForCompletion();
    MixThread.Close();
  }

  // Shut down audio output, input, and API
  AudioIO.StopStreams(true, true);
  AudioIO.ShutDown();

  if (LowPass)
    delete LowPass;
}

//**************************************************************************************************
void AudioMixer::Update()
{
  // Execute tasks from the mix thread
  HandleTasks();

  // Check if we need to send microphone input to the external system
  if (mSendMicrophoneInputData.Get() != 0)
    DispatchMicrophoneInput();
}

//**************************************************************************************************
void AudioMixer::MixLoopThreaded()
{
#ifdef TRACK_TIME 
  double maxTime = ??
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
      AudioIO.WaitUntilOutputNeededThreaded();

  } while (running && Zero::ThreadingEnabled);
}

//**************************************************************************************************
void AudioMixer::AddTask(Functor* task, HandleOf<SoundNode> node)
{
  MixThreadTasksLock.Lock();
  TasksForMixThread[mMixThreadTaskWriteIndex].PushBack(AudioTask(task, node));
  MixThreadTasksLock.Unlock();
}

//**************************************************************************************************
void AudioMixer::AddTaskThreaded(Functor* task, HandleOf<SoundNode> node)
{
  GameThreadTasksLock.Lock();
  TasksForGameThread[mGameThreadTaskWriteIndex].PushBack(AudioTask(task, node));
  GameThreadTasksLock.Unlock();
}

//**************************************************************************************************
void AudioMixer::SetLatency(AudioLatency::Enum latency)
{
  AddTask(CreateFunctor(&AudioIOInterface::SetOutputLatencyThreaded, &AudioIO, latency), nullptr);
}

//**************************************************************************************************
bool AudioMixer::StartInput()
{
  if (AudioIO.GetStreamStatus(StreamTypes::Input) == StreamStatus::Started)
    return true;

  if (!AudioIO.Initialize(false, true))
    return false;

  return AudioIO.StartStreams(false, true);
}

//**************************************************************************************************
float AudioMixer::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void AudioMixer::SetVolume(const float volume)
{
  // TODO this should probably be interpolated

  mVolume.Set(volume, AudioThreads::MainThread);
}

//**************************************************************************************************
bool AudioMixer::GetMuteAllAudio()
{
  return mMuted.Get() == cTrue;
}

//**************************************************************************************************
void AudioMixer::SetMuteAllAudio(const bool muteAudio)
{
  AddTask(CreateFunctor(&AudioMixer::SetMutedThreaded, this, muteAudio), nullptr);
}

//**************************************************************************************************
unsigned AudioMixer::GetOutputChannels()
{
  return mSystemChannels.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void AudioMixer::SetOutputChannels(const unsigned channels)
{
  if (channels == mSystemChannels.Get(AudioThreads::MainThread) || channels > 8)
    return;

  if (channels == 0)
    mSystemChannels.Set(AudioIO.GetStreamChannels(StreamTypes::Output), AudioThreads::MainThread);
  else
    mSystemChannels.Set(channels, AudioThreads::MainThread);
}

//**************************************************************************************************
float AudioMixer::GetPeakOutputVolume()
{
  return mPeakVolumeLastMix.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
float AudioMixer::GetRMSOutputVolume()
{
  return mRmsVolumeLastMix.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void AudioMixer::SetMinimumVolumeThreshold(const float volume)
{
  AddTask(CreateFunctor(&AudioMixer::mMinimumVolumeThresholdThreaded, this, volume), nullptr);
}

//**************************************************************************************************
void AudioMixer::SetSendUncompressedMicInput(const bool sendInput)
{
  if (sendInput == mSendMicrophoneInputUncompressed)
    return;

  // Both modes now off - turn off sending input data
  if (!sendInput && !mSendMicrophoneInputCompressed)
    SetSendMicInput(false);
  // Both modes were off but this one is on - turn on sending input data
  else if (sendInput && !mSendMicrophoneInputCompressed)
    SetSendMicInput(true);

  mSendMicrophoneInputUncompressed = sendInput;
}

//**************************************************************************************************
void AudioMixer::SetSendCompressedMicInput(const bool sendInput)
{
  if (sendInput == mSendMicrophoneInputCompressed)
    return;

  // Both modes now off - turn off sending input data
  if (!sendInput && !mSendMicrophoneInputUncompressed)
    SetSendMicInput(false);
  // Both modes were off but this one is on - turn on sending input data
  // and initialize (or re-initialize) the encoder
  else if (sendInput && !mSendMicrophoneInputUncompressed)
  {
    Encoder.InitializeEncoder();
    SetSendMicInput(true);
  }

  mSendMicrophoneInputCompressed = sendInput;
}

//**************************************************************************************************
float AudioMixer::GetPeakInputVolume()
{
  if (mSendMicrophoneInputUncompressed || mSendMicrophoneInputCompressed)
    return mPeakInputVolume;
  else
  {
    DoNotifyWarning("Audio Warning", "To get PeakInputVolume turn on sending microphone data, either uncompressed or compressed");
    return 0.0f;
  }
}

//**************************************************************************************************
void AudioMixer::SendListenerRemovedEvent(ListenerNode* listener)
{
  SoundEvent event(listener);
  DispatchEvent(Events::SoundListenerRemoved, &event);
}

//**************************************************************************************************
bool AudioMixer::MixCurrentInstancesThreaded()
{
  if (!FinalOutputNode)
    return mShuttingDown.Get() == false;

  // Find out how many samples we can write to the ring buffer
  unsigned samplesNeeded = AudioIO.OutputRingBuffer.GetWriteAvailable();

  // Check to make sure there is available write space
  if (samplesNeeded == 0)
    return true;

  // Save the number of channels in the audio output
  unsigned outputChannels = AudioIO.GetStreamChannels(StreamTypes::Output);
  // Number of frames in the output
  unsigned outputFrames = samplesNeeded / outputChannels;

  int mixChannels = mSystemChannels.Get(AudioThreads::MixThread);

  // Check for resampling and update resample factor if necessary
  // (this can change at any time if the audio output device changes)
  CheckForResamplingThreaded();

  // Number of frames in the mix (will be different if resampling)
  unsigned mixFrames = outputFrames;
  if (mResamplingThreaded)
    mixFrames = OutputResampler.GetOutputFrameCount(outputFrames);

  // Get the audio input data
  GetAudioInputDataThreaded(samplesNeeded);

  // If sending microphone input to the external system, add the input buffer to the queue
  if (mSendMicrophoneInputData.Get() != 0)
    InputDataBuffer.Write(InputBuffer.Data(), InputBuffer.Size());

  // Resize BufferForOutput to match samples needed
  BufferForOutput.Resize(mixFrames * mixChannels);

  // Get samples from output node
  bool isThereData = FinalOutputNode->GetOutputSamples(&BufferForOutput, mixChannels, nullptr, true);

  ++mMixVersionThreaded;

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

    if (mResamplingThreaded)
      OutputResampler.SetInputBuffer(BufferForOutput.Data(), mixFrames, mixChannels);

    // Step through each frame in the output buffer
    for (unsigned frameIndex = 0; frameIndex < outputFrames; ++frameIndex)
    {
      // If not resampling, set the samples on the frame object from this frame in the mix
      if (!mResamplingThreaded)
        frame.SetSamples(BufferForOutput.Data() + (frameIndex * mixChannels), mixChannels);
      // Otherwise, interpolate between two mix frames
      else
      {
        Zero::Array<float> samples(mixChannels);
        OutputResampler.GetNextFrame(samples.Data());
        frame.SetSamples(samples.Data(), mixChannels);
      }

      // Apply the system volume
      frame *= mVolume.Get(AudioThreads::MixThread);
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
  if (peakVolume != mPreviousPeakVolumeThreaded || rmsVolume != mPreviousRMSVolumeThreaded)
  {
    mPreviousPeakVolumeThreaded = peakVolume;
    mPreviousRMSVolumeThreaded = rmsVolume;
    rmsVolume /= outputFrames;
    mPeakVolumeLastMix.Set(peakVolume, AudioThreads::MixThread);
    mRmsVolumeLastMix.Set(Math::Sqrt((float)rmsVolume) / (float)((1 << 15) - 1), AudioThreads::MixThread);
  }

  // Check if there is a volume adjustment to apply
  if (!VolumeInterpolatorThreaded.Finished())
  {
    // Apply the interpolated volume to each frame
    for (unsigned i = 0; i < MixedOutput.Size(); i += outputChannels)
    {
      float volume = VolumeInterpolatorThreaded.NextValue();

      for (unsigned j = 0; j < outputChannels; ++j)
        MixedOutput[i + j] *= volume;
    }
  }

  // If shutting down, wait for volume to ramp down to zero
  if (mShuttingDown.Get() == cTrue)
  {
    // Ramp the volume down to zero
    VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, outputFrames);
    for (unsigned i = 0; i < MixedOutput.Size(); i += outputChannels)
    {
      float volume = VolumeInterpolatorThreaded.NextValue();

      for (unsigned j = 0; j < outputChannels; ++j)
        MixedOutput[i + j] *= volume;
    }

    // Copy the data to the ring buffer
    AudioIO.OutputRingBuffer.Write(MixedOutput.Data(), MixedOutput.Size());

    return false;
  }

  // If muted, don't actually output any audio data, set everything to zero
  if (mMuted.Get() == cTrue)
    memset(MixedOutput.Data(), 0, sizeof(float) * MixedOutput.Size());

  // Check if we are switching to muted
  if (mMutingThreaded && VolumeInterpolatorThreaded.Finished())
  {
    mMutingThreaded = false;
    mMuted.Set(cTrue);
  }

  // Copy the data to the ring buffer
  AudioIO.OutputRingBuffer.Write(MixedOutput.Data(), MixedOutput.Size());

  // Still running, return true
  return true;
}

//**************************************************************************************************
int SwapBufferIndexes(int* index)
{
  int current = *index;
  *index = (current + 1) % 2;
  return current;
}

//**************************************************************************************************
void AudioMixer::HandleTasksThreaded()
{
  MixThreadTasksLock.Lock();
  TaskListType& list = TasksForMixThread[SwapBufferIndexes(&mMixThreadTaskWriteIndex)];
  MixThreadTasksLock.Unlock();

  forRange(AudioTask& task, list.All())
  {
    ErrorIf(!task.mFunction, "Functor pointer is null");
    task.mFunction->Execute();
  }

  list.Clear();
}

//**************************************************************************************************
void AudioMixer::HandleTasks()
{
  GameThreadTasksLock.Lock();
  TaskListType& list = TasksForGameThread[SwapBufferIndexes(&mGameThreadTaskWriteIndex)];
  GameThreadTasksLock.Unlock();

  forRange(AudioTask& task, list.All())
  {
    ErrorIf(!task.mFunction, "Functor pointer is null");
    task.mFunction->Execute();
  }

  list.Clear();
}

//**************************************************************************************************
void AudioMixer::CheckForResamplingThreaded()
{
  unsigned outputSampleRate = AudioIO.GetStreamSampleRate(StreamTypes::Output);

  if (cSystemSampleRate != outputSampleRate)
  {
    mResamplingThreaded = true;
    OutputResampler.SetFactor((double)cSystemSampleRate / (double)outputSampleRate);
  }
  else 
  {
    mResamplingThreaded = false;
  }
}

//**************************************************************************************************
void AudioMixer::GetAudioInputDataThreaded(unsigned howManySamples)
{
  unsigned inputChannels = AudioIO.GetStreamChannels(StreamTypes::Input);
  unsigned inputRate = AudioIO.GetStreamSampleRate(StreamTypes::Input);

  if (inputChannels == 0 || inputRate == 0)
    return;

  int mixChannels = mSystemChannels.Get(AudioThreads::MixThread);

  // Channels and sample rates match, don't need to process anything
  if (inputChannels == mixChannels && inputRate == cSystemSampleRate)
  {
    AudioIO.GetInputDataThreaded(InputBuffer, howManySamples);
  }
  else
  {
    // Save the number of frames of input to get, adjusting for resampling if necessary
    unsigned inputFrames = howManySamples / mixChannels;
    if (inputRate != cSystemSampleRate)
      inputFrames = (unsigned)(inputFrames * (float)inputRate / (float)cSystemSampleRate);

    // Need to adjust channels
    if (inputChannels != mixChannels)
    {
      // Get input data in a temporary array
      Zero::Array<float> inputSamples;
      AudioIO.GetInputDataThreaded(inputSamples, inputFrames * inputChannels);

      // Reset the InputBuffer
      InputBuffer.Clear();

      // Translate the channels for each audio frame and add samples to InputBuffer
      for (unsigned i = 0; i < inputSamples.Size(); i += inputChannels)
      {
        AudioFrame frame(inputSamples.Data() + i, inputChannels);
        float* samples = frame.GetSamples(mixChannels);
        for (int j = 0; j < mixChannels; ++j)
          InputBuffer.PushBack(samples[j]);
      }
    }
    // Not adjusting channels, just get input
    else
      AudioIO.GetInputDataThreaded(InputBuffer, inputFrames * mixChannels);

    // Need to resample
    if (inputRate != cSystemSampleRate)
    {
      // Temporary array for resampled data
      Zero::Array<float> resampledInput;
      // Set the resampling factor on the resampler object
      InputResampler.SetFactor((double)inputRate / (double)cSystemSampleRate);
      // Set the buffer on the resampler
      InputResampler.SetInputBuffer(InputBuffer.Data(), InputBuffer.Size() / mixChannels, mixChannels);
      // Array to get a frame of samples from the resampler
      Zero::Array<float> frame(mixChannels);

      bool working(true);
      while (working)
      {
        // Get the next frame of resampled data
        working = InputResampler.GetNextFrame(frame.Data());
        // Add it to the array
        resampledInput.Append(frame.All());
      }

      // Swap the resampled data into the InputBuffer
      InputBuffer.Swap(resampledInput);
    }
  }
}

//**************************************************************************************************
void AudioMixer::SetMutedThreaded(bool muteAudio)
{
  if (muteAudio && mMuted.Get() == cFalse && !mMutingThreaded)
  {
    mMutingThreaded = true;
    VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, cPropertyChangeFrames);
  }
  else if (!muteAudio && mMuted.Get() == cTrue)
  {
    mMuted.Set(cFalse);
    VolumeInterpolatorThreaded.SetValues(0.0f, 1.0f, cPropertyChangeFrames);
  }
}

//**************************************************************************************************
void AudioMixer::DispatchMicrophoneInput()
{
  unsigned available = InputDataBuffer.GetReadAvailable();
  if (available == 0)
    return;

  Array<float> inputData(available);
  InputDataBuffer.Read(inputData.Data(), available);
  
  // Save the peak volume from the input samples
  mPeakInputVolume = 0.0f;
  forRange(float sample, inputData.All())
  {
    float absSample = Math::Abs(sample);
    if (absSample > mPeakInputVolume)
      mPeakInputVolume = absSample;
  }

  int channels = mSystemChannels.Get(AudioThreads::MainThread);

  // If we're sending uncompressed input data, send the entire buffer
  if (mSendMicrophoneInputUncompressed)
  {
    AudioFloatDataEvent event;
    event.Channels = channels;
    event.AudioData = ZilchAllocate(ArrayClass<float>);
    event.AudioData->NativeArray = inputData;
    Z::gSound->DispatchEvent(Events::MicrophoneUncompressedFloatData, &event);
  }

  // Check if we're sending compressed data and have an encoder
  if (mSendMicrophoneInputCompressed)
  {
    // Add the input samples to the end of the buffer
    PreviousInputSamples.Append(inputData.All());

    unsigned totalPacketSamples = AudioFileEncoder::cPacketFrames * channels;

    // While we have at least the number of samples for a packet, encode them
    while (PreviousInputSamples.Size() > totalPacketSamples)
    {
      Zero::Array<float> monoSamples;

      // If the system is in mono, just add samples
      if (channels == 1)
        monoSamples.Append(PreviousInputSamples.SubRange(0, AudioFileEncoder::cPacketFrames));
      else
      {
        // Translate samples to mono
        BufferRange sampleRange = PreviousInputSamples.All();
        for (unsigned i = 0; i < totalPacketSamples; i += channels)
        {
          float monoValue(0.0f);
          for (int j = 0; j < channels; ++j, sampleRange.PopFront())
            monoValue += sampleRange.Front();
          monoValue /= channels;
          monoSamples.PushBack(monoValue);
        }
      }

      // Remove the samples from the array
      PreviousInputSamples.Erase(PreviousInputSamples.SubRange(0, totalPacketSamples));

      // Encode the packet
      Zero::Array<byte> dataArray;
      Encoder.EncodePacket(monoSamples.Data(), AudioFileEncoder::cPacketFrames, dataArray);

      // Send the event with the encoded data
      AudioByteDataEvent event;
      event.AudioData = ZilchAllocate(ArrayClass<byte>);
      event.AudioData->NativeArray = dataArray;
      Z::gSound->DispatchEvent(Events::MicrophoneCompressedByteData, &event);
    }
  }
}

//**************************************************************************************************
void AudioMixer::SetSendMicInput(bool turnOn)
{
  if (turnOn)
  {
    if (!StartInput())
      return;

    InputDataBuffer.ResetBuffer();
    mSendMicrophoneInputData.Set(1);
  }
  else
    mSendMicrophoneInputData.Set(0);
}

//-------------------------------------------------------------------------------------- Audio Frame

namespace AudioChannelTranslation
{

// Matrix columns are FrontLeft, FrontRight, Center, LowFreq, SideLeft, SideRight, BackLeft, BackRight

const float cSqrt2Inv = 1.0f / Math::Sqrt(2.0f);

const float ChannelMatrix1[cMaxChannels] =
{
  cSqrt2Inv, cSqrt2Inv, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f
};

const float ChannelMatrix2[cMaxChannels * 2] =
{
  1.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f,
  0.0f, 1.0f, cSqrt2Inv, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv
};

const float ChannelMatrix3[cMaxChannels * 3] =
{
  1.0f, 0.0f, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, cSqrt2Inv, 0.0f, cSqrt2Inv,
  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};

const float ChannelMatrix4[cMaxChannels * 4] =
{
  1.0f, 0.0f, cSqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, cSqrt2Inv, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
};

const float ChannelMatrix5[cMaxChannels * 5] =
{
  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
};

const float ChannelMatrix6[cMaxChannels * 6] =
{
  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
};

const float ChannelMatrix7[cMaxChannels * 7] =
{
  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

const float ChannelMatrix8[cMaxChannels * 8] =
{
  1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
};

} // namespace AudioChannelTranslation

//**************************************************************************************************
AudioFrame::AudioFrame(float* samples, unsigned channels)
{
  SetSamples(samples, channels);

  Matrices[0] = nullptr;
  Matrices[1] = AudioChannelTranslation::ChannelMatrix1;
  Matrices[2] = AudioChannelTranslation::ChannelMatrix2;
  Matrices[3] = AudioChannelTranslation::ChannelMatrix3;
  Matrices[4] = AudioChannelTranslation::ChannelMatrix4;
  Matrices[5] = AudioChannelTranslation::ChannelMatrix5;
  Matrices[6] = AudioChannelTranslation::ChannelMatrix6;
  Matrices[7] = AudioChannelTranslation::ChannelMatrix7;
  Matrices[8] = AudioChannelTranslation::ChannelMatrix8;
}

//**************************************************************************************************
AudioFrame::AudioFrame() :
  mStoredChannels(1)
{
  memset(mSamples, 0, sizeof(float) * cMaxChannels);

  Matrices[0] = nullptr;
  Matrices[1] = AudioChannelTranslation::ChannelMatrix1;
  Matrices[2] = AudioChannelTranslation::ChannelMatrix2;
  Matrices[3] = AudioChannelTranslation::ChannelMatrix3;
  Matrices[4] = AudioChannelTranslation::ChannelMatrix4;
  Matrices[5] = AudioChannelTranslation::ChannelMatrix5;
  Matrices[6] = AudioChannelTranslation::ChannelMatrix6;
  Matrices[7] = AudioChannelTranslation::ChannelMatrix7;
  Matrices[8] = AudioChannelTranslation::ChannelMatrix8;
}

//**************************************************************************************************
AudioFrame::AudioFrame(const AudioFrame& copy) :
  mStoredChannels(copy.mStoredChannels)
{
  memcpy(mSamples, copy.mSamples, sizeof(float) * cMaxChannels);

  Matrices[0] = nullptr;
  Matrices[1] = AudioChannelTranslation::ChannelMatrix1;
  Matrices[2] = AudioChannelTranslation::ChannelMatrix2;
  Matrices[3] = AudioChannelTranslation::ChannelMatrix3;
  Matrices[4] = AudioChannelTranslation::ChannelMatrix4;
  Matrices[5] = AudioChannelTranslation::ChannelMatrix5;
  Matrices[6] = AudioChannelTranslation::ChannelMatrix6;
  Matrices[7] = AudioChannelTranslation::ChannelMatrix7;
  Matrices[8] = AudioChannelTranslation::ChannelMatrix8;
}

//**************************************************************************************************
float* AudioFrame::GetSamples(const unsigned outputChannels)
{
  if (outputChannels != mStoredChannels)
  {
    float output[cMaxChannels] = { 0 };

    // Down-mixing
    if (outputChannels < mStoredChannels)
    {
      for (unsigned outChannel = 0; outChannel < outputChannels; ++outChannel)
      {
        for (unsigned i = 0; i < cMaxChannels; ++i)
        {
          output[outChannel] += mSamples[i] * Matrices[outputChannels][i + (outChannel * cMaxChannels)];
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
            output[Center] = (mSamples[0] + mSamples[1]) * AudioChannelTranslation::cSqrt2Inv;

          output[BackLeft] = output[BackRight] = output[SideLeft] = output[SideRight] 
            = (mSamples[0] - mSamples[1]) * AudioChannelTranslation::cSqrt2Inv;
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

//**************************************************************************************************
void AudioFrame::SetSamples(const float* samples, unsigned channels)
{
  mStoredChannels = Math::Max(channels, 1u);

  memset(mSamples, 0, sizeof(float) * cMaxChannels);

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

//**************************************************************************************************
void AudioFrame::Clamp()
{
  for (unsigned i = 0; i < cMaxChannels; ++i)
    mSamples[i] = Math::Clamp(mSamples[i], -1.0f, 1.0f);
}

//**************************************************************************************************
float AudioFrame::GetMaxValue()
{
  float value = Math::Abs(mSamples[0]);

  for (unsigned i = 1; i < cMaxChannels; ++i)
  {
    float newValue = Math::Abs(mSamples[i]);
    if (newValue > value)
      value = newValue;
  }

  return value;
}

//**************************************************************************************************
float AudioFrame::GetMonoValue()
{
  if (mStoredChannels == 1)
    return mSamples[0];

  float value = mSamples[0];

  for (unsigned i = 1; i < cMaxChannels; ++i)
    value += mSamples[i];

  value /= mStoredChannels;

  return value;
}

//**************************************************************************************************
void AudioFrame::operator*=(float multiplier)
{
  for (unsigned i = 0; i < cMaxChannels; ++i)
    mSamples[i] *= multiplier;
}

//**************************************************************************************************
void AudioFrame::operator=(const AudioFrame& copy)
{
  mStoredChannels = copy.mStoredChannels;
  memcpy(mSamples, copy.mSamples, sizeof(float) * cMaxChannels);
}

//**************************************************************************************************
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

} // namespace Zero
