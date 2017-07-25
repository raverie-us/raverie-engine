///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
class SoundSystem;
class SoundInstance;

//-------------------------------------------------------------------------------------- Sound Space

/// Sound functionality associated with a Space
class SoundSpace : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundSpace();
  ~SoundSpace();

  void Initialize(CogInitializer& config) override;
  void Serialize(Serializer& stream) override;

  /// The volume adjustment applied to all sounds in the space. 
  /// A value of 1 does nothing, 2 will double the volume, 0.5 will halve it.
  float GetVolume();
  void SetVolume(float value);
  /// Interpolates the SoundSpace's Volume property from its current value to the value 
  /// passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float value, float interpolationTime);
  /// The volume adjustment, in decibels, applied to all sounds in the space. 
  /// A value of 0 does nothing, 6 will double the sound's volume, -6 will halve it.
  float GetDecibels();
  void SetDecibels(float decibels);
  /// Interpolates the SoundSpace's Decibels property from its current value to the value 
  /// passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float decibels, float interpolationTime);
  /// The pitch adjustment applied to all sounds in the space. A value of 0 will do nothing, 1 will 
  /// raise the pitch by an octave and speed up the sound, -1 will lower the sound by an octave and slow it down. 
  float GetPitch();
  void SetPitch(float pitch);
  /// Interpolates the SoundSpace's Pitch property from its current value to the 
  /// value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolatePitch(float pitch, float time);
  /// The pitch adjustment, in semitones (or half-steps), applied to all sounds in the space. 
  /// A value of 0 will do nothing, 12 will raise the pitch by an octave and speed up the sound, 
  /// -12 will lower the sound by an octave and slow it down.
  float GetSemitones();
  void SetSemitones(float pitch);
  /// Interpolates the SoundSpace's Semitones property from its current value to the 
  /// value passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateSemitones(float pitch, float time);
  /// Setting this Property to true will pause all audio in the space. Setting it to false will resume all audio.
  bool GetPaused();
  void SetPaused(bool pause);
  /// If true, the audio of the space will pause when the space is paused.
  bool mPauseWithTimeSpace;
  /// If true, the audio in the SoundSpace will be pitched according to the TimeScale of the Space 
  /// (if time slows down the audio will slow down and lower in pitch, if it speeds up the audio will speed up and raise in pitch).
  bool mPitchWithTimeSpace;
  /// Plays the passed-in SoundCue non-positionally and returns the resulting SoundInstance.
  HandleOf<SoundInstance> PlayCue(SoundCue* cue);
  /// Plays the passed-in SoundCue non-positionally and returns the resulting SoundInstance, which starts off paused.
  HandleOf<SoundInstance> PlayCuePaused(SoundCue* cue);
  /// The SoundNode which is the ultimate output of all sounds in this space.
  HandleOf<SoundNode> GetInputNode();
  /// The SoundNode which can be used to attach other nodes which should process all audio in the SoundSpace.
  HandleOf<SoundNode> GetOutputNode();

//Internals
  InList<SoundEmitter> mEmitters;
  InList<SoundListener> mListeners;
  Audio::AudioSystemInterface* mAudioSystem;
  Link<SoundSpace> link;

  void Update();
  InList<SoundListener>* GetListeners();

private:
  SoundSystem* mSoundSystem;
  Audio::PitchNode* mPitchNode;
  Audio::VolumeNode* mVolumeNode;
  float mVolume;
  float mPitch;
  bool mPause;
  bool mLevelPaused;
  bool mEditorMode;
  HandleOf<SoundNode> mInputNode;
  HandleOf<SoundNode> mOutputNode;
  unsigned mSpaceNodeID;

  class NodeInterface : public Audio::ExternalNodeInterface
  {
    void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override {}
  };

  NodeInterface mNodeInterface;

  friend class SoundSystem;
  friend class SoundNodeGraph;
};

}