///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

#define MAXNODES 2048

namespace Audio
{
  AudioSystemInternal *gAudioSystem;

  //-------------------------------------------------------------------------- Audio System Internal

  //************************************************************************************************
  AudioSystemInternal::AudioSystemInternal(ExternalSystemInterface* extInterface) : 
    ExternalInterface(extInterface), 
    MinimumVolumeThresholdThreaded(0.015f),
    SystemSampleRate(44100),
    ShuttingDownThreaded(false), 
    SystemVolumeThreaded(1.0f),
    SystemChannelsThreaded(2),
    Volume(1.0f), 
    ResetPA(false), 
    MixVersionNumber(0), 
    FinalOutputNode(nullptr), 
    FinalOutputNodeThreaded(nullptr),
    DecodingFinished(true),
    PeakVolumeLastMix(0.0f),
    RmsVolumeLastMix(0.0f),
    NodeCount(0),
    NextInterpolator(0),
    LowPass(nullptr)
  {
    gAudioSystem = this;

    // Start with sixteen interpolator objects
    InterpolatorArray.Resize(16);
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
    MixBufferSizeThreaded = AudioIO.GetBaseBufferSize() * SystemChannelsThreaded;

    // Initialize Port Audio, and quit if it didn't work
    AudioIO.Initialize(status);
    if (status.Failed())
      return;

    // Start up the mix thread
    MixThread.Initialize(StartMix, this, "MixThread");
    MixThread.Resume();
    if (!MixThread.IsValid())
    {
      ZPrint("Error creating audio mix thread\n");
      status.SetFailed("Error creating audio mix thread");
      AudioIO.StopStream();
      return;
    }

    ZPrint("Mix thread initialized\n");
    ZPrint("Audio was successfully initialized\n");

    // Create output nodes
    FinalOutputNode = new OutputNode(status, "FinalOutputNode", &NodeInt, false);
    FinalOutputNodeThreaded = (OutputNode*)FinalOutputNode->GetSiblingNode();

    // For low frequency channel (uses audio system in constructor)
    LowPass = new LowPassFilter();
    LowPass->SetCutoffFrequency(120.0f);
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

    if (!DecodingFinished)
    {
      DecodeLock.Lock();
      DecodingList.Clear();
      DecodeLock.Unlock();
      DecodeThread.WaitForCompletion();
      DecodeThread.Close();
    }

    // Shut down PortAudio
    AudioIO.ShutDown(status);

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

    forRange(InterpolatorContainer& container, InterpolatorArray.All())
      delete container.Object;
    
    if (LowPass)
      delete LowPass;
  }

  //************************************************************************************************
  void AudioSystemInternal::MixLoopThreaded()
  {
    BufferForOutput.Resize(MixBufferSizeThreaded);

#ifdef _DEBUG 
    double maxTime = (double)AudioIO.OutputBufferSizeThreaded / (double)AudioIO.OutputChannelsThreaded 
      / (double)SystemSampleRate;
#endif
    
    bool running = true;
    while (running)
    {
#ifdef _DEBUG
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

#ifdef _DEBUG
      double timeDiff = (double)(clock() - time) / CLOCKS_PER_SEC;
      if (timeDiff > maxTime)
      {
        Zero::String* string = new Zero::String("Mix took too long (informational message, not an error)");
        AddTaskThreaded(Zero::CreateFunctor(&ExternalSystemInterface::SendAudioEvent, 
          ExternalInterface, Notify_Error, (void*)string));
      }
#endif

      // Wait until more data is needed
      if (running)
        AudioIO.WaitUntilOutputNeeded();
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

    // Store number of frames 
    unsigned numberOfFrames = MixBufferSizeThreaded / SystemChannelsThreaded;

    // Get samples from output node
    bool isThereData = FinalOutputNodeThreaded->GetOutputSamples(&BufferForOutput, 
      SystemChannelsThreaded, nullptr, true);
    ++MixVersionNumber;

    // Get the current output buffer
    float* outputBuffer = AudioIO.GetOutputBuffer();

    float peakVolume(0.0f);
    unsigned rmsVolume(0);

    // If there is no real data, just reset output buffer to 0
    if (!isThereData)
      memset(outputBuffer, 0, AudioIO.OutputBufferSizeThreaded * sizeof(float));
    else
    {
      // Copy samples into output buffer, apply volume, and check for clipping
      bool clipping(false);
      float* output;
      for (unsigned i = 0; i < numberOfFrames; ++i)
      {
        AudioFrame frame(BufferForOutput.Data() + (i * SystemChannelsThreaded), SystemChannelsThreaded);
        if (SystemChannelsThreaded != AudioIO.OutputChannelsThreaded)
        {
          // Translate channels 
          frame.TranslateChannels(AudioIO.OutputChannelsThreaded);
          //AudioFrame outputFrame(inputFrame, AudioIO.OutputChannelsThreaded);
          output = frame.Samples;
        }
        else
          output = BufferForOutput.Data() + (i * SystemChannelsThreaded);

        float frameTotal(0.0f);
        // Apply system volume to copied samples, check for clipping, and copy to output buffer
        for (unsigned j = 0; j < AudioIO.OutputChannelsThreaded; ++j)
        {
          // Save reference to sample
          float& sample = output[j];
          // Apply system volume adjustment
          sample *= SystemVolumeThreaded;

          // Check for clipping
          if (sample > 1.0f)
          {
            sample = 1.0f;
            clipping = true;
          }
          else if (sample < -1.0f)
          {
            sample = -1.0f;
            clipping = true;
          }

          frameTotal += sample;
          if (Math::Abs(sample) > peakVolume)
            peakVolume = Math::Abs(sample);

          // Copy sample to output buffer
          outputBuffer[(i * AudioIO.OutputChannelsThreaded) + j] = sample;
        }

        // Use 16 bit int for RMS volume
        unsigned value = (unsigned)((frameTotal / AudioIO.OutputChannelsThreaded) * ((1 << 15) - 1));
        rmsVolume += value * value;

        // If 5.1 or 7.1, handle low frequency channel
        if (AudioIO.OutputChannelsThreaded == 6 || AudioIO.OutputChannelsThreaded == 8)
        {
          // Get mono sample value
          float monoSample(0.0f);
          for (unsigned j = 0; j < AudioIO.OutputChannelsThreaded; ++j)
            monoSample += outputBuffer[(i * AudioIO.OutputChannelsThreaded) + j];

          LowPass->ProcessSample(&monoSample, outputBuffer + (i * AudioIO.OutputChannelsThreaded) + 3, 1);
        }
      }

      // If there was clipping during this mix, notify the external engine
      if (clipping)
        AddTaskThreaded(Zero::CreateFunctor(&ExternalSystemInterface::SendAudioEvent, ExternalInterface, 
          Notify_AudioClipping, (void*)nullptr));
    }

    // Update the output volumes
    rmsVolume /= numberOfFrames;
    AddTaskThreaded(Zero::CreateFunctor(&AudioSystemInternal::SetVolumes, this, peakVolume, rmsVolume));

    // If shutting down, wait for volume to ramp down to zero
    if (ShuttingDownThreaded)
    {
      // Volume will interpolate down to zero
      for (unsigned i = 0; i < AudioIO.OutputBufferSizeThreaded; i += AudioIO.OutputChannelsThreaded)
      {
        float volume = VolumeInterpolatorThreaded.NextValue();

        for (unsigned j = 0; j < AudioIO.OutputChannelsThreaded; ++j)
          outputBuffer[i + j] *= volume;
      }

      // If we reached zero volume, it's okay to shut down
      if (VolumeInterpolatorThreaded.Finished())
        return false;
    }

    // Increment buffer number
    AudioIO.FinishedMixingBuffer();

    forRange(TagObject& tag, TagListThreaded.All())
      tag.UpdateCompressorInput();

    // Still running, return true
    return true;
  }

  //************************************************************************************************
  void AudioSystemInternal::AddTask(Zero::Functor* function)
  {
    // Don't add tasks to the list when audio output is stopped
    if (!AudioIO.IsStreamOpen())
    {
      delete function;
      return;
    }

    TasksForMixThread.Write(function);
    return;
  }

  //************************************************************************************************
  void AudioSystemInternal::AddTaskThreaded(Zero::Functor* function)
  {
    TasksForGameThread.Write(function);
  }

  //************************************************************************************************
  void AudioSystemInternal::SetMinVolumeThresholdThreaded(const float volume)
  {
    MinimumVolumeThresholdThreaded = volume;
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
  void AudioSystemInternal::AddAsset(SoundAssetNode* asset)
  {
    AssetList.PushBack(asset);
  }

  //************************************************************************************************
  void AudioSystemInternal::RemoveAsset(SoundAssetNode* asset)
  {
    AssetList.Erase(asset);
  }

  //************************************************************************************************
  void AudioSystemInternal::HandleTasksThreaded()
  {
    int counter(0);
    Zero::Functor* function;
    while (counter < MaxTasksToReadThreaded && TasksForMixThread.Read(function))
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
    VolumeInterpolatorThreaded.SetValues(1.0f, 0.0f, 1000 * gAudioSystem->SystemChannelsThreaded);
  }

  //************************************************************************************************
  void AudioSystemInternal::SetSystemChannelsThreaded(const unsigned channels)
  {
    SystemChannelsThreaded = channels;
    MixBufferSizeThreaded = AudioIO.GetBaseBufferSize() * channels;
    BufferForOutput.Resize(MixBufferSizeThreaded);
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

      if (NodeCount >= MAXNODES)
        return false;
      else
        return true;
    }
    else
    {
      NodeListThreaded.PushBack(node);
      return true;
    }
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
  Audio::InterpolatingObject* AudioSystemInternal::GetInterpolatorThreaded()
  {
    // If we've used all available interpolators, increase the array size
    if ((unsigned)NextInterpolator >= InterpolatorArray.Size())
      InterpolatorArray.Resize(InterpolatorArray.Size() * 2);

    ErrorIf(InterpolatorArray[NextInterpolator].Active == true, 
      "Audio Engine: Trying to use interpolator which is already active");
    ErrorIf(InterpolatorArray[NextInterpolator].Object->Container != &InterpolatorArray[NextInterpolator],
      "Audio Engine: Interpolator container pointer does not match container");

    // Mark as active
    InterpolatorArray[NextInterpolator].Active = true;
    // Return the interpolator and increment the index
    return InterpolatorArray[NextInterpolator++].Object;
  }

  //************************************************************************************************
  void AudioSystemInternal::ReleaseInterpolatorThreaded(InterpolatingObject* object)
  {
    if (!object)
      return;
    
    // Clear the values
    object->SetValues(0.0f, 0.0f, 0.0f);
    // Mark as not active
    object->Container->Active = false;
    // Decrement index
    --NextInterpolator;
    // If the interpolator at the next available index is active, swap this object's data with that one
    if (InterpolatorArray[NextInterpolator].Active)
      InterpolatorArray[NextInterpolator].Swap(*object->Container);

    ErrorIf(NextInterpolator < 0, "Audio Engine: Interpolator index went below 0");
  }

  //************************************************************************************************
  void AudioSystemInternal::SetVolumes(const float peak, const unsigned rms)
  {
    PeakVolumeLastMix = peak;
    RmsVolumeLastMix = Math::Sqrt((float)rms) / (float)((1 << 15) - 1);
  }

  //************************************************************************************************
  void AudioSystemInternal::SetUseHighLatency(const bool useHighLatency)
  {
    Zero::Status status;

    AudioIO.RestartStream(!useHighLatency, status);

    MixBufferSizeThreaded = AudioIO.GetBaseBufferSize() * SystemChannelsThreaded;
    BufferForOutput.Resize(MixBufferSizeThreaded);
  }

  //************************************************************************************************
  void AudioSystemInternal::AddDecodingTask(Zero::Functor* function)
  {
    DecodeLock.Lock();

    // Check if the decoding thread is currently running
    if (!DecodingFinished)
    {
      DecodingList.PushBack(function);
      DecodeLock.Unlock();
    }
    else
    {
      DecodeLock.Unlock();

      DecodingList.PushBack(function);
      DecodingFinished = false;

      // Start a new thread
      DecodeThread.Initialize(StartDecoding, this, "Decoding thread");
      DecodeThread.Resume();
    }
  }

  //************************************************************************************************
  void AudioSystemInternal::DecodeLoopThreaded()
  {
    bool finished(false);
    while (!finished)
    {
      DecodeLock.Lock();
      // If the list is empty, nothing more to do, can close thread
      if (DecodingList.Empty())
      {
        finished = true;
        DecodingFinished = true;
        DecodeLock.Unlock();
        continue;
      }
      // Get all objects in the current list
      Zero::Array<Zero::Functor*> samplesToDecode(DecodingList);
      DecodingList.Clear();
      DecodeLock.Unlock();

      unsigned size = samplesToDecode.Size();
      for (unsigned i = 0; i < size; ++i)
      {
        samplesToDecode[i]->Execute();
        delete samplesToDecode[i];
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
  void AudioFrame::TranslateChannels(const unsigned channels)
  {
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

}

