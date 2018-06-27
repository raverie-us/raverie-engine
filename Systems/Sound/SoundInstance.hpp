///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

namespace Events
{

DeclareEvent(SoundStopped);
DeclareEvent(SoundLooped);
DeclareEvent(MusicBeat);
DeclareEvent(MusicBar);
DeclareEvent(MusicEighthNote);
DeclareEvent(MusicQuarterNote);
DeclareEvent(MusicHalfNote);
DeclareEvent(MusicWholeNote);
DeclareEvent(MusicCustomTime);

DeclareEvent(SoundCuePrePlay);
DeclareEvent(SoundCuePostPlay);

} // namespace Events

//----------------------------------------------------------------------------- Sound Instance Event

/// Sent for various SoundInstance-related events
class SoundInstanceEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundInstanceEvent() : mSoundInstance(nullptr) {}
  SoundInstanceEvent(const HandleOf<SoundInstance>& instance) : mSoundInstance(instance) {}

  /// The SoundInstance associated with this event
  HandleOf<SoundInstance> GetSoundInstance() { return mSoundInstance; }

private:
  HandleOf<SoundInstance> mSoundInstance;
};

//-------------------------------------------------------------------------------- Cross Fade Object

class AudioFadeObject
{
public:
  AudioFadeObject();

  void StartFade(float startingVolume, unsigned startingIndex, unsigned fadeFrames,
    SoundAsset* asset, bool crossFade);
  void ApplyFade(float* buffer, unsigned howManyFrames);
  void GetMoreSamples();

  // If true, sound is currently fading
  bool mFading;
  // The current frame index of the fade
  unsigned mFrameIndex;
  // The asset frame at which the fade started
  unsigned mStartFrame;
  // If true, the audio will be cross-faded rather than just faded out
  // (new audio will fade in proportionally to the audio fading out)
  bool mCrossFade;
  // The asset associated with this sound
  SoundAsset* mAsset;
  // Used to interpolate cross-fading volumes. 
  InterpolatingObject VolumeInterpolator;
  // The samples to use for cross-fading
  BufferType FadeSamples;
  // Number of frames to use for default fade
  unsigned mDefaultFrames;
  // The ID of the parent instance
  unsigned mInstanceID;
};

//------------------------------------------------------------------------ Music Notification Object

class MusicNotificationObject
{
public:
  MusicNotificationObject() :
    mSecondsPerBeat(0),
    mBeatsPerBar(0),
    mBeatsCount(0),
    mBeatNoteType(0),
    mSecondsPerEighth(0),
    mEighthNoteCount(0),
    mTotalEighths(0),
    mTotalBeats(0)
  {}

  void ProcessAndNotify(float currentTime, SoundInstance* instance);
  void ResetBeats(float currentTime, SoundInstance* instance);

  // Number of seconds per music beat
  Threaded<float> mSecondsPerBeat;
  // Number of music beats per music bar
  int mBeatsPerBar;
  // Accumulates beats to know when we hit another bar
  int mBeatsCount;
  // Type of note used for the beat (4 = quarter note, etc.)
  int mBeatNoteType;
  // Number of seconds for each eighth note
  float mSecondsPerEighth;
  // Accumulates number of eighth notes (reset on each bar)
  int mEighthNoteCount;
  // Tracks the total number of eighth notes from the beginning of the music
  int mTotalEighths;
  // Tracks the total number of music beats from the beginning of the music
  int mTotalBeats;
};

//----------------------------------------------------------------------------------- Sound Instance

class TagObject;

/// The object associated with a currently playing sound
class SoundInstance : public SimpleCollapseNode
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundInstance(Status& status, SoundSpace* space, SoundAsset* asset, float volume, float pitch);
  ~SoundInstance();

  /// The volume adjustment of the SoundInstance, initially set by the SoundCue's Volume property. 
  /// A value of 1 does nothing, 2 will double the sound's volume, 0.5 will halve it.
  /// The Volume property is linked to the Decibels property (changing one will change the other).
  float GetVolume();
  void SetVolume(float volume);
  /// Interpolates the SoundInstance's Volume property from its current value 
  /// to the value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float volume, float interpolationTime);
  /// The volume adjustment (in decibels) of the SoundInstance, initially set by the SoundCue's Decibels property. 
  /// A value of 0 does nothing, 6 will double the sound's volume, -6 will halve it.
  /// The Decibels property is linked to the Volume property (changing one will change the other).
  float GetDecibels();
  void SetDecibels(float decibels);
  /// Interpolates the SoundInstance's Decibels property from its current value 
  /// to the value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float decibels, float interpolationTime);
  /// The pitch adjustment of the SoundInstance, initially set by the SoundCue's Pitch property. 
  /// A value of 0 will do nothing, 1 will raise the pitch by an octave and speed up the sound, 
  /// -1 will lower the sound by an octave and slow it down. 
  /// The Pitch property is linked to the Semitones property (changing one will change the other).
  float GetPitch();
  void SetPitch(float pitch);
  /// Interpolates the SoundInstance's Pitch property from its current value to the 
  /// value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolatePitch(float pitch, float interpolationTime);
  /// The pitch adjustment, in semitones (or half-steps), of the SoundInstance, 
  /// initially set by the SoundCue's Semitones property.  A value of 0 will do nothing, 12 will raise the pitch 
  /// by an octave and speed up the sound, and -12 will lower the sound by an octave and slow it down. 
  /// The Semitones property is linked to the Pitch property (changing one will change the other).
  float GetSemitones();
  void SetSemitones(float semitones);
  /// Interpolates the SoundInstance's Semitones property from its current value to the 
  /// value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateSemitones(float pitchSemitones, float interpolationTime);
  /// Setting this Property to true will pause a currently playing SoundInstance.
  /// Setting it to false will resume playback.
  bool GetPaused();
  void SetPaused(bool pause);
  /// Stops the playback of this SoundInstance. It cannot be re-started.
  void Stop();
  /// This Property will be true while the SoundInstance is playing, then will become false when its sound has stopped.
  bool GetIsPlaying();
  /// The SoundNode associated with this SoundInstance.
  HandleOf<SoundNode> GetSoundNode();
  /// When this Property is true the SoundInstance will loop indefinitely. If changed to false 
  /// while a SoundInstance is looping the SoundInstance will continue playing to its EndTime and then stop.
  bool GetLooping();
  void SetLooping(bool loop);
  /// This property tells you to the current playback position, in seconds from the beginning of the file, 
  /// and allows you to tell the instance to change its playback position to a different time. 
  /// Be aware that the time will not be precisely accurate. If the Sound resource used to play 
  /// the SoundInstance has Streamed selected, you cannot set the playback position.
  float GetTime();
  void SetTime(float seconds);
  /// The length of the entire audio file, in seconds.
  float GetFileLength();
  /// The time in seconds from the beginning of the file that the instance will stop.
  float GetEndTime();
  void SetEndTime(float seconds);
  /// The time in seconds from the beginning of the file that the instance will jump back to when it loops.
  float GetLoopStartTime();
  void SetLoopStartTime(float seconds);
  /// The time in seconds from the beginning of the file that the instance will stop and jump back when looping.
  float GetLoopEndTime();
  void SetLoopEndTime(float seconds);
  /// The time in seconds after looping that will fade out and play concurrently with the audio where
  /// the instance jumped back to.
  float GetLoopTailTime();
  void SetLoopTailTime(float seconds);
  /// If true, the loop tail will be cross-faded: the audio from where the instance jumped back to
  /// will fade in as the loop tail fades out. If false, the new audio will start at full volume.
  bool GetCrossFadeLoopTail();
  void SetCrossFadeLoopTail(bool crossFade);
  /// The time (in seconds from the beginning of the file) to get a MusicCustomTime event.
  float GetCustomEventTime();
  void SetCustomEventTime(float seconds);
  /// The name of the Sound being played by this SoundInstance.
  String GetSoundName();

// Internals
  Array<SoundTag*> SoundTags;
  // The speed of the music in beats per minute.
  float GetBeatsPerMinute();
  void SetBeatsPerMinute(float beats);
  // Sets the time signature of the music.
  void SetTimeSignature(float beats, float noteType);

  void Play(bool loop, SoundNode* outputNode, bool startPaused);

  // Returns a volume modifier from the array.
  InstanceVolumeModifier* GetAvailableVolumeModThreaded();
  // Adds the requested number of audio frames to the back of the specified buffer
  bool GetOutputForThisMixThreaded(BufferType* buffer, const unsigned numberOfChannels);
  // Gets the cumulative volume attenuation from all output nodes
  float GetAttenuationThisMixThreaded();

  void DispatchInstanceEventFromMixThread(const String eventID);

private:
  bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest) override;
  // Fills the provided buffer with the audio data for the current mix
  void AddSamplesToBufferThreaded(BufferType* buffer, unsigned outputFrames, unsigned outputChannels);
  // Resets back to the loop start point
  void LoopThreaded();
  // Translates the audio data in the array to the specified output channels, and puts the data
  // back into the array
  static void TranslateChannelsThreaded(BufferType* inputSamples, const unsigned inputFrames,
    const unsigned inputChannels, const unsigned outputChannels);
  // Sends notification and removes instance from any associated tags.
  void FinishedCleanUpThreaded();
  // Check for whether the total volume is lower than the minimum.
  bool BelowMinimumVolumeThreaded(unsigned frames);
  // Removes this instance from all tags it is associated with.
  void RemoveFromAllTagsThreaded();
  // Handle music beat notifications.
  void MusicNotificationsThreaded();
  // Resets the music beat tracking to the current location in the file.
  void ResetMusicBeatsThreaded();
  // Calls the disconnect function on the base class and then calls FinishedCleanUp 
  void DisconnectThisAndAllInputs() override;

  void SetPausedThreaded(bool pause);
  void StopThreaded();
  void SetVolumeThreaded(float newVolume, float time);
  void SetPitchThreaded(float semitones, float time);
  void SetTimeThreaded(float seconds);
  void SetBeatsPerMinuteThreaded(float beats);
  void SetTimeSignatureThreaded(float beats, float noteType);

  typedef Zero::Array<TagObject*> TagListType;
  // List of tags that the instance is currently associated with.
  TagListType TagListThreaded;

  // Handles fading the audio when looping or jumping.
  AudioFadeObject Fade;
  // Handles sending music notifications to the external object.
  MusicNotificationObject MusicNotify;
  // Handles pitch changes.
  PitchChangeHandler Pitch;

  HandleOf<SoundAsset> mAssetObject;
  SoundSpace* mSpace;

  // The (approximate) current time position in the file.
  Threaded<double> mCurrentTime;
  // If true, sound will loop instead of stopping.
  ThreadedInt mLooping;
  // If true, sound instance is paused.
  ThreadedInt mPaused;
  // Current volume of this sound.
  Threaded<float> mVolume;
  // If true, the sound has finished playing.
  ThreadedInt mFinished;
  // The time at which the instance will stop playing.
  float mEndTime;
  // The time to jump back to when looping.
  float mLoopStartTime;
  // The time to stop and jump back when looping.
  float mLoopEndTime;
  // The time after the LoopEndTime to use for cross-fading.
  float mLoopTailTime;
  // If true, the loop tail will cross-fade (new audio will fade in as the tail fades out).
  ThreadedInt mCrossFadeTail;
  // Time for custom notification.
  Threaded<float> mNotifyTime;
  // Tracks whether the custom notification has been sent.
  Threaded<bool> mCustomNotifySent;
  // The current number of semitones by which the pitch is being changed.
  Threaded<float> mPitchSemitones;

  const float cMaxLoopTailTime = 30.0f;

  // ** Threaded variables **

  // Index of the current audio frame. 
  int mFrameIndexThreaded;
  // If true, sound is ramping volume down to zero to pause. 
  bool mPausingThreaded;
  // If true, sound is ramping volume down to zero to stop. 
  bool mStoppingThreaded;
  // Counts number of frames until pausing or stopping. 
  int mStopFrameCountThreaded;
  // Number of frames to wait until pausing or stopping. 
  int mStopFramesToWaitThreaded;
  // If true, volume is being interpolated. 
  bool mInterpolatingVolumeThreaded;
  // If true, currently changing pitch. 
  bool mPitchShiftingThreaded;
  // The frame this instance's playback should stop at.
  int mEndFrameThreaded;
  // The frame to jump back to when looping.
  int mLoopStartFrameThreaded;
  // The frame to stop and jump back when looping.
  int mLoopEndFrameThreaded;
  // The frames after the LoopEndTime to use for fading.
  int mLoopTailFramesThreaded;
  // Used to control volume modifications while pausing.
  InstanceVolumeModifier *PausingModifierThreaded;
  // Used to interpolate from one volume to another. 
  InterpolatingObject VolumeInterpolatorThreaded;
  // Volume adjustments, used by the instance and by tags.
  Array<InstanceVolumeModifier*> VolumeModListThreaded;
  // The mix version of the audio data saved in InputSamples
  unsigned mSavedOutputVersionThreaded;
  // Processed samples that are saved between mixes
  BufferType SavedSamplesThreaded;
};



}//namespace Zero
