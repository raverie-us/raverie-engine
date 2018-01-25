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
    StopDecodeThread(0),
    SendMicrophoneInputData(false)
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
  Zero::OsInt StartDecoding(void* system)
  {
    ((Audio::AudioSystemInternal*)system)->DecodeLoopThreaded();
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

    // Initialize audio input (non-essential)
    StreamStatus::Enum inputStatus = AudioIO->InitializeStream(StreamTypes::Input);

    CheckForResampling();

    // Create output nodes
    Zero::Status tempStatus;
    FinalOutputNode = new OutputNode(tempStatus, "FinalOutputNode", &NodeInt, false);
    FinalOutputNodeThreaded = (OutputNode*)FinalOutputNode->GetSiblingNode();

    // For low frequency channel (uses audio system in constructor)
    LowPass = new LowPassFilter();
    LowPass->SetCutoffFrequency(120.0f);

    // Start up the decoding thread
    DecodeThreadEvent.Initialize();
    DecodeThread.Initialize(StartDecoding, this, "Audio decoding");
    DecodeThread.Resume();
    if (!DecodeThread.IsValid())
    {
      ZPrint("Error creating audio decoding thread\n");
      status.SetFailed("Error creating audio decoding thread");
      return;
    }

    ZPrint("Audio decoding thread initialized\n");

    AudioIO->OutputRingBuffer.ResetBuffer();

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

    AudioIO->StartStream(StreamTypes::Output);
    AudioIO->StartStream(StreamTypes::Input);

    ZPrint("Audio was successfully initialized\n");
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

    if (DecodeThread.IsValid())
    {
      Zero::AtomicStore(&StopDecodeThread, 1);
      DecodeThreadEvent.Signal();
      DecodeThread.WaitForCompletion();
      DecodeThread.Close();
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
    while (running)
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
      if (running)
        AudioIO->WaitUntilOutputNeededThreaded();
    }

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
    if (!Resampling)
      BufferForOutput.Resize(samplesNeeded);
    else
    {
      BufferForOutput.Resize(mixFrames * SystemChannelsThreaded);
    }

    // Get samples from output node
    bool isThereData = FinalOutputNodeThreaded->GetOutputSamples(&BufferForOutput, 
      SystemChannelsThreaded, nullptr, true);
    ++MixVersionNumber;

    // Set the size of the MixedOutput buffer
    MixedOutput.Resize(BufferForOutput.Size());

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

        // If the mix and output channels don't match, translate the samples
        frame.TranslateChannels(outputChannels);
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

        // If 5.1 or 7.1, handle low frequency channel
        if (outputChannels == 6 || outputChannels == 8)
        {
          float monoSample = frame.GetMonoValue();
          LowPass->ProcessFrame(&monoSample, &frame.Samples[3], 1);
        }

        // Copy this frame of samples to the output buffer
        memcpy(MixedOutput.Data() + (frameIndex * outputChannels), frame.Samples,
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

    // If shutting down, wait for volume to ramp down to zero
    if (ShuttingDownThreaded)
    {
      VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, outputFrames);

      // Volume will interpolate down to zero
      for (unsigned i = 0; i < MixedOutput.Size(); i += outputChannels)
      {
        float volume = VolumeInterpolatorThreaded.NextValue();

        for (unsigned j = 0; j < outputChannels; ++j)
          MixedOutput[i + j] *= volume;
      }

      // Copy the data to the ring buffer
      AudioIO->OutputRingBuffer.Write(MixedOutput.Data(), MixedOutput.Size());

      return false;
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
  void AudioSystemInternal::AddDecodingTask(Zero::Functor* function)
  {
    DecodingQueue.Write(function);
    DecodeThreadEvent.Signal();
  }

  //************************************************************************************************
  void AudioSystemInternal::DecodeLoopThreaded()
  {
    while (Zero::AtomicCompareExchangeBool(&StopDecodeThread, 0, 0))
    {
      Zero::Functor* function;
      while (DecodingQueue.Read(function))
      {
        function->Execute();
        delete function;
      }

      DecodeThreadEvent.Wait();
    }
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
          frame.TranslateChannels(SystemChannelsThreaded);
          for (unsigned j = 0; j < SystemChannelsThreaded; ++j)
            InputBuffer.PushBack(frame.Samples[j]);
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

  //------------------------------------------------------------------------- Audio Channels Manager

  //************************************************************************************************
  AudioChannelsManager::AudioChannelsManager()
  {
    CreateSpeakerMatrix();
  }

  //************************************************************************************************
  AudioChannelsManager::~AudioChannelsManager()
  {
    // Delete speaker data, starting with the data for 2 channels
    delete SpeakerMatrixArrays[2];
    // Continue deleting data for other channel setups
    for (unsigned i = 3; i < 9; ++i)
    {
      if (SpeakerMatrixArrays[i])
        delete[] SpeakerMatrixArrays[i];
    }
  }

  //************************************************************************************************
  bool CheckGains(float& largestSmallestGain, const float gainCheck1, const float gainCheck2, 
    float& gain1, float& gain2)
  {
    if (gainCheck1 < gainCheck2 && gainCheck1 > largestSmallestGain)
    {
      gain1 = gainCheck1;
      gain2 = gainCheck2;
      largestSmallestGain = gainCheck1;
      return true;
    }
    else if (gainCheck2 < gainCheck1 && gainCheck2 > largestSmallestGain)
    {
      gain1 = gainCheck1;
      gain2 = gainCheck2;
      largestSmallestGain = gainCheck2;
      return true;
    }
    else
      return false;
  }

  //************************************************************************************************
  void AudioChannelsManager::GetClosestSpeakerValues(Math::Vec2 sourceVec, unsigned numberOfChannels, 
    float& gain1, float& gain2, int& channel1, int& channel2)
  {
    // Only valid for two channels or more
    if (numberOfChannels < 2)
      return;

    float largestSmallestGain = 0.0f;
    SpeakerInfo& info = SpeakerMatrixArrays[numberOfChannels][0];

    // Get gain values from first speaker pair
    info.GetGainValues(sourceVec, gain1, gain2);
    if (gain1 < gain2)
      largestSmallestGain = gain1;
    else
      largestSmallestGain = gain2;

    if (numberOfChannels > 2)
    {
      // Check to see if any other speaker pairs are better
      for (unsigned i = 1; i < numberOfChannels; ++i)
      {
        // 6 and 8 channels only have 5 and 7 speakers (LF handled separately)
        if ((numberOfChannels == 6 && i == 5) || (numberOfChannels == 8 && i == 7))
          break;

        float tempGain1, tempGain2;
        SpeakerMatrixArrays[numberOfChannels][i].GetGainValues(sourceVec, tempGain1, tempGain2);
        if (CheckGains(largestSmallestGain, tempGain1, tempGain2, gain1, gain2))
        {
          info = SpeakerMatrixArrays[numberOfChannels][i];
        }
      }
    }

    // Get channels from best speaker pair
    channel1 = info.Channel1;
    channel2 = info.Channel2;
  }

  //************************************************************************************************
  void AudioChannelsManager::CreateSpeakerMatrix()
  {
    float radianMultiplier = Math::cTwoPi / 360.0f;
    Math::Mat2 rotationMatrix;
    Math::Vec2 normalVec(1.0f, 0.0f);

    Math::Vec2 speaker1Vector = normalVec;

    rotationMatrix.Rotate(30.0f * radianMultiplier);
    Math::Vec2 speaker2Vector = rotationMatrix.Transform(normalVec);

    rotationMatrix.Rotate(110.0f * radianMultiplier);
    Math::Vec2 speaker3Vector = rotationMatrix.Transform(normalVec);

    rotationMatrix.Rotate(145.0f * radianMultiplier);
    Math::Vec2 speaker4Vector = rotationMatrix.Transform(normalVec);

    rotationMatrix.Rotate(215.0f * radianMultiplier);
    Math::Vec2 speaker5Vector = rotationMatrix.Transform(normalVec);

    rotationMatrix.Rotate(250.0f * radianMultiplier);
    Math::Vec2 speaker6Vector = rotationMatrix.Transform(normalVec);

    rotationMatrix.Rotate(330.0f * radianMultiplier);
    Math::Vec2 speaker7Vector = rotationMatrix.Transform(normalVec);

    Math::Mat2 Speaker1_2MatrixInv = Math::Mat2(speaker1Vector.x, speaker1Vector.y, speaker2Vector.x, 
      speaker2Vector.y).Inverted();
    Math::Mat2 Speaker2_3MatrixInv = Math::Mat2(speaker2Vector.x, speaker2Vector.y, speaker3Vector.x,
      speaker3Vector.y).Inverted();
    Math::Mat2 Speaker3_4MatrixInv = Math::Mat2(speaker3Vector.x, speaker3Vector.y, speaker4Vector.x,
      speaker4Vector.y).Inverted();
    Math::Mat2 Speaker4_5MatrixInv = Math::Mat2(speaker4Vector.x, speaker4Vector.y, speaker5Vector.x, 
      speaker5Vector.y).Inverted();
    Math::Mat2 Speaker5_6MatrixInv = Math::Mat2(speaker5Vector.x, speaker5Vector.y, speaker6Vector.x,
      speaker6Vector.y).Inverted();
    Math::Mat2 Speaker6_7MatrixInv = Math::Mat2(speaker6Vector.x, speaker6Vector.y, speaker7Vector.x, 
      speaker7Vector.y).Inverted();
    Math::Mat2 Speaker7_1MatrixInv = Math::Mat2(speaker7Vector.x, speaker7Vector.y, speaker1Vector.x, 
      speaker1Vector.y).Inverted();
    Math::Mat2 Speaker2_7MatrixInv = Math::Mat2(speaker2Vector.x, speaker2Vector.y, speaker7Vector.x, 
      speaker7Vector.y).Inverted();
    Math::Mat2 Speaker3_6MatrixInv = Math::Mat2(speaker3Vector.x, speaker3Vector.y, speaker6Vector.x, 
      speaker6Vector.y).Inverted();

    // No matrices for these numbers of channels
    SpeakerMatrixArrays[0] = nullptr;
    SpeakerMatrixArrays[1] = nullptr;

    // **** 2 channels ****
    SpeakerMatrixArrays[2] = new SpeakerInfo;
    SpeakerInfo* info = SpeakerMatrixArrays[2];
    // Front left to front right
    info->SpeakerMatrix = Speaker2_7MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 1;

    // **** 3 channels ****
    SpeakerMatrixArrays[3] = new SpeakerInfo[3];
    // Center to front left
    info = &SpeakerMatrixArrays[3][0];
    info->SpeakerMatrix = Speaker1_2MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 0;
    // Front left to front right
    info = &SpeakerMatrixArrays[3][1];
    info->SpeakerMatrix = Speaker2_7MatrixInv;
    info->Channel1 = 0;
    info->Channel1 = 1;
    // Front right to center
    info = &SpeakerMatrixArrays[3][2];
    info->SpeakerMatrix = Speaker7_1MatrixInv;
    info->Channel1 = 1;
    info->Channel2 = 2;

    // **** 4 channels ****
    SpeakerMatrixArrays[4] = new SpeakerInfo[4];
    // Front left to side left
    info = &SpeakerMatrixArrays[4][0];
    info->SpeakerMatrix = Speaker2_3MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 2;
    // Side left to side right
    info = &SpeakerMatrixArrays[4][1];
    info->SpeakerMatrix = Speaker3_6MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 3;
    // Side right to front right
    info = &SpeakerMatrixArrays[4][2];
    info->SpeakerMatrix = Speaker6_7MatrixInv;
    info->Channel1 = 3;
    info->Channel2 = 1;
    // Front left to front right
    info = &SpeakerMatrixArrays[4][3];
    info->SpeakerMatrix = Speaker2_7MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 1;

    // **** 5 channels **** 
    SpeakerMatrixArrays[5] = new SpeakerInfo[5];
    // Center to front left
    info = &SpeakerMatrixArrays[5][0];
    info->SpeakerMatrix = Speaker1_2MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 0;
    // Front left to side left
    info = &SpeakerMatrixArrays[5][1];
    info->SpeakerMatrix = Speaker2_3MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 3;
    // Side left to side right
    info = &SpeakerMatrixArrays[5][2];
    info->SpeakerMatrix = Speaker3_6MatrixInv;
    info->Channel1 = 3;
    info->Channel2 = 4;
    // Side right to front right
    info = &SpeakerMatrixArrays[5][3];
    info->SpeakerMatrix = Speaker6_7MatrixInv;
    info->Channel1 = 4;
    info->Channel2 = 1;
    // Front right to center
    info = &SpeakerMatrixArrays[5][4];
    info->SpeakerMatrix = Speaker7_1MatrixInv;
    info->Channel1 = 1;
    info->Channel2 = 2;

    // **** 6 channels (5.1) **** 
    SpeakerMatrixArrays[6] = new SpeakerInfo[5];
    // Center to front left
    info = &SpeakerMatrixArrays[6][0];
    info->SpeakerMatrix = Speaker1_2MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 0;
    // Front left to side left
    info = &SpeakerMatrixArrays[6][1];
    info->SpeakerMatrix = Speaker2_3MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 4;
    // Side left to side right
    info = &SpeakerMatrixArrays[6][2];
    info->SpeakerMatrix = Speaker3_6MatrixInv;
    info->Channel1 = 4;
    info->Channel2 = 5;
    // Side right to front right
    info = &SpeakerMatrixArrays[6][3];
    info->SpeakerMatrix = Speaker6_7MatrixInv;
    info->Channel1 = 5;
    info->Channel2 = 1;
    // Front right to center
    info = &SpeakerMatrixArrays[6][4];
    info->SpeakerMatrix = Speaker7_1MatrixInv;
    info->Channel1 = 1;
    info->Channel2 = 2;

    // **** 7 channels ****
    SpeakerMatrixArrays[7] = new SpeakerInfo[7];
    // Center to front left
    info = &SpeakerMatrixArrays[7][0];
    info->SpeakerMatrix = Speaker1_2MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 0;
    // Front left to side left
    info = &SpeakerMatrixArrays[7][1];
    info->SpeakerMatrix = Speaker2_3MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 3;
    // Side left to back left
    info = &SpeakerMatrixArrays[7][2];
    info->SpeakerMatrix = Speaker3_4MatrixInv;
    info->Channel1 = 3;
    info->Channel2 = 5;
    // Back left to back right
    info = &SpeakerMatrixArrays[7][3];
    info->SpeakerMatrix = Speaker4_5MatrixInv;
    info->Channel1 = 5;
    info->Channel2 = 6;
    // Back right to side right
    info = &SpeakerMatrixArrays[7][4];
    info->SpeakerMatrix = Speaker5_6MatrixInv;
    info->Channel1 = 6;
    info->Channel2 = 4;
    // Side right to front right
    info = &SpeakerMatrixArrays[7][5];
    info->SpeakerMatrix = Speaker6_7MatrixInv;
    info->Channel1 = 4;
    info->Channel2 = 1;
    // Front right to center
    info = &SpeakerMatrixArrays[7][6];
    info->SpeakerMatrix = Speaker7_1MatrixInv;
    info->Channel1 = 1;
    info->Channel2 = 2;

    // **** 8 channels (7.1) ****
    SpeakerMatrixArrays[8] = new SpeakerInfo[7];
    // Center to front left
    info = &SpeakerMatrixArrays[8][0];
    info->SpeakerMatrix = Speaker1_2MatrixInv;
    info->Channel1 = 2;
    info->Channel2 = 0;
    // Front left to side left
    info = &SpeakerMatrixArrays[8][1];
    info->SpeakerMatrix = Speaker2_3MatrixInv;
    info->Channel1 = 0;
    info->Channel2 = 4;
    // Side left to back left
    info = &SpeakerMatrixArrays[8][2];
    info->SpeakerMatrix = Speaker3_4MatrixInv;
    info->Channel1 = 4;
    info->Channel2 = 6;
    // Back left to back right
    info = &SpeakerMatrixArrays[8][3];
    info->SpeakerMatrix = Speaker4_5MatrixInv;
    info->Channel1 = 6;
    info->Channel2 = 7;
    // Back right to side right
    info = &SpeakerMatrixArrays[8][4];
    info->SpeakerMatrix = Speaker5_6MatrixInv;
    info->Channel1 = 7;
    info->Channel2 = 5;
    // Side right to front right
    info = &SpeakerMatrixArrays[8][5];
    info->SpeakerMatrix = Speaker6_7MatrixInv;
    info->Channel1 = 5;
    info->Channel2 = 1;
    // Front right to center
    info = &SpeakerMatrixArrays[8][6];
    info->SpeakerMatrix = Speaker7_1MatrixInv;
    info->Channel1 = 1;
    info->Channel2 = 2;
  }

  //************************************************************************************************
  void AudioChannelsManager::SpeakerInfo::GetGainValues(const Math::Vec2& sourceVec, float& gain1, 
    float& gain2)
  {
    if (Channel1 < 0)
    {
      gain1 = 0;
      gain2 = 0;
      return;
    }

    gain1 = (sourceVec.x * SpeakerMatrix.m00 + sourceVec.y * SpeakerMatrix.m10);
    gain2 = (sourceVec.x * SpeakerMatrix.m01 + sourceVec.y * SpeakerMatrix.m11);
  }

  //------------------------------------------------------------------------------------ Audio Frame

  //************************************************************************************************
  AudioFrame::AudioFrame(float* samples, unsigned channels) : 
    HowManyChannels(channels)
  {
    SetSamples(samples, channels);

    Matrices[0] = ChannelMatrix1;
    Matrices[1] = ChannelMatrix2;
    Matrices[2] = ChannelMatrix3;
    Matrices[3] = ChannelMatrix4;
    Matrices[4] = ChannelMatrix5;
    Matrices[5] = ChannelMatrix6;
    Matrices[6] = ChannelMatrix7;
    Matrices[7] = ChannelMatrix8;
  }

  //************************************************************************************************
  AudioFrame::AudioFrame() :
    HowManyChannels(0)
  {
    memset(Samples, 0, sizeof(float) * MaxChannels);

    Matrices[0] = ChannelMatrix1;
    Matrices[1] = ChannelMatrix2;
    Matrices[2] = ChannelMatrix3;
    Matrices[3] = ChannelMatrix4;
    Matrices[4] = ChannelMatrix5;
    Matrices[5] = ChannelMatrix6;
    Matrices[6] = ChannelMatrix7;
    Matrices[7] = ChannelMatrix8;
  }

  //************************************************************************************************
  AudioFrame::AudioFrame(const AudioFrame& copy) :
    HowManyChannels(copy.HowManyChannels)
  {
    memset(Samples, 0, sizeof(float) * MaxChannels);
    memcpy(Samples, copy.Samples, sizeof(float) * HowManyChannels);
  }

  //************************************************************************************************
  void AudioFrame::TranslateChannels(const unsigned channels)
  {
    if (channels == HowManyChannels)
      return;

    float output[MaxChannels] = { 0 };

    const float* matrix;
    if (channels < HowManyChannels)
      matrix = Matrices[channels - 1];
    else
      matrix = Matrices[HowManyChannels - 1];

    for (unsigned outChannel = 0; outChannel < channels; ++outChannel)
    {
      for (unsigned sourceChannel = 0; sourceChannel < MaxChannels; ++sourceChannel)
      {
        output[outChannel] += Samples[sourceChannel] * matrix[(sourceChannel * channels) + outChannel];
      }
    }

    memcpy(Samples, output, sizeof(float) * MaxChannels);
  }

  //************************************************************************************************
  void AudioFrame::SetSamples(float* samples, unsigned channels)
  {
    HowManyChannels = channels;

    memset(Samples, 0, sizeof(float) * MaxChannels);

    switch (channels)
    {
    case 1:
      Samples[0] = samples[0];
      break;
    case 2:
      memcpy(Samples, samples, sizeof(float) * 2);
      break;
    case 3:
      memcpy(Samples, samples, sizeof(float) * 3);
      break;
    case 4:
      memcpy(Samples, samples, sizeof(float) * 2);
      memcpy(Samples + BackLeft, samples + 2, sizeof(float) * 2);
      break;
    case 5:
      memcpy(Samples, samples, sizeof(float) * 3);
      memcpy(Samples + SideLeft, samples + 3, sizeof(float) * 2);
      break;
    case 6:
      memcpy(Samples, samples, sizeof(float) * 6);
      break;
    case 7:
      memcpy(Samples, samples, sizeof(float) * 3);
      memcpy(Samples + SideLeft, samples + 3, sizeof(float) * 4);
      break;
    case 8:
      memcpy(Samples, samples, sizeof(float) * 8);
      break;
    default:
      break;
    }
  }

  //************************************************************************************************
  void AudioFrame::Clamp()
  {
    for (unsigned i = 0; i < HowManyChannels; ++i)
      Samples[i] = Math::Clamp(Samples[i], -1.0f, 1.0f);
  }

  //************************************************************************************************
  float AudioFrame::GetMaxValue()
  {
    float value = Math::Abs(Samples[0]);

    for (unsigned i = 1; i < HowManyChannels; ++i)
    {
      float newValue = Math::Abs(Samples[i]);
      if (newValue > value)
        value = newValue;
    }

    return value;
  }

  //************************************************************************************************
  float AudioFrame::GetMonoValue()
  {
    if (HowManyChannels == 1)
      return Samples[0];

    float value = Samples[0];

    for (unsigned i = 1; i < HowManyChannels; ++i)
      value += Samples[i];

    value /= HowManyChannels;

    return value;
  }

  //************************************************************************************************
  void AudioFrame::operator*=(float multiplier)
  {
    for (unsigned i = 0; i < HowManyChannels; ++i)
      Samples[i] *= multiplier;
  }

  //************************************************************************************************
  void AudioFrame::operator=(const AudioFrame& copy)
  {
    HowManyChannels = copy.HowManyChannels;
    memcpy(Samples, copy.Samples, sizeof(float) * MaxChannels);
  }

}

