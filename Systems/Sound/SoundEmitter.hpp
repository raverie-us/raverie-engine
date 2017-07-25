///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//----------------------------------------------------------------------------- Instance Attenuation

// Used to store attenuation information for each instance
struct InstanceAttenuation
{
  InstanceAttenuation(SoundAttenuatorNode* attenuatorNode, HandleOf<SoundAttenuator> attenuator) :
    mAttenuatorNode(attenuatorNode), 
    mAttenuator(attenuator) 
  {}
  InstanceAttenuation() : 
    mAttenuatorNode(nullptr) 
  {}

  SoundAttenuatorNode* mAttenuatorNode;
  HandleOf<SoundAttenuator> mAttenuator;

  Link<InstanceAttenuation> link;
};

//------------------------------------------------------------------------------------ Sound Emitter

class SoundEmitterDisplay : public MetaDisplay
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  String GetName(HandleParam object) override;
  String GetDebugText(HandleParam object) override;
};

/// Allows 3D positioning of sounds relative to SoundListeners
class SoundEmitter : public Component, public Audio::ExternalNodeInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundEmitter();
  ~SoundEmitter();

  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void SetDefaults() override {}
  void DebugDraw() override;

  /// The volume adjustment applied to all sounds played through this SoundEmitter.
  /// A value of 1 does nothing, 2 will double the volume, 0.5 will halve it.
  /// The Volume property is linked to the Decibels property (changing one will change the other).
  float GetVolume();
  void SetVolume(float volume);
  /// Interpolates the Volume property from its current value to the value passed in as the 
  /// first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateVolume(float volume, float interpolationTime);
  /// The volume adjustment, in decibels, applied to all sounds played through this SoundEmitter.
  /// A value of 0 does nothing, 6 will double the volume, -6 will halve it.
  /// The Decibels property is linked to the Volume property (changing one will change the other).
  float GetDecibels();
  void SetDecibels(float decibels);
  /// Interpolates the Decibels property from its current value to the value 
  /// passed in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateDecibels(float decibels, float interpolationTime);
  /// This property affects both the pitch and speed of all sounds played through this SoundEmitter.
  /// A value of 0 will do nothing, 1 will raise the pitch by an octave and 
  /// speed up the sound, -1 will lower the sound by an octave and slow it down. 
  /// The Pitch property is linked to the Semitones property (changing one will change the other).
  float GetPitch();
  void SetPitch(float pitch);
  /// Interpolates the Pitch property from its current value to the value passed in 
  /// as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolatePitch(float pitch, float interpolationTime);
  /// This property, specified in semitones (or half-steps), affects both the pitch and speed of 
  /// all sounds played through this SoundEmitter.. A value of 0 will do nothing, 12 will raise the pitch 
  /// by an octave and speed up the sound, -12 will lower the sound by an octave and slow it down.
  /// The Semitones property is linked to the Pitch property (changing one will change the other).
  float GetSemitones();
  void SetSemitones(float pitch);
  /// Interpolates the Semitones property from its current value to the value passed 
  /// in as the first parameter, over the number of seconds passed in as the second parameter.
  void InterpolateSemitones(float pitch, float interpolationTime);
  /// Setting this property to true pauses all sounds currently playing 
  /// through the SoundEmitter. Setting it to false will resume playback.
  bool GetPaused();
  void SetPaused(bool pause);
  /// Plays the SoundCue passed into the function and returns the resulting SoundInstance.
  HandleOf<SoundInstance> PlayCue(SoundCue* cue);
  /// Plays the SoundCue passed into the function and returns the resulting SoundInstance, which starts off paused.
  HandleOf<SoundInstance> PlayCuePaused(SoundCue* cue);
  /// This property will be true if there are SoundInstances currently associated with this 
  /// SoundEmitter, even if they are paused or otherwise not audible.
  bool GetIsPlaying();
  /// When true, the audio output of the SoundEmitter will be limited by the EmitAngle, 
  /// so that sound in front of the object will be louder than sound behind it. 
  /// Within the EmitAngle the sound will be at full volume. The volume interpolates 
  /// logarithmically until it reaches the RearVolume value directly behind the object.
  bool mDirectional;
  /// The angle of full volume sound for a directional SoundEmitter. An angle of 90, for example, 
  /// will be centered at the object's front, extending 45 degrees to the left and right. 
  float GetEmitAngle();
  void SetEmitAngle(float angleInDegrees);
  /// The volume of sound heard directly behind a directional SoundEmitter. It will only reach 
  /// this value in a small area, since volume is interpolated from the edge of the EmitAngle. 
  /// To make the volume as quiet as possible behind the object, use a small EmitAngle.
  float GetRearVolume();
  void SetRearVolume(float minimumVolume);
  /// If a SoundAttenuator resource other than DefaultNoAttenuation is selected it will be applied 
  /// to SoundCues without their own SoundAttenuator resource. If a SoundCue has attenuation 
  /// settings those will always be used. If neither has settings, the sound will not be attenuated.
  HandleOf<SoundAttenuator> GetAttenuator();
  void SetAttenuator(HandleOf<SoundAttenuator> attenuator);
  /// The SoundNode to use for attaching other nodes to the input of the SoundEmitter.
  HandleOf<SoundNode> GetInputNode();
  /// The SoundNode to use for attaching other nodes to the output of the SoundEmitter.
  HandleOf<SoundNode> GetOutputNode();

//Internals
  SoundSpace* mSpace;
  Transform* mTransform;
  Vec3 mPrevPosition;
  Link<SoundEmitter> link;

  void Update(float dt);
  HandleOf<SoundInstance> PlayCueInternal(SoundCue* cue, bool startPaused);

private:
  // Input node if no attenuation
  Audio::EmitterNode* mEmitterObject;
  // After emitter node in graph
  Audio::PitchNode* mPitchNode;
  // After pitch node in graph (this one is output)
  Audio::VolumeNode* mVolumeNode;
  HandleOf<SoundNode> mInputNode;
  HandleOf<SoundNode> mOutputNode;
  float mPitch;
  float mVolume;
  float mEmitAngle;
  float mRearVolume;
  bool mIsPaused;
  unsigned mNodeID;

  HandleOf<SoundAttenuator> mAttenuator;
  SoundAttenuatorNode* mAttenuatorNode;

  typedef InList<InstanceAttenuation> AttenuatorListType;
  AttenuatorListType mAttenuatorList;

  bool CheckAttenuatorInputs();
  void SetUpAttenuatorNode(HandleOf<SoundAttenuator> attenuator);
  void SendAudioEvent(const Audio::AudioEventType eventType, void* data) override;
  SoundAttenuatorNode* IsAttenuatorInList(SoundAttenuator* attenuator);
};

}//namespace Zero
