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
}

//----------------------------------------------------------------------------- Sound Instance Event

/// Sent for various SoundInstance-related events
class SoundInstanceEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundInstanceEvent() : mSoundInstance(nullptr) {}
  SoundInstanceEvent(HandleOf<SoundInstance> instance) : mSoundInstance(instance) {}

  /// The SoundInstance associated with this event
  HandleOf<SoundInstance> GetSoundInstance() { return mSoundInstance; }

private:
  HandleOf<SoundInstance> mSoundInstance;
};

//----------------------------------------------------------------------------------- Sound Instance

/// The object associated with a currently playing sound
class SoundInstance : public ReferenceCountedEventObject, Audio::ExternalNodeInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundInstance(Status& status, SoundSpace* space, Audio::SoundAssetNode* asset, float volume, float pitch);

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
  SoundNode* GetSoundNode();
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
  /// The time (in seconds from the beginning of the file) to get a MusicCustomTime event.
  float GetCustomEventTime();
  void SetCustomEventTime(float seconds);
  /// The name of the Sound being played by this SoundInstance.
  StringParam GetSoundName();

  // Internals
  Array<SoundTag*> SoundTags;
  // The speed of the music in beats per minute.
  float GetBeatsPerMinute();
  void SetBeatsPerMinute(float beats);
  // Sets the time signature of the music.
  void SetTimeSignature(float beats, float noteType);

private:
  HandleOf<SoundNode> mSoundNode;
  void Play(bool loop, SoundTag *tag, Audio::SoundNode* outputNode, bool startPaused);
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;

  Audio::SoundAssetNode* mAssetObject;
  SoundSpace* mSpace;
  bool mIsPaused;
  bool mIsPlaying;

  friend class SoundEmitter;
  friend class SoundSpace;
  friend class SoundCue;
  friend class SoundEntry;
  friend class SoundSystem;
};



}//namespace Zero
