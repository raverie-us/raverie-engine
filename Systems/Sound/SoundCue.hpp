///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
/// Whether a SoundCue should play once or loop continuously.
/// <param name="Single">The sound will play once to its EndTime and then stop.</param>
/// <param name="Looping">The sound will loop continuously between its LoopStartTime and LoopEndTime.</param>
DeclareEnum2(SoundPlayMode, Single, Looping);
/// Whether a SoundCue should pick a Sound to play randomly or sequentially.
/// <param name="Random">The SoundCue will pick a Sound at random, taking into account the Weight values.</param>
/// <param name="Sequential">Each time the SoundCue is played it will pick the next Sound in the order they are displayed.</param>
DeclareEnum2(SoundSelectMode, Random, Sequential);

//-------------------------------------------------------------------------------------- Sound Entry

/// Stores Sounds and associated properties to be used by a SoundCue.
class SoundEntry : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundEntry();

  void Serialize(Serializer& stream) override;
  void SetDefaults() {}

  /// The Sound resource that will be played by this SoundEntry. 
  Sound* GetSound();
  void SetSound(Sound* sound);
  /// The weighted randomization value for this particular SoundEntry to be chosen to play.
  /// The values of all SoundEntries are considered: two SoundEntries with weights of 1 and 1
  /// will each play 50 percent of the time, as will weights of 10 and 10.
  float mWeight;
  /// Preview this sound with no SoundCue settings.
  void Preview();
  /// Stop previewing this sound.
  void StopPreview();
  /// The time (in seconds) at which the Sound will start playing. 
  /// A value of 0 will start the Sound at the beginning of the audio file.
  float mStartTime;
  /// The time (in seconds) at which the Sound will stop playing. Defaults to the length of the audio file.
  float mEndTime;
  /// The time (in seconds) from the beginning of the audio file that a looping SoundInstance will jump back to 
  /// after it reaches the LoopEndTime. The Sound will still start at the StartTime when it is played, 
  /// but after it begins looping it will start at the LoopStartTime. 
  float mLoopStartTime;
  /// The time (in seconds) from the beginning of the audio file at which a looping SoundInstance 
  /// jumps back to the LoopStartTime. If it stops looping while playing it will continue to the EndTime and then stop. 
  float mLoopEndTime;
  /// The length (in seconds) of the tail, from the LoopEndTime, which will continue to play 
  /// after the Sound jumps back to the LoopStartTime. The loop tail will fade out smoothly.
  float mLoopTailLength;
  /// If false, the loop tail will be added to the audio and will fade out over the specified time.
  /// If true, the audio will be cross-faded, so the beginning of the loop will fade in as the tail fades out.
  bool mCrossFadeLoopTail;

private:
  HandleOf<Sound> mSound;
};

class SoundEntryDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

//---------------------------------------------------------------------------------- Sound Tag Entry

/// Stores a SoundTag which will be applied to all SoundInstances created by this SoundCue.
class SoundTagEntry : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundTagEntry();

  void Serialize(Serializer& stream) override;
  void SetDefaults() {}

  /// This SoundTag will be added to all SoundInstances created by the SoundCue.
  SoundTag* GetSoundTag();
  void SetSoundTag(SoundTag* tag);

private:
  HandleOf<SoundTag> mSoundTag;
};

class SoundTagEntryDisplay : public MetaDisplay
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

//---------------------------------------------------------------------------------------- Sound Cue

typedef Array<SoundTag*> SoundTagList;

class SoundCueDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

/// Settings and Sounds for playing audio files.
class SoundCue : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundCue();
  ~SoundCue();

  void Serialize(Serializer& serializer) override;
  void Unload() override;

  /// If Single is chosen the SoundInstance created by the SoundCue will be played once and will stop 
  /// when it reaches its EndTime. If Looping is chosen the SoundInstance will play continuously 
  /// until either it is stopped or its Looping property is set to false.
  SoundPlayMode::Enum GetPlayMode();
  void SetPlayMode(SoundPlayMode::Enum playMode);
  /// If Random is chosen the SoundCue will randomly choose which SoundEntry to play.
  /// If Sequential is chosen it will play the SoundEntries in order.
  SoundSelectMode::Enum GetSelectMode();
  void SetSelectMode(SoundSelectMode::Enum selectMode);
  /// The volume adjustment that will be applied to the sound when it plays. 
  /// A value of 1 does nothing, 2 will double the sound's volume, 0.5 will halve it.
  /// The Volume property is linked to the Decibels property (changing one will change the other).
  float GetVolume();
  void SetVolume(float volume);
  /// The volume adjustment, in decibels, that will be applied to the sound when it plays. 
  /// A value of 0 does nothing, 6 will double the sound's volume, -6 will halve it.
  /// The Decibels property is linked to the Volume property (changing one will change the other).
  float GetDecibels();
  void SetDecibels(float decibels);
  /// If false, the VolumeVariation value will be used to randomize the volume. 
  /// If true, the DecibelVariation field will be shown and will be used for randomization.
  bool mUseDecibelVariation;
  /// Sets how much the Volume will be randomized every time the SoundCue plays. If Volume is 1, 
  /// and VolumeVariation is 0.5, the volume adjustment will be chosen randomly between 0.5 and 1.5. 
  float mVolumeVariation;
  /// Sets how much the Decibels will be randomized every time the SoundCue plays. If Decibels is 0, 
  /// and DecibelVariation is 4, the volume adjustment will be chosen randomly between -4 and 4. 
  float mDecibelVariation;
  /// This property affects both the pitch and speed of the sound played by the SoundCue. 
  /// A value of 0 will do nothing, 1 will raise the pitch by an octave and speed up the sound, 
  /// -1 will lower the sound by an octave and slow it down. 
  /// The Pitch property is linked to the Semitones property (changing one will change the other).
  float GetPitch();
  void SetPitch(float pitch);
  /// This property, specified in semitones (or half-steps), affects both the pitch and speed 
  /// of the sound played by the SoundCue. A value of 0 will do nothing, 12 will raise the pitch 
  /// by an octave and speed up the sound, and -12 will lower the sound by an octave and slow it down. 
  /// The Semitones property is linked to the Pitch property (changing one will change the other).
  float GetSemitones();
  void SetSemitones(float semitones);
  /// If false, the PitchVariation value will be used to randomize the volume. 
  /// If true, the SemitoneVariation field will be shown and will be used for randomization.
  bool mUseSemitoneVariation;
  /// Sets how much the pitch will be randomized every time the SoundCue plays. If Pitch is 0, 
  /// and PitchVariation is 0.3, the pitch of the sound will be chosen randomly between -0.3 and 0.3.
  float mPitchVariation;
  /// Sets how much the pitch will be randomized every time the SoundCue plays. If Semitones is 0, 
  /// and SemitoneVariation is 5, the pitch of the sound will be chosen randomly between -5 and 5.
  float mSemitoneVariation;
  /// If true, the music options will be shown. If false, they will be hidden.
  bool mShowMusicOptions;
  /// The speed of the music, using beats per minute.
  float mBeatsPerMinute;
  /// The top number of the music's time signature (beats per measure).
  float mTimeSigBeats;
  /// The bottom number of the music's time signature (which type of note has the beat).
  float mTimeSigValue;
  /// If a SoundAttenuator resource is selected, it will be applied to reduce the sound's volume 
  /// with distance when played through a SoundEmitter. If DefaultNoAttenuation is selected on the SoundCue 
  /// and a different SoundAttenuator is selected on the SoundEmitter, the SoundEmitter's settings will be applied.
  /// If DefaultNoAttenuation is selected on both the sound will not be attenuated.
  HandleOf<SoundAttenuator> GetAttenuator();
  void SetAttenuator(SoundAttenuator* attenuation);
  /// Adds a new SoundEntry to this SoundCue.
  void AddSoundEntry(Sound* sound, float weight);
  /// Adds a new SoundTagEntry to this SoundCue.
  void AddSoundTagEntry(SoundTag* soundTag);
  /// Plays this SoundCue using a specified SoundNode as the output and returns the resulting SoundInstance.
  HandleOf<SoundInstance> PlayCueOnNode(SoundNode* outputNode, bool startPaused);

//Internals
  void Preview();
  void StopPreview();
  HandleOf<SoundInstance> PlayCue(SoundSpace *space, Audio::SoundNode* outputNode, bool startPaused);

private:
  Array<SoundEntry> Sounds;
  Array<SoundTagEntry> SoundTags;
  HandleOf<SoundAttenuator> mAttenuator;

  SoundPlayMode::Enum mPlayMode;
  SoundSelectMode::Enum mSelectMode;
  float mVolume;
  float mPitch;
  int mSoundIndex;
};

//-------------------------------------------------------------------------------- Sound Cue Manager

///Cue Manager manages sound cues.
class SoundCueManager : public ResourceManager
{
public:
  DeclareResourceManager(SoundCueManager, SoundCue);
  SoundCueManager(BoundType* resourceType);
};

}//namespace Zero
