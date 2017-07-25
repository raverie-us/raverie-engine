///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{
//------------------------------------------------------------------------------------- Simple Sound

/// Plays a specified SoundCue, either when created or when the Play method is called.
class SimpleSound : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SimpleSound();
  ~SimpleSound();

  // Component Interface
  void Initialize(CogInitializer& initializer) override;
  void Serialize(Serializer& stream) override;

  /// Begins playing the SoundCue chosen in the Cue property and returns the resulting SoundInstance. 
  /// If already playing it will be stopped and re-started.
  HandleOf<SoundInstance> Play();
  /// Stops a currently playing SoundInstance if it exists.
  void Stop();
  /// Setting this Property to true will pause a currently playing SoundCue. Setting it to false will resume playback.
  bool GetPaused();
  void SetPaused(bool pause);
  /// Will be true if the SoundCue is currently being played.
  bool GetIsPlaying();
  /// If this property is true the SoundCue will begin playing as soon as the object is created.
  bool mStartPlaying;
  /// If this property is true the SoundCue will be played positionally (heard at a specific 
  /// location by SoundListeners) through the SoundEmitter component on the same object. 
  /// If false, the SoundCue will be played through the SoundSpace, and will NOT be affected
  /// by any SoundEmitter settings.
  bool mPositional;
  /// The SoundCue that will be played.
  ResourceProperty(SoundCue, Cue);

private:
  // Stored pointer to the sound's space
  SoundSpace* mSpace;
  // Reference to a currently playing instance
  HandleOf<SoundInstance> mInstance;

  friend class SoundEmitter;
};

}//namespace Zero
