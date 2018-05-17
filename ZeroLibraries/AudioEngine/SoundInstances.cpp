///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------ Cross Fade Object

  //************************************************************************************************
  AudioFadeObject::AudioFadeObject() :
    mFading(false),
    mFrameIndex(0),
    mStartFrame(0),
    mDefaultFrames(PropertyChangeFrames),
    mCrossFade(false),
    mAsset(nullptr)
  {
    VolumeInterpolator.SetCurve(CurveTypes::Squared);
  }

  //************************************************************************************************
  void AudioFadeObject::StartFade(float startingVolume, unsigned startingIndex, unsigned fadeFrames, 
    SoundAsset* asset, bool crossFade)
  {
    mFading = true;
    mFrameIndex = 0;
    mStartFrame = startingIndex;
    mCrossFade = crossFade;
    mAsset = asset;
    VolumeInterpolator.SetValues(startingVolume, 0.0f, fadeFrames);

    // We can only get a second of samples at a time (will get the rest later)
    if (fadeFrames > SystemSampleRate)
      fadeFrames = SystemSampleRate;

    asset->AppendSamples(&FadeSamples, startingIndex, fadeFrames * asset->GetChannels());
  }

  //************************************************************************************************
  void AudioFadeObject::ApplyFade(float* buffer, unsigned howManyFrames)
  {
    if (!mFading)
      return;

    unsigned assetChannels = mAsset->GetChannels();
    unsigned sampleIndex = mFrameIndex * assetChannels;
    unsigned bufferIndex = 0;

    for (unsigned frame = 0; frame < howManyFrames; ++frame)
    {
      // Check if we need more samples
      if (sampleIndex + assetChannels > FadeSamples.Size())
        GetMoreSamples();

      float volume = VolumeInterpolator.ValueAtIndex(mFrameIndex);

      for (unsigned channel = 0; channel < assetChannels; ++channel, ++sampleIndex, ++bufferIndex)
      {
        // If cross-fading, apply the other volume to the current sample (goes from 0-1)
        if (mCrossFade)
          buffer[bufferIndex] *= 1.0f - volume;

        // Apply the volume to the faded sample and add to current sample (goes from 1-0)
        buffer[bufferIndex] += FadeSamples[sampleIndex] * volume;
      }

      // Advance the index
      ++mFrameIndex;

      // Check if cross-fading is done or if we would run past the end of the data
      if (mFrameIndex >= VolumeInterpolator.GetTotalFrames() || mStartFrame + mFrameIndex
        >= mAsset->GetNumberOfFrames())
      {
        mFading = false;
        return;
      }
    }
  }

  //************************************************************************************************
  void AudioFadeObject::GetMoreSamples()
  {
    // Get another second of samples
    unsigned newFramesToGet = SystemSampleRate;
    // If this would be more than we need, adjust the amount
    if (mFrameIndex + newFramesToGet > VolumeInterpolator.GetTotalFrames())
      newFramesToGet -= mFrameIndex + newFramesToGet - VolumeInterpolator.GetTotalFrames();

    // Add the new samples to the end of the sample array
    mAsset->AppendSamples(&FadeSamples, mStartFrame + mFrameIndex, newFramesToGet * mAsset->GetChannels());
  }
  
  //---------------------------------------------------------------------- Music Notification Object

  //************************************************************************************************
  void MusicNotificationObject::ProcessAndNotify(float currentTime, SoundNode* siblingNode)
  {
    if (mSecondsPerEighth == 0.0f || !siblingNode)
      return;

    float eighths = currentTime / mSecondsPerEighth;
    float beats = currentTime / mSecondsPerBeat;

    // Check for beats
    if ((int)beats > mTotalBeats)
    {
      // There should never be a jump in the time without calling ResetBeats, so we don't need
      // to handle increases of more than one beat
      ++mTotalBeats;
      ++mBeatsCount;

      // Check for new bar
      if (mBeatsCount == mBeatsPerBar)
      {
        mEighthNoteCount = -1;
        mBeatsCount = 0;

        // Send notification
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicBar));
      }

      // Send notification
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
        siblingNode, AudioEventTypes::MusicBeat));
    }

    // Check for eighth notes
    if ((int)eighths > mTotalEighths)
    {
      // There should never be a jump in the time without calling ResetBeats, so we don't need
      // to handle increases of more than one beat
      ++mTotalEighths;
      ++mEighthNoteCount;

      // Send notification for eighth note
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
        siblingNode, AudioEventTypes::MusicEighthNote));

      // Check for other note values

      // Check for quarter note
      if (mEighthNoteCount % 2 == 0)
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicQuarterNote));
      }

      // Check for half note
      if (mEighthNoteCount % 4 == 0)
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicHalfNote));
      }

      // Check for whole note
      if (mEighthNoteCount % 8 == 0)
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicWholeNote));
      }
    }
  }

  //************************************************************************************************
  void MusicNotificationObject::ResetBeats(float currentTime, SoundNode* siblingNode)
  {
    if (mSecondsPerEighth == 0.0f)
      return;

    float timePerMeasure = mSecondsPerBeat * mBeatsPerBar;
    float measuresSoFar = currentTime / timePerMeasure;
    float timeSinceLastBar = currentTime - ((int)measuresSoFar * timePerMeasure);

    // If we are at or very close to the beginning of a measure, reset everything
    if (timeSinceLastBar < 0.0001f)
    {
      mTotalEighths = 0;
      mTotalBeats = 0;
      mBeatsCount = 0;
      mEighthNoteCount = 0;

      if (siblingNode)
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicBar));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicBeat));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicWholeNote));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicEighthNote));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicQuarterNote));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          siblingNode, AudioEventTypes::MusicHalfNote));
      }
    }
    else
    {
      mTotalEighths = (int)(currentTime / mSecondsPerEighth);
      mEighthNoteCount = (int)(timeSinceLastBar / mSecondsPerEighth);

      mTotalBeats = (int)(currentTime / mSecondsPerBeat);
      mBeatsCount = (int)(timeSinceLastBar / mSecondsPerBeat);
    }
  }

  //---------------------------------------------------------------------------- Sound Instance Node

  //************************************************************************************************
  SoundInstanceNode::SoundInstanceNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      SoundAsset* parentAsset, const bool looping, const bool startPaused, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, true, isThreaded), 
    Asset(parentAsset), 
    mVolume(0.8f), 
    mFinished(false), 
    mLooping(looping), 
    mPaused(startPaused), 
    mCurrentTime(0),
    mStartTime(0),
    mEndTime((float)parentAsset->GetNumberOfFrames() / SystemSampleRate),
    mLoopStartTime(0),
    mLoopEndTime((float)parentAsset->GetNumberOfFrames() / SystemSampleRate),
    mLoopTailTime(0),
    mCrossFadeTail(false),
    mNotifyTime(0),
    mCustomNotifySent(false),
    mVirtual(false),
    mPitchFactor(1.0f),
    PausingModifier(nullptr),
    mFrameIndex(0),
    mPitchShifting(false),
    mStopFrameCount(0),
    mStopping(false),
    mPausing(false),
    mInterpolatingVolume(false),
    mStartFrame(0),
    mEndFrame(0),
    mLoopStartFrame(0),
    mLoopEndFrame(0),
    mLoopTailFrames(0),
    mSavedOutputVersion(gAudioSystem->MixVersionNumber - 1)
  {
    if (!Threaded)
    {
      if (parentAsset->AddReference())
      {
        SetSiblingNodes(new SoundInstanceNode(status, name, ID, parentAsset->ThreadedAsset, looping, 
          startPaused, nullptr, true));
      }
      // Couldn't attach to this asset
      else
      {
        status.SetFailed(Zero::String::Format(
          "Unable to play instance from asset %s. Usually caused by trying to play an already playing streaming file.", 
          Asset->mName.c_str()));
        Asset = nullptr;
      }
    }
    else
    {
      mEndFrame = Asset->GetNumberOfFrames() - 1;
      mLoopEndFrame = mEndFrame;

      // Set volume ramp if not paused
      if (!mPaused)
      {
        InstanceVolumeModifier *volumeMod = GetAvailableVolumeMod();
        if (volumeMod)
          volumeMod->Reset(0.0f, 1.0f, PropertyChangeFrames,
            PropertyChangeFrames);

        ResetMusicBeats();
      }
    }
  }

  //************************************************************************************************
  SoundInstanceNode::~SoundInstanceNode()
  {
    // Remove this instance from any associated tags
    forRange(TagObject* tag, TagList.All())
      tag->RemoveInstanceFromLists(this);

    if (!Threaded && Asset)
      Asset->ReleaseReference();
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetPaused()
  {
    return mPaused;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetPaused(bool isPaused)
  {
    if (!Threaded)
    {
      mPaused = isPaused;
      AddTaskForSibling(&SoundInstanceNode::SetPaused, isPaused);
    }
    else
    {
      // If setting to paused and is not currently paused or pausing
      if (isPaused && !mPaused && !mPausing)
      {
        mPausing = true;
        mStopFrameCount = 0;
        mStopFramesToWait = PropertyChangeFrames + 10;
        InstanceVolumeModifier* mod = GetAvailableVolumeMod();
        if (mod)
        {
          mod->Reset(1.0f, 0.0f, PropertyChangeFrames,
            PropertyChangeFrames);
          PausingModifier = mod;
        }
      }
      // If setting to not paused and is currently paused or pausing
      else if (!isPaused && (mPaused || mPausing))
      {
        mPaused = false;
        mPausing = false;
        InstanceVolumeModifier* mod = GetAvailableVolumeMod();
        if (mod)
          mod->Reset(0.0f, 1.0f, PropertyChangeFrames, 
            PropertyChangeFrames);

        if (mCurrentTime == 0)
          ResetMusicBeats();
      }
    }
  }


  //************************************************************************************************
  void SoundInstanceNode::Stop()
  {
    if (!Threaded)
    {
      AddTaskForSibling(&SoundInstanceNode::Stop);
    }
    else
    {
      mStopping = true;
      mStopFrameCount = 0;
      mStopFramesToWait = PropertyChangeFrames + 10;
      InstanceVolumeModifier* mod = GetAvailableVolumeMod();
      if (mod)
        mod->Reset(1.0f, 0.0f, PropertyChangeFrames,
          PropertyChangeFrames);
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetLooping()
  {
    return mLooping;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLooping(const bool loop)
  {
    mLooping = loop;

    if (!Threaded)
      AddTaskForSibling(&SoundInstanceNode::SetLooping, loop);
  }

  //************************************************************************************************
  float SoundInstanceNode::GetStartTime()
  {
    return mStartTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetStartTime(const float startTime)
  {
    if (!Threaded)
    {
      mStartTime = startTime;
      if (mStartTime < 0.0f || mStartTime * SystemSampleRate >= Asset->GetNumberOfFrames())
        mStartTime = 0.0f;

      AddTaskForSibling(&SoundInstanceNode::SetStartTime, mStartTime);
    }
    else
    {
      mStartFrame = (unsigned)(startTime * SystemSampleRate);

      if (mFrameIndex < mStartFrame)
        mFrameIndex = mStartFrame;
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetEndTime()
  {
    return mEndTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetEndTime(const float endTime)
  {
    if (!Threaded)
    {
      mEndTime = endTime;
      if (mEndTime < 0.0f)
        mEndTime = 0.0f;
      else if (mEndTime * SystemSampleRate >= Asset->GetNumberOfFrames())
        mEndTime = (float)Asset->GetNumberOfFrames() / (float)SystemSampleRate;

      AddTaskForSibling(&SoundInstanceNode::SetEndTime, mEndTime);
    }
    else
    {
      mEndFrame = (unsigned)(endTime * SystemSampleRate);
      if (mEndFrame >= Asset->GetNumberOfFrames())
        mEndFrame = Asset->GetNumberOfFrames() - 1;
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopStartTime()
  {
    return mLoopStartTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopStartTime(const float time)
  {
    if (!Threaded)
    {
      mLoopStartTime = time;
      if (mLoopStartTime < 0.0f || mLoopStartTime * SystemSampleRate 
          >= Asset->GetNumberOfFrames())
        mLoopStartTime = 0.0f;

      AddTaskForSibling(&SoundInstanceNode::SetLoopStartTime, mLoopStartTime);
    }
    else
    {
      mLoopStartFrame = (unsigned)(time *SystemSampleRate);
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopEndTime()
  {
    return mLoopEndTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopEndTime(const float time)
  {
    if (!Threaded)
    {
      mLoopEndTime = time;
      if (mLoopEndTime < 0.0f)
        mLoopEndTime = 0.0f;
      else if (mLoopEndTime * SystemSampleRate >= Asset->GetNumberOfFrames())
        mLoopEndTime = (float)Asset->GetNumberOfFrames() / (float)SystemSampleRate;

      AddTaskForSibling(&SoundInstanceNode::SetLoopEndTime, mLoopEndTime);
    }
    else
    {
      mLoopEndFrame = (unsigned)(time * SystemSampleRate);
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopTailTime()
  {
    return mLoopTailTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopTailTime(const float time)
  {
    if (!Threaded)
    {
      mLoopTailTime = time;
      if (mLoopTailTime < 0.0f)
        mLoopTailTime = 0.0f;

      AddTaskForSibling(&SoundInstanceNode::SetLoopTailTime, mLoopTailTime);
    }
    else
    {
      mLoopTailFrames = (unsigned)(time * SystemSampleRate);
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetCrossFadeTail()
  {
    return mCrossFadeTail;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetCrossFadeTail(bool crossFade)
  {
    mCrossFadeTail = crossFade;

    if (!Threaded)
      AddTaskForSibling(&SoundInstanceNode::SetCrossFadeTail, crossFade);
  }

  //************************************************************************************************
  float SoundInstanceNode::GetVolume()
  {
    return mVolume;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetVolume(const float newVolume, float time)
  {
    if (!Threaded)
    {
      // If not interpolating, paused, or the volume is close, set the volume directly
      if (time == 0 || mPaused || IsWithinLimit(newVolume, mVolume, 0.01f))
        mVolume = newVolume;

      AddTaskForSibling(&SoundInstanceNode::SetVolume, newVolume, time);
    }
    else
    {
      // If paused or the volume is close, just set the volume directly
      if (mPaused || IsWithinLimit(newVolume, mVolume, 0.01f))
      {
        mInterpolatingVolume = false;
        mVolume = newVolume;
      }
      else
      {
        mInterpolatingVolume = true;

        // Make sure there's a minimum interpolation time
        if (time < 0.02f)
          time = 0.02f;

        VolumeInterpolator.SetValues(mVolume, newVolume, 
          (unsigned)(time * SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetPitch()
  {
    if (mPitchFactor == 0)
      return 0;
    else
      return 12.0f * Math::Log2(mPitchFactor);
  }

  //************************************************************************************************
  void SoundInstanceNode::SetPitch(const float pitchSemitones, const float time)
  {
    if (!Threaded)
    {
      // If not interpolating, set the pitch value
      if (time == 0)
        mPitchFactor = Math::Pow(2.0f, pitchSemitones / 12.0f);
      AddTaskForSibling(&SoundInstanceNode::SetPitch, pitchSemitones, time);
    }
    else
    {
      // Check for no pitch shift and no interpolation
      if (pitchSemitones == 0 && time == 0)
      {
        mPitchShifting = false;
        Pitch.SetPitchFactor(1.0f, 0.0f);
      }
      else
      {
        mPitchShifting = true;
        Pitch.SetPitchFactor(Math::Pow(2.0f, pitchSemitones / 12.0f), time);
      }
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::IsPlaying()
  {
    return !mFinished;
  }

  //************************************************************************************************
  void SoundInstanceNode::JumpTo(const float seconds)
  {
    if (!Threaded)
    {
      // Can't jump when using a streaming asset
      if (!Asset->GetStreaming())
        AddTaskForSibling(&SoundInstanceNode::JumpTo, seconds);
    }
    else
    {
      // Only need to cross-fade if we're not at the beginning of the file
      if (mFrameIndex > mStartFrame)
        Fade.StartFade(mVolume, mFrameIndex, Fade.mDefaultFrames, Asset, true);

      mFrameIndex = (unsigned)(seconds * SystemSampleRate);
      if (mFrameIndex > mEndFrame)
        mFrameIndex = mEndFrame;
      else if (mFrameIndex < mStartFrame)
        mFrameIndex = mStartFrame;

      mCurrentTime = mFrameIndex * SystemTimeIncrement;
      ResetMusicBeats();
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetTime()
  {
    return (float)mCurrentTime;
  }

  //************************************************************************************************
  float SoundInstanceNode::GetBeatsPerMinute()
  {
    return 60.0f / MusicNotify.mSecondsPerBeat;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetBeatsPerMinute(const float bpm)
  {
    if (bpm <= 0.0f)
    {
      MusicNotify.mSecondsPerBeat = 0.0f;
      MusicNotify.mSecondsPerEighth = 0.0f;
    }
    else
    {
      MusicNotify.mSecondsPerBeat = 60.0f / bpm;
      MusicNotify.mSecondsPerEighth = MusicNotify.mSecondsPerBeat *  MusicNotify.mBeatNoteType / 8.0f;
    }

    if (!Threaded)
      AddTaskForSibling(&SoundInstanceNode::SetBeatsPerMinute, bpm);
  }

  //************************************************************************************************
  void SoundInstanceNode::SetTimeSignature(const int beats, const int noteType)
  {
    MusicNotify.mBeatsPerBar = Math::Max(beats, 0);
    MusicNotify.mBeatNoteType = Math::Max(noteType, 0);

    if (MusicNotify.mBeatsPerBar == 0 || MusicNotify.mBeatNoteType == 0)
    {
      MusicNotify.mSecondsPerEighth = 0.0f;
    }
    else
    {
      MusicNotify.mSecondsPerEighth = MusicNotify.mSecondsPerBeat *  MusicNotify.mBeatNoteType / 8.0f;
    }

    if (!Threaded)
      AddTaskForSibling(&SoundInstanceNode::SetTimeSignature, beats, noteType);
  }

  //************************************************************************************************
  float SoundInstanceNode::GetCustomNotifyTime()
  {
    return mNotifyTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetCustomNotifyTime(const float time)
  {
    mNotifyTime = time;
    mCustomNotifySent = false;

    if (!Threaded)
      AddTaskForSibling(&SoundInstanceNode::SetCustomNotifyTime, time);
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetOutputSamples(BufferType* outputBuffer,
    const unsigned numberOfChannels, ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    // Get the audio output
    bool result = GetOutputForThisMix(outputBuffer, numberOfChannels);

    // If there was valid output, apply changes from any associated tags
    if (result)
    {
      forRange(TagObject* tag, TagList.All())
        tag->ProcessInstance(outputBuffer, numberOfChannels, this);
    }

    return result;
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetOutputForThisMix(BufferType* outputBuffer,
    const unsigned numberOfChannels)
  {
    if (!Threaded)
      return false;

    // Check if we already processed output for this mix version
    if (mSavedOutputVersion == gAudioSystem->MixVersionNumber)
    {
      // Check if the buffer sizes don't match, and there is saved audio data
      if (outputBuffer->Size() != InputSamples.Size() && !InputSamples.Empty())
      {
        // Need to get additional samples
        if (outputBuffer->Size() > InputSamples.Size())
        {
          AddSamplesToBuffer(&InputSamples, (outputBuffer->Size() - InputSamples.Size()) / numberOfChannels,
            numberOfChannels);
        }
        // Need to save samples for next time
        else
        {
          // Add the extra samples to the end of the SavedSamples buffer
          CopyIntoBuffer(&SavedSamples, InputSamples, outputBuffer->Size(), 
            InputSamples.Size() - outputBuffer->Size());

          // Resize the InputSamples buffer to match the output buffer
          InputSamples.Resize(outputBuffer->Size());
        }
      }

      // Copy the saved audio samples to the output buffer
      if (!InputSamples.Empty())
      {
        outputBuffer->Resize(InputSamples.Size());
        memcpy(outputBuffer->Data(), InputSamples.Data(), sizeof(float) * InputSamples.Size());
      }
      // Return true if there was actual audio data and false if there was not
      return !InputSamples.Empty();
    }
    else
    {
      // Set the mix version
      mSavedOutputVersion = gAudioSystem->MixVersionNumber;

      if (mFinished || mPaused)
        return false;

      //if (mVirtual)
      //{
      //  SkipForward(bufferFrames);
      //  return false;
      //}

      // Reset the InputSamples buffer
      InputSamples.Clear();
      // Fill the InputSamples buffer with the needed number of samples
      AddSamplesToBuffer(&InputSamples, outputBuffer->Size() / numberOfChannels, numberOfChannels);

      // Apply modifications
      forRange(InstanceVolumeModifier* modifier, VolumeModList.All())
      {
        if (modifier->Active)
          modifier->ApplyVolume(InputSamples.Data(), InputSamples.Size(), numberOfChannels);
      }

      // Copy from input buffer to output buffer
      ErrorIf(outputBuffer->Size() != InputSamples.Size(), "Buffer sizes do not match in SoundInstance output");
      memcpy(outputBuffer->Data(), InputSamples.Data(), sizeof(float) * outputBuffer->Size());

      if (mPaused && PausingModifier)
      {
        PausingModifier->Active = false;
        PausingModifier = nullptr;
      }

      if (GetSiblingNode())
      {
        // Set the time variable on the non-threaded node
        AddTaskForSiblingThreaded(&SoundInstanceNode::mCurrentTime, mCurrentTime);

        // Update the volume on the non-threaded instance
        if (mInterpolatingVolume)
          AddTaskForSiblingThreaded(&SoundInstanceNode::mVolume, mVolume);

        // Update the pitch on the non-threaded instance
        if (Pitch.Interpolating())
          AddTaskForSiblingThreaded(&SoundInstanceNode::mPitchFactor, Pitch.GetPitchFactor());
      }

      return true;
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::AddSamplesToBuffer(BufferType* buffer, unsigned outputFrames,
    unsigned outputChannels)
  {
    // Check if there are saved samples from last mix
    if (!SavedSamples.Empty())
    {
      // Find out how many samples need to be copied over
      unsigned samplesToCopy = Math::Min(SavedSamples.Size(), outputFrames * outputChannels);
      // Add the saved samples to the end of the buffer
      CopyIntoBuffer(buffer, SavedSamples, 0, samplesToCopy);
      // If there are more saved samples, erase the ones we copied
      if (SavedSamples.Size() > samplesToCopy)
        SavedSamples.Erase(SavedSamples.SubRange(0, samplesToCopy));
      // Otherwise just clear the array
      else
        SavedSamples.Clear();

      // Adjust the number of frames needed
      outputFrames -= samplesToCopy / outputChannels;
      // If we don't need any more frames, just return
      if (outputFrames == 0)
        return;
    }

    unsigned inputChannels = Asset->GetChannels();
    unsigned inputFrames = outputFrames;
    BufferType samples;

    // If pitch shifting, determine number of asset frames we need
    if (mPitchShifting)
    {
      Pitch.CalculateBufferSize(outputFrames * outputChannels, outputChannels);
      inputFrames = Pitch.GetInputFrameCount();
    }

    // Store the starting frame
    unsigned startingFrameIndex = mFrameIndex;
    // Move the frame index forward
    mFrameIndex += inputFrames;

    // Check if we are looping and will reach the loop end frame
    if (mLooping && mFrameIndex >= mLoopEndFrame)
    {
      unsigned sectionFrames = 0;

      if (startingFrameIndex < mLoopEndFrame)
      {
        // Save the number of frames in the first section
        sectionFrames = inputFrames - (mFrameIndex - mLoopEndFrame);
        // Get the samples from the asset
        Asset->AppendSamples(&samples, startingFrameIndex, sectionFrames * inputChannels);
      }

      // Reset back to the loop start frame
      Loop();

      // Save the number of frames in the second section
      sectionFrames = inputFrames - sectionFrames;
      // Get the samples from the asset
      Asset->AppendSamples(&samples, mFrameIndex, sectionFrames * inputChannels);

      // Move frame index forward
      mFrameIndex += sectionFrames;
    }
    // Check if we will reach the end of the audio
    else if (mFrameIndex >= mEndFrame)
    {
      // Get the available samples
      if (startingFrameIndex < mEndFrame)
        Asset->AppendSamples(&samples, startingFrameIndex,
        (inputFrames - (mFrameIndex - mEndFrame)) * inputChannels);

      // Resize the array to full size, setting the rest of the samples to 0
      samples.Resize(inputFrames * inputChannels, 0.0f);

      FinishedCleanUp();
    }
    // Otherwise, no need to adjust anything
    else
    {
      Asset->AppendSamples(&samples, startingFrameIndex, inputFrames * inputChannels);
    }

    // Apply fading if needed
    Fade.ApplyFade(samples.Data(), inputFrames);

    // Check if the output channels are different than the audio data
    if (inputChannels != outputChannels)
      TranslateChannels(&samples, inputFrames, inputChannels, outputChannels);

    // If pitch shifting, interpolate the samples into the buffer
    if (mPitchShifting)
    {
      // Save the interpolation state
      bool interpolating = Pitch.Interpolating();

      // Pitch shifts the samples into a temporary buffer
      BufferType pitchShiftedSamples(outputFrames * outputChannels);
      Pitch.ProcessBuffer(&samples, &pitchShiftedSamples);

      // Move the samples back into the original buffer
      samples.Swap(pitchShiftedSamples);

      // If interpolating the pitch (whether or not it finished) update the non-threaded 
      // object with the pitch factor
      if (interpolating && GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&PitchChangeHandler::SetPitchFactor,
          &((SoundInstanceNode*)GetSiblingNode())->Pitch, Pitch.GetPitchFactor(), 0.0f));
    }

    // Check if we need to apply a volume change
    if (mInterpolatingVolume || !IsWithinLimit(mVolume, 1.0f, 0.01f))
    {
      for (unsigned i = 0; i < samples.Size(); i += outputChannels)
      {
        // If interpolating volume, get new value
        if (mInterpolatingVolume)
        {
          mVolume = VolumeInterpolator.NextValue();

          mInterpolatingVolume = !VolumeInterpolator.Finished(GetSiblingNode());

          // Update the non-threaded instance with the final volume
          if (!mInterpolatingVolume)
            AddTaskForSiblingThreaded(&SoundInstanceNode::mVolume, mVolume);
        }

        // Adjust volume for all samples on this frame
        for (unsigned j = 0; j < outputChannels; ++j)
          samples[i + j] *= mVolume;
      }
    }

    // Add the samples to the end of the supplied buffer
    CopyIntoBuffer(buffer, samples, 0, samples.Size());

    // Check for pausing or stopping 
    if (mPausing || mStopping)
    {
      mStopFrameCount += inputFrames;

      if (mStopFrameCount >= mStopFramesToWait)
      {
        // If finished pausing, set variables
        if (mPausing)
        {
          mPaused = true;
          mPausing = false;
        }
        // If finished stopping, handle notifications and clean up
        else
          FinishedCleanUp();
      }
    }

    // Advance time and handle music notifications
    mCurrentTime = mFrameIndex * SystemTimeIncrement;
    MusicNotifications();
  }

  //************************************************************************************************
  float SoundInstanceNode::GetAttenuationThisMix()
  {
    float volume = 0.0f;
    forRange(SoundNode* node, GetOutputs()->All())
      volume += node->GetVolumeChangeFromOutputs();

    return volume;
  }

  //************************************************************************************************
  void SoundInstanceNode::Loop()
  {
    // Handle fading if we're not at the end of the audio
    if (mLoopEndFrame < mEndFrame)
    {
      // Use the default cross fade size if streaming or if the variable hasn't been set
      unsigned fadeSize = Fade.mDefaultFrames;
      if (mLoopTailFrames > 0 && !Asset->GetStreaming())
        fadeSize = mLoopTailFrames;
      // Make sure we don't go past the end of the file
      if (mLoopEndFrame + fadeSize > mEndFrame)
        fadeSize -= mLoopEndFrame + fadeSize - mEndFrame;
      // Start the cross-fade
      Fade.StartFade(mVolume, mLoopEndFrame, fadeSize, Asset, mCrossFadeTail);
    }

    AddTaskForSiblingThreaded(&SoundNode::SendEventToExternalData, AudioEventTypes::InstanceLooped);

    // Reset variables
    mFrameIndex = mLoopStartFrame;
    mCurrentTime = mFrameIndex * SystemTimeIncrement;
    ResetMusicBeats();

    // If streaming, reset to the beginning of the file
    if (Asset->GetStreaming())
      Asset->ResetStreamingFile();
  }

  //************************************************************************************************
  void SoundInstanceNode::TranslateChannels(BufferType* inputSamples, const unsigned inputFrames,
    const unsigned inputChannels, const unsigned outputChannels)
  {
    // Temporary array for channel translation
    BufferType adjustedSamples(inputFrames * outputChannels);

    // Step through each frame of audio data
    for (unsigned frame = 0, inputIndex = 0, outputIndex = 0; frame < inputFrames;
      ++frame, inputIndex += inputChannels, outputIndex += outputChannels)
    {
      // Create the AudioFrame object
      AudioFrame thisFrame(inputSamples->Data() + inputIndex, inputChannels);
      // Copy the translated samples into the other array
      memcpy(adjustedSamples.Data() + outputIndex, thisFrame.GetSamples(outputChannels), 
        sizeof(float) * outputChannels);
    }

    // Move the translated data into the other array
    inputSamples->Swap(adjustedSamples);
  }

  //************************************************************************************************
  void SoundInstanceNode::FinishedCleanUp()
  {
    if (mFinished)
      return;

    mFinished = true;

    if (Threaded && GetSiblingNode())
    {
      // Send notification that this instance is finished
      AddTaskForSiblingThreaded(&SoundNode::SendEventToExternalData, AudioEventTypes::InstanceFinished);

      // Call the non-threaded versions of the clean-up functions
      AddTaskForSiblingThreaded(&SoundInstanceNode::FinishedCleanUp);
      AddTaskForSiblingThreaded(&SoundInstanceNode::RemoveFromAllTags);
    }
    // If not threaded and no external interface, can delete
    else if (!Threaded && !GetExternalInterface())
    {
      DisconnectOnlyThis();
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::BelowMinimumVolume(unsigned frames)
  {
    if (!Threaded)
      return false;

    // Determine overall volume at the beginning and end of the mix
    float volume1(mVolume);
    float volume2(mVolume);

    // If interpolating volume, get the volume at the end of the mix
    if (mInterpolatingVolume)
      volume2 = VolumeInterpolator.ValueAtIndex(mFrameIndex + frames);

    // Adjust with all volume modifiers
    forRange(InstanceVolumeModifier* modifier, VolumeModList.All())
    {
      if (modifier->Active)
      {
        volume1 *= modifier->GetCurrentVolume();
        volume2 *= modifier->GetFutureVolume(frames);
      }
    }

    // Return true if both volumes lower than minimum volume threshold
    if (volume1 < gAudioSystem->MinimumVolumeThresholdThreaded 
      && volume2 < gAudioSystem->MinimumVolumeThresholdThreaded)
      return true;
    else
      return false;
  }

  //************************************************************************************************
  InstanceVolumeModifier* SoundInstanceNode::GetAvailableVolumeMod()
  {
    if (!Threaded)
      return nullptr;

    for (unsigned i = 0; i < VolumeModList.Size(); ++i)
    {
      if (!VolumeModList[i]->Active)
      {
        VolumeModList[i]->Reset(1.0f, 1.0f, (unsigned)0, (unsigned)0);
        return VolumeModList[i];
      }
    }

    VolumeModList.PushBack(new InstanceVolumeModifier);
    return VolumeModList.Back();
  }

  //************************************************************************************************
  void SoundInstanceNode::RemoveFromAllTags()
  {
    if (!Threaded)
    {
      while (!TagList.Empty())
        TagList.Back()->RemoveInstance(this);
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::MusicNotifications()
  {
    if (!Threaded)
      return;

    // If we are looping and close to the loop end, don't do notifications
    if (mLooping && mLoopEndFrame - mFrameIndex < 100)
      return;

    // If there is a custom notification time set and we've hit that time
    if (mNotifyTime > 0 && !mCustomNotifySent && mCurrentTime >= mNotifyTime)
    {
      mCustomNotifySent = true;

      // Send notification
      AddTaskForSiblingThreaded(&SoundNode::SendEventToExternalData, AudioEventTypes::MusicCustomTime);
    }

    MusicNotify.ProcessAndNotify((float)mCurrentTime, GetSiblingNode());
  }

  //************************************************************************************************
  void SoundInstanceNode::ResetMusicBeats()
  {
    if (!Threaded)
      return;

    MusicNotify.ResetBeats((float)mCurrentTime, GetSiblingNode());
  }

  //************************************************************************************************
  void SoundInstanceNode::DisconnectThisAndAllInputs()
  {
    if (Threaded)
      return;

    SoundNode::DisconnectThisAndAllInputs();

    AddTaskForSibling(&SoundInstanceNode::FinishedCleanUp);
  }
}