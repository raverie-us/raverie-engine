// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
// Simple Sound

RaverieDefineType(SimpleSound, builder, type)
{
  RaverieBindComponent();
  RaverieBindDocumented();
  RaverieBindSetup(SetupMode::DefaultSerialization);

  RaverieBindDependency(Cog);
  RaverieBindDependency(SoundEmitter);
  RaverieBindTag(Tags::Sound);

  RaverieBindGetterSetterProperty(Cue);
  RaverieBindFieldProperty(mStartPlaying);
  RaverieBindFieldProperty(mPositional);

  RaverieBindGetter(IsPlaying);
  RaverieBindGetterSetter(Paused);
  RaverieBindMethod(Play);
  RaverieBindMethod(Stop);
}

SimpleSound::SimpleSound() : mInstance(nullptr), mStartPlaying(true), mPositional(false)
{
}

SimpleSound::~SimpleSound()
{
  Stop();
}

void SimpleSound::Initialize(CogInitializer& initializer)
{
  // Get the sound space
  mSpace = initializer.mSpace->has(SoundSpace);

  ErrorIf(!mSpace, "No SoundSpace when playing SimpleSound");

  // If not in editor mode and should play on initialize, start playing
  if (mStartPlaying && !initializer.mSpace->IsEditorMode() && mSpace)
    Play();
}

void SimpleSound::Serialize(Serializer& stream)
{
  SerializeNameDefault(mStartPlaying, true);
  SerializeNameDefault(mPositional, false);
  SerializeResourceName(mCue, SoundCueManager);
}

HandleOf<SoundInstance> SimpleSound::Play()
{
  SoundInstance* sound = mInstance;

  // If the sound is currently playing, stop it
  if (sound)
  {
    sound->Stop();
    mInstance = nullptr;
  }

  if (mSpace)
  {
    // Play the instance on the emitter (has dependency on SoundEmitter so will
    // always be valid)
    if (mPositional)
      mInstance = GetOwner()->has(SoundEmitter)->PlayCue(mCue);
    // If not positional, play on the SoundSpace
    else
      mInstance = mSpace->PlayCue(mCue);
  }

  return mInstance;
}

void SimpleSound::Stop()
{
  SoundInstance* sound = mInstance;
  if (sound)
    sound->Stop();
}

bool SimpleSound::GetPaused()
{
  SoundInstance* sound = mInstance;
  if (sound)
    return sound->GetPaused();
  else
    return false;
}

void SimpleSound::SetPaused(bool pause)
{
  SoundInstance* sound = mInstance;
  if (sound)
    sound->SetPaused(pause);
}

bool SimpleSound::GetIsPlaying()
{
  SoundInstance* sound = mInstance;
  if (sound)
    return sound->GetIsPlaying();
  else
    return false;
}

} // namespace Raverie
