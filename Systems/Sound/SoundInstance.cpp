///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

namespace Events
{

DefineEvent(SoundStopped);
DefineEvent(SoundLooped);
DefineEvent(MusicBeat);
DefineEvent(MusicBar);
DefineEvent(MusicEighthNote);
DefineEvent(MusicQuarterNote);
DefineEvent(MusicHalfNote);
DefineEvent(MusicWholeNote);
DefineEvent(MusicCustomTime);

DefineEvent(SoundCuePrePlay);
DefineEvent(SoundCuePostPlay);

} // namespace Events

//----------------------------------------------------------------------------- Sound Instance Event

//**************************************************************************************************
ZilchDefineType(SoundInstanceEvent, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetter(SoundInstance);
}

//-------------------------------------------------------------------------------- Cross Fade Object

//**************************************************************************************************
AudioFadeObject::AudioFadeObject() :
  mFading(false),
  mFrameIndex(0),
  mStartFrame(0),
  mDefaultFrames(cPropertyChangeFrames),
  mCrossFade(false),
  mAsset(nullptr),
  mInstanceID(0)
{
  VolumeInterpolator.SetCurve(FalloffCurveType::Squared);
}

//**************************************************************************************************
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
  if (fadeFrames > cSystemSampleRate)
    fadeFrames = cSystemSampleRate;

  asset->AppendSamplesThreaded(&FadeSamples, startingIndex, fadeFrames * asset->mChannels, mInstanceID);
}

//**************************************************************************************************
void AudioFadeObject::ApplyFade(float* buffer, unsigned howManyFrames)
{
  if (!mFading)
    return;

  unsigned assetChannels = mAsset->mChannels;
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
      >= mAsset->mFrameCount)
    {
      mFading = false;
      return;
    }
  }
}

//**************************************************************************************************
void AudioFadeObject::GetMoreSamples()
{
  // Get another second of samples
  unsigned newFramesToGet = cSystemSampleRate;
  // If this would be more than we need, adjust the amount
  if (mFrameIndex + newFramesToGet > VolumeInterpolator.GetTotalFrames())
    newFramesToGet -= mFrameIndex + newFramesToGet - VolumeInterpolator.GetTotalFrames();

  // Add the new samples to the end of the sample array
  mAsset->AppendSamplesThreaded(&FadeSamples, mStartFrame + mFrameIndex, newFramesToGet * 
    mAsset->mChannels, mInstanceID);
}

//------------------------------------------------------------------------ Music Notification Object

//**************************************************************************************************
void MusicNotificationObject::ProcessAndNotify(float currentTime, SoundInstance* instance)
{
  if (mSecondsPerEighth == 0.0f)
    return;

  float eighths = currentTime / mSecondsPerEighth;
  float beats = currentTime / mSecondsPerBeat.Get(AudioThreads::MixThread);

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
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
        instance, Events::MusicBar), instance);
    }

    // Send notification
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicBeat), instance);
  }

  // Check for eighth notes
  if ((int)eighths > mTotalEighths)
  {
    // There should never be a jump in the time without calling ResetBeats, so we don't need
    // to handle increases of more than one beat
    ++mTotalEighths;
    ++mEighthNoteCount;

    // Send notification for eighth note
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicEighthNote), instance);

    // Check for other note values

    // Check for quarter note
    if (mEighthNoteCount % 2 == 0)
    {
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
        instance, Events::MusicQuarterNote), instance);
    }

    // Check for half note
    if (mEighthNoteCount % 4 == 0)
    {
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
        instance, Events::MusicHalfNote), instance);
    }

    // Check for whole note
    if (mEighthNoteCount % 8 == 0)
    {
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
        instance, Events::MusicWholeNote), instance);
    }
  }
}

//************************************************************************************************
void MusicNotificationObject::ResetBeats(float currentTime, SoundInstance* instance)
{
  if (mSecondsPerEighth == 0.0f)
    return;

  float timePerMeasure = mSecondsPerBeat.Get(AudioThreads::MixThread) * mBeatsPerBar;
  float measuresSoFar = currentTime / timePerMeasure;
  float timeSinceLastBar = currentTime - ((int)measuresSoFar * timePerMeasure);

  // If we are at or very close to the beginning of a measure, reset everything
  if (timeSinceLastBar < 0.0001f)
  {
    mTotalEighths = 0;
    mTotalBeats = 0;
    mBeatsCount = 0;
    mEighthNoteCount = 0;

    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicBar), instance);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicBeat), instance);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicWholeNote), instance);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicEighthNote), instance);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicQuarterNote), instance);
    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      instance, Events::MusicHalfNote), instance);
  }
  else
  {
    mTotalEighths = (int)(currentTime / mSecondsPerEighth);
    mEighthNoteCount = (int)(timeSinceLastBar / mSecondsPerEighth);

    mTotalBeats = (int)(currentTime / mSecondsPerBeat.Get(AudioThreads::MixThread));
    mBeatsCount = (int)(timeSinceLastBar / mSecondsPerBeat.Get(AudioThreads::MixThread));
  }
}

//----------------------------------------------------------------------------------- Sound Instance

//**************************************************************************************************
ZilchDefineType(SoundInstance, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetter(Volume);
  ZilchBindGetterSetter(Decibels);
  ZilchBindGetterSetter(Pitch);
  ZilchBindGetterSetter(Semitones);
  ZilchBindGetter(IsPlaying);
  ZilchBindGetter(SoundNode);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindGetterSetter(Paused);
  ZilchBindMethod(Stop);
  ZilchBindGetterSetter(Looping);
  ZilchBindGetterSetter(Time);
  ZilchBindGetter(FileLength);
  ZilchBindGetterSetter(EndTime);
  ZilchBindGetterSetter(LoopStartTime);
  ZilchBindGetterSetter(LoopEndTime);
  ZilchBindGetterSetter(LoopTailTime);
  ZilchBindGetterSetter(CrossFadeLoopTail);
  ZilchBindGetterSetter(CustomEventTime);
  ZilchBindGetter(SoundName);

  ZeroBindEvent(Events::SoundLooped, SoundInstanceEvent);
  ZeroBindEvent(Events::SoundStopped, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBeat, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicBar, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicEighthNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicQuarterNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicHalfNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicWholeNote, SoundInstanceEvent);
  ZeroBindEvent(Events::MusicCustomTime, SoundInstanceEvent);
}

//**************************************************************************************************
SoundInstance::SoundInstance(Status& status, SoundSpace* space, SoundAsset* asset, float volume,
  float pitch) :
  SimpleCollapseNode("SoundInstance", Z::gSound->mCounter++, false, true),
  mAssetObject(asset),
  mSpace(space),
  mCurrentTime(0.0),
  mLooping(cFalse),
  mPaused(cTrue),
  mVolume(Math::Max(volume, 0.0f)),
  mFinished(cFalse),
  mEndTime(asset->mFileLength),
  mLoopStartTime(0.0f),
  mLoopEndTime(asset->mFileLength),
  mLoopTailTime(0.0f),
  mCrossFadeTail(cFalse),
  mNotifyTime(0.0f),
  mCustomNotifySent(false),
  mPitchSemitones(0.0f),
  mFrameIndexThreaded(0),
  mPausingThreaded(false),
  mStoppingThreaded(false),
  mStopFrameCountThreaded(0),
  mStopFramesToWaitThreaded(0),
  mInterpolatingVolumeThreaded(false),
  mPitchShiftingThreaded(false),
  mEndFrameThreaded(asset->mFrameCount),
  mLoopStartFrameThreaded(0),
  mLoopEndFrameThreaded(asset->mFrameCount),
  mLoopTailFramesThreaded(0),
  PausingModifierThreaded(nullptr),
  mSavedOutputVersionThreaded(Z::gSound->Mixer.mMixVersionThreaded - 1)
{
  Fade.mInstanceID = cNodeID;

  mAssetObject->AddInstance(cNodeID);

  // Set the pitch if necessary
  if (pitch != 0.0f)
  {
    mPitchSemitones.SetDirectly(PitchToSemitones(pitch));
    Pitch.SetPitchFactor(Math::Pow(2.0f, mPitchSemitones.Get(AudioThreads::MainThread) / 12.0f), 0.0f);
    mPitchShiftingThreaded = true;
  }
}

//**************************************************************************************************
SoundInstance::~SoundInstance()
{
  
}

//**************************************************************************************************
float SoundInstance::GetVolume()
{
  return mVolume.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundInstance::SetVolume(float newVolume)
{
  InterpolateVolume(newVolume, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateVolume(float newVolume, float interpolationTime)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetVolumeThreaded, this,
    Math::Clamp(newVolume, 0.0f, cMaxVolumeValue), Math::Max(interpolationTime, 0.0f)), this);
}

//**************************************************************************************************
float SoundInstance::GetDecibels()
{
  return VolumeToDecibels(mVolume.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void SoundInstance::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateDecibels(float decibels, float interpolationTime)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetVolumeThreaded, this,
    Math::Clamp(DecibelsToVolume(decibels), 0.0f, cMaxVolumeValue), 
    Math::Max(interpolationTime, 0.0f)), this);
}

//**************************************************************************************************
float SoundInstance::GetPitch()
{
  return SemitonesToPitch(mPitchSemitones.Get(AudioThreads::MainThread));
}

//**************************************************************************************************
void SoundInstance::SetPitch(float newPitch)
{
  InterpolatePitch(newPitch, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolatePitch(float newPitch, float interpolationTime)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetPitchThreaded, this,
    Math::Clamp(newPitch, cMinPitchValue, cMaxPitchValue), Math::Max(interpolationTime, 0.0f)), this);
}

//**************************************************************************************************
float SoundInstance::GetSemitones()
{
  return mPitchSemitones.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundInstance::SetSemitones(float newSemitones)
{
  InterpolateSemitones(newSemitones, 0.0f);
}

//**************************************************************************************************
void SoundInstance::InterpolateSemitones(float newSemitones, float interpolationTime)
{
  if (interpolationTime == 0.0f)
    mPitchSemitones.Set(newSemitones, AudioThreads::MainThread);

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetPitchThreaded, this, newSemitones,
    interpolationTime), this);
}

//**************************************************************************************************
bool SoundInstance::GetPaused()
{
  return mPaused.Get() == cTrue;
}

//**************************************************************************************************
void SoundInstance::SetPaused(bool pause)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetPausedThreaded, this, pause), this);
}

//**************************************************************************************************
void SoundInstance::Stop()
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::StopThreaded, this), this);
}

//**************************************************************************************************
bool SoundInstance::GetIsPlaying()
{
  return mFinished.Get() == cFalse;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundInstance::GetSoundNode()
{
  // TODO deprecate this

  return this;
}

//**************************************************************************************************
bool SoundInstance::GetLooping()
{
  return mLooping.Get() == cTrue;
}

//**************************************************************************************************
void SoundInstance::SetLooping(bool loop)
{
  if (loop)
    mLooping.Set(cTrue);
  else
    mLooping.Set(cFalse);
}

//**************************************************************************************************
float SoundInstance::GetTime()
{
  return (float)mCurrentTime.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundInstance::SetTime(float seconds)
{
  // Make sure the asset isn't streaming
  if (!mAssetObject->mStreaming)
    Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetTimeThreaded, this, seconds), this);
  else
    DoNotifyWarning("Time Set on Streaming Sound", "You cannot set the time on a SoundInstance created from a streaming Sound resource.");
}

//**************************************************************************************************
float SoundInstance::GetFileLength()
{
  if (mAssetObject)
    return mAssetObject->mFileLength;
  else
    return 0.0f;
}

//**************************************************************************************************
float SoundInstance::GetEndTime()
{
  return mEndTime;
}

//**************************************************************************************************
void SoundInstance::SetEndTime(float seconds)
{
  if (!mAssetObject)
    return;

  mEndTime = seconds;
  int frame = (int)(mEndTime * cSystemSampleRate);

  // If the end time is at zero or past the end of the file, set it to the length of the file
  if (mEndTime <= 0.0f || mEndTime >= mAssetObject->mFileLength)
  {
    mEndTime = mAssetObject->mFileLength;
    frame = mAssetObject->mFrameCount - 1;
  }

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::mEndFrameThreaded, this, frame), this);
}

//**************************************************************************************************
float SoundInstance::GetLoopStartTime()
{
  return mLoopStartTime;
}

//**************************************************************************************************
void SoundInstance::SetLoopStartTime(float seconds)
{
  if (!mAssetObject)
    return;

  mLoopStartTime = seconds;
  int frame = (int)(seconds * cSystemSampleRate);

  // If the loop start time is negative or past the end of the file, set it to zero
  if (mLoopStartTime < 0.0f || mLoopStartTime >= mAssetObject->mFileLength)
  {
    mLoopStartTime = 0.0f;
    frame = 0;
  }

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::mLoopStartFrameThreaded, this, frame), this);
}

//**************************************************************************************************
float SoundInstance::GetLoopEndTime()
{
  return mLoopEndTime;
}

//**************************************************************************************************
void SoundInstance::SetLoopEndTime(float seconds)
{
  if (!mAssetObject)
    return;

  mLoopEndTime = seconds;
  int frame = (int)(seconds * cSystemSampleRate);

  // If the loop end time is negative or past the end of the file, set it to the length of the file
  if (mLoopEndTime <= 0.0f || mLoopEndTime >= mAssetObject->mFileLength)
  {
    mLoopEndTime = mAssetObject->mFileLength;
    frame = mAssetObject->mFrameCount;
  }

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::mLoopEndFrameThreaded, this, frame), this);
}

//**************************************************************************************************
float SoundInstance::GetLoopTailTime()
{
  return mLoopTailTime;
}

//**************************************************************************************************
void SoundInstance::SetLoopTailTime(float seconds)
{
  mLoopTailTime = Math::Clamp(seconds, 0.0f, cMaxLoopTailTime);

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::mLoopTailFramesThreaded, this, 
    (int)(mLoopTailTime * cSystemSampleRate)), this);
}

//**************************************************************************************************
bool SoundInstance::GetCrossFadeLoopTail()
{
  return mCrossFadeTail.Get() == cTrue;
}

//**************************************************************************************************
void SoundInstance::SetCrossFadeLoopTail(bool crossFade)
{
  if (crossFade)
    mCrossFadeTail.Set(cTrue);
  else
    mCrossFadeTail.Set(cFalse);
}

//**************************************************************************************************
float SoundInstance::GetBeatsPerMinute()
{
  return 60.0f / MusicNotify.mSecondsPerBeat.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundInstance::SetBeatsPerMinute(float beats)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetBeatsPerMinuteThreaded, this, beats), this);
}

//**************************************************************************************************
void SoundInstance::SetTimeSignature(float beats, float noteType)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::SetTimeSignatureThreaded, this, beats, 
    noteType), this);
}

//**************************************************************************************************
float SoundInstance::GetCustomEventTime()
{
  return mNotifyTime.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundInstance::SetCustomEventTime(float seconds)
{
  mNotifyTime.Set(seconds, AudioThreads::MainThread);
  mCustomNotifySent.Set(false, AudioThreads::MainThread);
}

//**************************************************************************************************
String SoundInstance::GetSoundName()
{
  if (mAssetObject)
    return mAssetObject->mName;
  else
    return "";
}

//**************************************************************************************************
void SoundInstance::Play(bool loop, SoundNode* outputNode, bool startPaused)
{
  SetLooping(loop);

  // If there is an output node, add the instance as input
  if (outputNode)
    outputNode->AddInputNode(this);
  // If there is no output node but there is a SoundSpace, add to direct output of space
  else if (mSpace)
    mSpace->GetInputNode()->AddInputNode(this);

  if (!startPaused)
    SetPaused(false);
}

//**************************************************************************************************
InstanceVolumeModifier* SoundInstance::GetAvailableVolumeModThreaded()
{
  forRange(InstanceVolumeModifier* modifier, VolumeModListThreaded.All())
  {
    if (!modifier->Active)
    {
      modifier->Reset(1.0f, 1.0f, 0u, 0u);
      return modifier;
    }
  }

  VolumeModListThreaded.PushBack(new InstanceVolumeModifier);
  return VolumeModListThreaded.Back();
}

//**************************************************************************************************
bool SoundInstance::GetOutputForThisMixThreaded(BufferType* outputBuffer, const unsigned numberOfChannels)
{
  // Check if we already processed output for this mix version
  if (mSavedOutputVersionThreaded == Z::gSound->Mixer.mMixVersionThreaded)
  {
    // Check if the buffer sizes don't match, and there is saved audio data
    if (outputBuffer->Size() != mInputSamplesThreaded.Size() && !mInputSamplesThreaded.Empty())
    {
      // Need to get additional samples
      if (outputBuffer->Size() > mInputSamplesThreaded.Size())
      {
        AddSamplesToBufferThreaded(&mInputSamplesThreaded, (outputBuffer->Size() 
          - mInputSamplesThreaded.Size()) / numberOfChannels, numberOfChannels);
      }
      // Need to save samples for next time
      else
      {
        // Add the extra samples to the end of the SavedSamples buffer
        AppendToBuffer(&SavedSamplesThreaded, mInputSamplesThreaded, outputBuffer->Size(),
          mInputSamplesThreaded.Size() - outputBuffer->Size());

        // Resize the InputSamples buffer to match the output buffer
        mInputSamplesThreaded.Resize(outputBuffer->Size());
      }
    }

    // Copy the saved audio samples to the output buffer
    if (!mInputSamplesThreaded.Empty())
    {
      outputBuffer->Resize(mInputSamplesThreaded.Size());
      memcpy(outputBuffer->Data(), mInputSamplesThreaded.Data(), sizeof(float) * mInputSamplesThreaded.Size());
    }
    // Return true if there was actual audio data and false if there was not
    return !mInputSamplesThreaded.Empty();
  }
  else
  {
    // Set the mix version
    mSavedOutputVersionThreaded = Z::gSound->Mixer.mMixVersionThreaded;

    if (!mAssetObject)
      mFinished.Set(cTrue);

    if (mFinished.Get() == cTrue || mPaused.Get() == cTrue)
      return false;

    //if (mVirtual)
    //{
    //  SkipForward(bufferFrames);
    //  return false;
    //}

    // Reset the InputSamples buffer
    mInputSamplesThreaded.Clear();
    // Fill the InputSamples buffer with the needed number of samples
    AddSamplesToBufferThreaded(&mInputSamplesThreaded, outputBuffer->Size() / numberOfChannels, 
      numberOfChannels);

    // Apply modifications
    forRange(InstanceVolumeModifier* modifier, VolumeModListThreaded.All())
    {
      if (modifier->Active)
        modifier->ApplyVolume(mInputSamplesThreaded.Data(), mInputSamplesThreaded.Size(), numberOfChannels);
    }

    // Copy from input buffer to output buffer
    ErrorIf(outputBuffer->Size() != mInputSamplesThreaded.Size(), 
      "Buffer sizes do not match in SoundInstance output");
    memcpy(outputBuffer->Data(), mInputSamplesThreaded.Data(), sizeof(float) * outputBuffer->Size());

    if (mPaused.Get() == cTrue && PausingModifierThreaded)
    {
      PausingModifierThreaded->Active = false;
      PausingModifierThreaded = nullptr;
    }

    return true;
  }
}

//**************************************************************************************************
float SoundInstance::GetAttenuationThisMixThreaded()
{
  float volume = 0.0f;
  forRange(HandleOf<SoundNode> node, GetOutputs(AudioThreads::MixThread)->All())
    volume += node->GetVolumeChangeFromOutputsThreaded();

  return volume;
}

//**************************************************************************************************
void SoundInstance::DispatchInstanceEventFromMixThread(const String eventID)
{
  SoundInstanceEvent event(this);
  DispatchEvent(eventID, &event);
}

//**************************************************************************************************
bool SoundInstance::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
  ListenerNode* listener, const bool firstRequest)
{
  // Get the audio output
  bool result = GetOutputForThisMixThreaded(outputBuffer, numberOfChannels);

  // If there was valid output, apply changes from any associated tags
  if (result)
  {
    forRange(TagObject* tag, TagListThreaded.All())
      tag->ProcessInstanceThreaded(outputBuffer, numberOfChannels, this);
  }

  return result;
}

//**************************************************************************************************
void SoundInstance::AddSamplesToBufferThreaded(BufferType* buffer, unsigned outputFrames, 
  unsigned outputChannels)
{
  // Check if there are saved samples from last mix
  if (!SavedSamplesThreaded.Empty())
  {
    // Find out how many samples need to be copied over
    unsigned samplesToCopy = Math::Min(SavedSamplesThreaded.Size(), outputFrames * outputChannels);
    // Add the saved samples to the end of the buffer
    AppendToBuffer(buffer, SavedSamplesThreaded, 0, samplesToCopy);
    // If there are more saved samples, erase the ones we copied
    if (SavedSamplesThreaded.Size() > samplesToCopy)
      SavedSamplesThreaded.Erase(SavedSamplesThreaded.SubRange(0, samplesToCopy));
    // Otherwise just clear the array
    else
      SavedSamplesThreaded.Clear();

    // Adjust the number of frames needed
    outputFrames -= samplesToCopy / outputChannels;
    // If we don't need any more frames, just return
    if (outputFrames == 0)
      return;
  }

  unsigned inputChannels = mAssetObject->mChannels;
  unsigned inputFrames = outputFrames;
  BufferType samples;

  // If pitch shifting, determine number of asset frames we need
  if (mPitchShiftingThreaded)
  {
    Pitch.CalculateBufferSize(outputFrames * outputChannels, outputChannels);
    inputFrames = Pitch.GetInputFrameCount();
  }

  // Store the starting frame
  int startingFrameIndex = mFrameIndexThreaded;
  // Move the frame index forward
  mFrameIndexThreaded += inputFrames;

  // Check if we are looping and will reach the loop end frame
  if (mLooping.Get() == cTrue && (mFrameIndexThreaded >= mLoopEndFrameThreaded || 
    mFrameIndexThreaded >= mEndFrameThreaded))
  {
    unsigned sectionFrames = 0;

    if (startingFrameIndex < mLoopEndFrameThreaded)
    {
      // Save the number of frames in the first section
      sectionFrames = inputFrames - (mFrameIndexThreaded - Math::Min(mLoopEndFrameThreaded, mEndFrameThreaded));
      // Get the samples from the asset
      mAssetObject->AppendSamplesThreaded(&samples, startingFrameIndex, sectionFrames * inputChannels, 
        cNodeID);
    }

    // Reset back to the loop start frame
    LoopThreaded();

    // Save the number of frames in the second section
    sectionFrames = inputFrames - sectionFrames;
    // Get the samples from the asset
    mAssetObject->AppendSamplesThreaded(&samples, mFrameIndexThreaded, sectionFrames * inputChannels, cNodeID);

    // Move frame index forward
    mFrameIndexThreaded += sectionFrames;
  }
  // Check if we will reach the end of the audio
  else if (mFrameIndexThreaded >= mEndFrameThreaded)
  {
    // Get the available samples
    if (startingFrameIndex < mEndFrameThreaded)
      mAssetObject->AppendSamplesThreaded(&samples, startingFrameIndex,
      (inputFrames - (mFrameIndexThreaded - mEndFrameThreaded)) * inputChannels, cNodeID);

    // Resize the array to full size, setting the rest of the samples to 0
    samples.Resize(inputFrames * inputChannels, 0.0f);

    FinishedCleanUpThreaded();
  }
  // Otherwise, no need to adjust anything
  else
  {
    mAssetObject->AppendSamplesThreaded(&samples, startingFrameIndex, inputFrames * inputChannels, cNodeID);
  }

  // Apply fading if needed
  Fade.ApplyFade(samples.Data(), inputFrames);

  // Check if the output channels are different than the audio data
  if (inputChannels != outputChannels)
    TranslateChannelsThreaded(&samples, inputFrames, inputChannels, outputChannels);

  // If pitch shifting, interpolate the samples into the buffer
  if (mPitchShiftingThreaded)
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
    if (interpolating)
      mPitchSemitones.Set(12.0f * Math::Log2(Pitch.GetPitchFactor()), AudioThreads::MixThread);
  }

  // Check if we need to apply a volume change
  if (mInterpolatingVolumeThreaded || !IsWithinLimit(mVolume.Get(AudioThreads::MixThread), 1.0f, 0.01f))
  {
    float volume = mVolume.Get(AudioThreads::MixThread);
    BufferRange sampleRange = samples.All();
    while (!sampleRange.Empty())
    {
      // If interpolating volume, get new value
      if (mInterpolatingVolumeThreaded)
      {
       volume = VolumeInterpolatorThreaded.NextValue(), AudioThreads::MixThread;

        mInterpolatingVolumeThreaded = !VolumeInterpolatorThreaded.Finished();

        if (!mInterpolatingVolumeThreaded)
        {
          mVolume.Set(volume, AudioThreads::MixThread);

          Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchEventFromMixThread,
            (SoundNode*)this, Events::AudioInterpolationDone), this);
        }
      }

      // Adjust volume for all samples on this frame
      for (unsigned j = 0; j < outputChannels; ++j, sampleRange.PopFront())
        sampleRange.Front() *= volume;
    }
  }

  // Add the samples to the end of the supplied buffer
  AppendToBuffer(buffer, samples, 0, samples.Size());

  // Check for pausing or stopping 
  if (mPausingThreaded || mStoppingThreaded)
  {
    mStopFrameCountThreaded += inputFrames;

    if (mStopFrameCountThreaded >= mStopFramesToWaitThreaded)
    {
      // If finished pausing, set variables
      if (mPausingThreaded)
      {
        mPaused.Set(cTrue);
        mPausingThreaded = false;
      }
      // If finished stopping, handle notifications and clean up
      else
        FinishedCleanUpThreaded();
    }
  }

  // Advance time and handle music notifications
  mCurrentTime.Set(mFrameIndexThreaded * cSystemTimeIncrement, AudioThreads::MixThread);
  MusicNotificationsThreaded();
}

//**************************************************************************************************
void SoundInstance::LoopThreaded()
{
  // Handle fading if we're not at the end of the audio
  if (mLoopEndFrameThreaded < mEndFrameThreaded)
  {
    // Use the default cross fade size if streaming or if the variable hasn't been set
    int fadeSize = (int)Fade.mDefaultFrames;
    if (mLoopTailFramesThreaded > 0 && !mAssetObject->mStreaming)
      fadeSize = mLoopTailFramesThreaded;
    // Make sure we don't go past the end of the file
    if (mLoopEndFrameThreaded + fadeSize > mEndFrameThreaded)
      fadeSize -= mLoopEndFrameThreaded + fadeSize - mEndFrameThreaded;
    // Start the cross-fade
    Fade.StartFade(mVolume.Get(AudioThreads::MixThread), mLoopEndFrameThreaded, fadeSize, mAssetObject, 
      mCrossFadeTail.Get() == cTrue);
  }

  Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread, 
    this, Events::SoundLooped), this);

  // Reset variables
  mFrameIndexThreaded = mLoopStartFrameThreaded;
  mCurrentTime.Set(mFrameIndexThreaded * cSystemTimeIncrement, AudioThreads::MixThread);
  ResetMusicBeatsThreaded();

  // If streaming, reset to the beginning of the file
  if (mAssetObject->mStreaming)
    mAssetObject->ResetStreamingFile(cNodeID);
}

//**************************************************************************************************
void SoundInstance::TranslateChannelsThreaded(BufferType* inputSamples, const unsigned inputFrames, 
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

//**************************************************************************************************
void SoundInstance::FinishedCleanUpThreaded()
{
  if (mFinished.Get() == cTrue)
    return;

  mFinished.Set(cTrue);

  // Remove this instance from any associated tags
  forRange(TagObject* tag, TagListThreaded.All())
    tag->RemoveInstanceThreaded(this);

  Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread, 
    this, Events::SoundStopped), this);
  Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::RemoveFromAllTagsThreaded, this), this);
  Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::RemoveAllOutputs, (SoundNode*)this), this);

  Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundAsset::RemoveInstance, *mAssetObject, cNodeID), this);
}

//**************************************************************************************************
bool SoundInstance::BelowMinimumVolumeThreaded(unsigned frames)
{
  // Determine overall volume at the beginning and end of the mix
  float volume1 = mVolume.Get(AudioThreads::MixThread);
  float volume2 = volume1;

  // If interpolating volume, get the volume at the end of the mix
  if (mInterpolatingVolumeThreaded)
    volume2 = VolumeInterpolatorThreaded.ValueAtIndex(mFrameIndexThreaded + frames);

  // Adjust with all volume modifiers
  forRange(InstanceVolumeModifier* modifier, VolumeModListThreaded.All())
  {
    if (modifier->Active)
    {
      volume1 *= modifier->GetCurrentVolume();
      volume2 *= modifier->GetFutureVolume(frames);
    }
  }

  // Return true if both volumes lower than minimum volume threshold
  if (volume1 < Z::gSound->Mixer.mMinimumVolumeThresholdThreaded
    && volume2 < Z::gSound->Mixer.mMinimumVolumeThresholdThreaded)
    return true;
  else
    return false;
}

//**************************************************************************************************
void SoundInstance::RemoveFromAllTagsThreaded()
{
  while (!TagListThreaded.Empty())
    TagListThreaded.Back()->RemoveInstanceThreaded(this);
}

//**************************************************************************************************
void SoundInstance::MusicNotificationsThreaded()
{
  // If we are looping and close to the loop end, don't do notifications
  if (mLooping.Get() == cTrue && mLoopEndFrameThreaded - mFrameIndexThreaded < 100)
    return;

  // If there is a custom notification time set and we've hit that time
  if (mNotifyTime.Get(AudioThreads::MixThread) > 0 && !mCustomNotifySent.Get(AudioThreads::MixThread) 
    && mCurrentTime.Get(AudioThreads::MixThread) >= mNotifyTime.Get(AudioThreads::MixThread))
  {
    mCustomNotifySent.Set(true, AudioThreads::MixThread);

    Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundInstance::DispatchInstanceEventFromMixThread,
      this, Events::MusicCustomTime), this);
  }

  MusicNotify.ProcessAndNotify((float)mCurrentTime.Get(AudioThreads::MixThread), this);
}

//**************************************************************************************************
void SoundInstance::ResetMusicBeatsThreaded()
{
  MusicNotify.ResetBeats((float)mCurrentTime.Get(AudioThreads::MixThread), this);
}

//**************************************************************************************************
void SoundInstance::DisconnectThisAndAllInputs()
{
  SoundNode::DisconnectThisAndAllInputs();

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundInstance::FinishedCleanUpThreaded, this), this);
}

//**************************************************************************************************
void SoundInstance::SetPausedThreaded(bool pause)
{
  // If setting to paused and is not currently paused or pausing
  if (pause && mPaused.Get() == cFalse && !mPausingThreaded)
  {
    mPausingThreaded = true;
    mStopFrameCountThreaded = 0;
    mStopFramesToWaitThreaded = cPropertyChangeFrames + 10;
    InstanceVolumeModifier* mod = GetAvailableVolumeModThreaded();
    if (mod)
    {
      mod->Reset(1.0f, 0.0f, cPropertyChangeFrames, cPropertyChangeFrames);
      PausingModifierThreaded = mod;
    }
  }
  // If setting to not paused and is currently paused or pausing
  else if (!pause && (mPaused.Get() == cTrue || mPausingThreaded))
  {
    mPaused.Set(cFalse);
    mPausingThreaded = false;
    InstanceVolumeModifier* mod = GetAvailableVolumeModThreaded();
    if (mod)
      mod->Reset(0.0f, 1.0f, cPropertyChangeFrames, cPropertyChangeFrames);

    if (mCurrentTime.Get(AudioThreads::MixThread) == 0.0)
      ResetMusicBeatsThreaded();
  }
}

//**************************************************************************************************
void SoundInstance::StopThreaded()
{
  mStoppingThreaded = true;
  mStopFrameCountThreaded = 0;
  mStopFramesToWaitThreaded = cPropertyChangeFrames + 10;
  InstanceVolumeModifier* mod = GetAvailableVolumeModThreaded();
  if (mod)
    mod->Reset(1.0f, 0.0f, cPropertyChangeFrames, cPropertyChangeFrames);
}

//**************************************************************************************************
void SoundInstance::SetVolumeThreaded(float newVolume, float time)
{
  // If paused or the volume is close, just set the volume directly
  if (mPaused.Get() == 1 || IsWithinLimit(newVolume, mVolume.Get(AudioThreads::MixThread), 0.01f))
  {
    mInterpolatingVolumeThreaded = false;
    mVolume.Set(newVolume, AudioThreads::MixThread);
  }
  else
  {
    mInterpolatingVolumeThreaded = true;

    // Make sure there's a minimum interpolation time
    if (time < 0.02f)
      time = 0.02f;

    VolumeInterpolatorThreaded.SetValues(mVolume.Get(AudioThreads::MixThread), newVolume, 
      (unsigned)(time * cSystemSampleRate));
  }
}

//**************************************************************************************************
void SoundInstance::SetPitchThreaded(float semitones, float time)
{
  // Check for no pitch shift and no interpolation
  if (semitones == 0.0f && time == 0.0f)
  {
    mPitchShiftingThreaded = false;
    Pitch.SetPitchFactor(1.0f, 0.0f);
  }
  else
  {
    mPitchShiftingThreaded = true;
    Pitch.SetPitchFactor(Math::Pow(2.0f, semitones / 12.0f), time);
  }
}

//**************************************************************************************************
void SoundInstance::SetTimeThreaded(float seconds)
{
  // Only need to cross-fade if we're not at the beginning of the file
  if (mFrameIndexThreaded > 0)
    Fade.StartFade(mVolume.Get(AudioThreads::MixThread), mFrameIndexThreaded, Fade.mDefaultFrames, mAssetObject, true);

  mFrameIndexThreaded = (int)(seconds * cSystemSampleRate);
  Math::Clamp(mFrameIndexThreaded, 0, mEndFrameThreaded);

  mCurrentTime.Set(mFrameIndexThreaded * cSystemTimeIncrement, AudioThreads::MixThread);
  ResetMusicBeatsThreaded();
}

//**************************************************************************************************
void SoundInstance::SetBeatsPerMinuteThreaded(float beats)
{
  MusicNotify.mSecondsPerBeat.Set(60.0f / Math::Max(beats, 0.0f), AudioThreads::MixThread);
  MusicNotify.mSecondsPerEighth = MusicNotify.mSecondsPerBeat.Get(AudioThreads::MixThread) 
    *  MusicNotify.mBeatNoteType / 8.0f;
}

//**************************************************************************************************
void SoundInstance::SetTimeSignatureThreaded(float beats, float noteType)
{
  MusicNotify.mBeatsPerBar = (int)Math::Max(beats, 0.0f);
  MusicNotify.mBeatNoteType = (int)Math::Max(noteType, 0.0f);

  if (MusicNotify.mBeatsPerBar == 0 || MusicNotify.mBeatNoteType == 0)
  {
    MusicNotify.mSecondsPerEighth = 0.0f;
  }
  else
  {
    MusicNotify.mSecondsPerEighth = MusicNotify.mSecondsPerBeat.Get(AudioThreads::MixThread) 
      *  MusicNotify.mBeatNoteType / 8.0f;
  }
}

}//namespace Zero
