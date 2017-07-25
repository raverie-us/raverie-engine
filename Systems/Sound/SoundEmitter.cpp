///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
Math::Random gRandom;

//------------------------------------------------------------------------------------ Sound Emitter

//**************************************************************************************************
ZilchDefineType(SoundEmitterDisplay, builder, type)
{
}

//**************************************************************************************************
String SoundEmitterDisplay::GetName(HandleParam object)
{
  return "SoundEmitter";
}

//**************************************************************************************************
String SoundEmitterDisplay::GetDebugText(HandleParam object)
{
  return "SoundEmitter";
}

//**************************************************************************************************
ZilchDefineType(SoundEmitter, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindTag(Tags::Sound);

  ZeroBindDependency(Cog);
  ZeroBindDependency(Transform);

  ZilchBindGetterSetterProperty(Volume)->Add(new EditorRange(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(Decibels)->Add(new EditorRange(-32.0f, 6.0f, 0.1f));
  ZilchBindGetterSetterProperty(Pitch)->Add(new EditorRange(-2.0f, 2.0f, 0.1f));
  ZilchBindGetterSetterProperty(Semitones)->Add(new EditorRange(-24.0f, 24.0f, 0.1f));
  ZilchBindFieldProperty(mDirectional)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(EmitAngle)->Add(new EditorRange(0.0f, 360.0f, 1.0f))->
    ZeroFilterBool(mDirectional);
  ZilchBindGetterSetterProperty(RearVolume)->Add(new EditorRange(0.0f, 1.0f, 0.1f))->
    ZeroFilterBool(mDirectional);

  ZilchBindGetterSetterProperty(Attenuator);

  ZilchBindGetterSetter(Paused);
  ZilchBindMethod(PlayCue);
  ZilchBindMethod(PlayCuePaused);
  ZilchBindGetter(IsPlaying);
  ZilchBindMethod(InterpolatePitch);
  ZilchBindMethod(InterpolateSemitones);
  ZilchBindMethod(InterpolateVolume);
  ZilchBindMethod(InterpolateDecibels);
  ZilchBindGetter(InputNode);
  ZilchBindGetter(OutputNode);

  type->Add(new SoundEmitterDisplay());
}

//**************************************************************************************************
SoundEmitter::SoundEmitter() : 
  mEmitterObject(nullptr), 
  mPitchNode(nullptr), 
  mVolumeNode(nullptr), 
  mAttenuatorNode(nullptr),
  mPitch(0.0f), 
  mVolume(1.0f), 
  mIsPaused(false), 
  mDirectional(false), 
  mEmitAngle(90.0f), 
  mRearVolume(0.2f), 
  mInputNode(nullptr), 
  mOutputNode(nullptr)
{
  mAttenuator = SoundAttenuatorManager::GetDefault();
}

//**************************************************************************************************
SoundEmitter::~SoundEmitter()
{
  // If the space still exists, remove this emitter from its list
  if (mSpace != nullptr)
    mSpace->mEmitters.Erase(this);

  // Remove any SoundCue attenuation nodes from their attenuators
  // and delete the InstanceAttenuation objects
  while (!mAttenuatorList.Empty())
  {
    // Get the InstanceAttenuation object
    InstanceAttenuation& info = mAttenuatorList.Front();
    // Remove it from the list
    mAttenuatorList.PopFront();
    // Check if the SoundAttenuator still exists
    SoundAttenuator* attenuator = info.mAttenuator;
    if (attenuator)
      // Remove the SoundAttenuatorNode from the SoundAttenuator
      attenuator->RemoveAttenuationNode(info.mAttenuatorNode);
    // Delete the InstanceAttenuation object
    delete &info;
  }

  // Remove this emitter's attenuation node from the attenuator
  if (mAttenuatorNode && mAttenuator)
    mAttenuator->RemoveAttenuationNode(mAttenuatorNode);

  // Disconnect emitter nodes and all nodes attached to this emitter
  if (mOutputNode && mOutputNode->mNode)
    mOutputNode->mNode->DisconnectThisAndAllInputs();

  // Delete nodes
  if (mEmitterObject)
    mEmitterObject->DeleteThisNode();
  if (mPitchNode)
    mPitchNode->DeleteThisNode();
  if (mVolumeNode)
    mVolumeNode->DeleteThisNode();

  // Stop sound nodes from doing anything in destructor
  if (mInputNode)
    mInputNode->mNode = nullptr;
  if (mOutputNode)
    mOutputNode->mNode = nullptr;
}

//**************************************************************************************************
void SoundEmitter::Serialize(Serializer& stream)
{
  SerializeNameDefault(mVolume, 1.0f);
  SerializeNameDefault(mPitch, 0.0f);
  SerializeResourceName(mAttenuator, SoundAttenuatorManager);
  SerializeNameDefault(mDirectional, false);
  SerializeNameDefault(mEmitAngle, 90.0f);
  SerializeNameDefault(mRearVolume, 0.2f);
}

//**************************************************************************************************
void SoundEmitter::Initialize(CogInitializer& initializer)
{
  // Get current position
  mTransform = GetOwner()->has(Transform);
  mPrevPosition = mTransform->GetWorldTranslation();

  // Save the ID for this emitter's SoundNodes
  mNodeID = Z::gSound->mCounter++;

  // Get the sound space
  mSpace = initializer.mSpace->has(SoundSpace);
  ErrorIf(!mSpace, "No SoundSpace when initializing SoundEmitter");
  if (mSpace)
  {
    Zero::Status status;

    // Add emitter to the space
    mSpace->mEmitters.PushBack(this);
    // Add a new emitter to the audio engine
    mEmitterObject = new Audio::EmitterNode(status, "Emitter", mNodeID, mPrevPosition, 
      Math::Vec3(0, 0, 0), this);
    if (status.Failed())
    {
      DoNotifyError("Error Initializing SoundEmitter", status.Message);
      mEmitterObject->DeleteThisNode();
      mEmitterObject = nullptr;
      return;
    }

    // If directional, set directional information in audio engine
    if (mDirectional)
    {
      mEmitterObject->SetDirectionalAngle(mEmitAngle, mRearVolume);

      Mat4 matrix = mTransform->GetWorldMatrix();
      Vec4 bx = matrix.BasisX();
      Vec4 by = matrix.BasisY();

      Vec3 x = Vec3(bx.x, bx.y, bx.z);
      Vec3 y = Vec3(by.x, by.y, by.z);
      Vec3 forward = x.Cross(y);

      mEmitterObject->SetForwardDirection(forward);
    }

    // Add a new pitch node below the emitter node
    mPitchNode = new Audio::PitchNode(status, "Emitter", mNodeID, this);
    if (status.Succeeded())
    {
      mPitchNode->SetPitch((int)(Z::gSound->PitchToSemitones(mPitch) * 100.0f), 0.0f);
      mPitchNode->AddInput(mEmitterObject);
    }
    else
    {
      DoNotifyWarning("Error Creating SoundEmitter PitchNode", status.Message);
      mPitchNode->DeleteThisNode();
      mPitchNode = nullptr;
    }

    // Add a new volume node below the emitter node
    mVolumeNode = new Audio::VolumeNode(status, "Emitter", mNodeID, this);
    if (status.Succeeded())
    {
      mVolumeNode->SetVolume(mVolume, 0.0f);
      if (mPitchNode)
        mVolumeNode->AddInput(mPitchNode);
      else
        mVolumeNode->AddInput(mEmitterObject);
    }
    else
    {
      DoNotifyWarning("Error Creating SoundEmitter VolumeNode", status.Message);
      mVolumeNode->DeleteThisNode();
      mVolumeNode = nullptr;
    }

    // Create the SoundNode for the OutputNode
    SoundNode* outNode = new SoundNode();
    // If the VolumeNode exists, set this as the output
    if (mVolumeNode)
      outNode->mNode = mVolumeNode;
    else
    {
      // If no VolumeNode but PitchNode exists, set that as the output
      if (mPitchNode)
        outNode->mNode = mPitchNode;
      // Otherwise set the EmitterNode as the output
      else
        outNode->mNode = mEmitterObject;
    }
    // Set the variables for the restrictions on the output node
    outNode->mCanInsertBefore = false;
    outNode->mCanRemove = false;
    outNode->mCanReplace = false;
    // Set the OutputNode handle
    mOutputNode = outNode;

    // Set up attenuation and create the InputNode
    SetUpAttenuatorNode(mAttenuator);

    // Add to all existing listeners in the space
    forRange(SoundListener& listener, mSpace->GetListeners()->All())
      listener.GetSoundNode()->AddInputNode(mOutputNode);
  }

}

//**************************************************************************************************
void SoundEmitter::DebugDraw()
{
  // If directional, draw a cone in the forward direction
  if (mDirectional)
  {
    Transform* t = GetOwner()->has(Transform);
    Vec3 pos = t->GetWorldTranslation();
    Vec3 scale = t->GetScale();

    Mat4 matrix = mTransform->GetWorldMatrix();
    Vec4 bx = matrix.BasisX();
    Vec4 by = matrix.BasisY();

    Vec3 x = Vec3(bx.x, bx.y, bx.z);
    Vec3 y = Vec3(by.x, by.y, by.z);
    Vec3 forward = x.Cross(y);

    gDebugDraw->Add(Debug::Cone(pos, -forward, 2.0f, scale.x).Color(Color::Red));
  }
}

//**************************************************************************************************
float SoundEmitter::GetVolume()
{
  return mVolume;
}

//**************************************************************************************************
void SoundEmitter::SetVolume(float volume)
{
  InterpolateVolume(volume, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolateVolume(float volume, float interpolationTime)
{
  mVolume = volume;

  if (mVolumeNode)
    mVolumeNode->SetVolume(volume, interpolationTime);
}

//**************************************************************************************************
float SoundEmitter::GetDecibels()
{
  return Z::gSound->VolumeToDecibels(mVolume);
}

//**************************************************************************************************
void SoundEmitter::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolateDecibels(float decibels, float interpolationTime)
{
  mVolume = Z::gSound->DecibelsToVolume(decibels);

  if (mVolumeNode)
    mVolumeNode->SetVolume(mVolume, interpolationTime);
}

//**************************************************************************************************
float SoundEmitter::GetPitch()
{
  return mPitch;
}

//**************************************************************************************************
void SoundEmitter::SetPitch(float pitch)
{
  InterpolatePitch(pitch, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolatePitch(float pitch, float interpolationTime)
{
  mPitch = pitch;

  if (mPitchNode)
    mPitchNode->SetPitch((int)(Z::gSound->PitchToSemitones(mPitch) * 100.0f), interpolationTime);
}

//**************************************************************************************************
float SoundEmitter::GetSemitones()
{
  return Z::gSound->PitchToSemitones(mPitch);
}

//**************************************************************************************************
void SoundEmitter::SetSemitones(float pitch)
{
  InterpolateSemitones(pitch, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolateSemitones(float pitch, float interpolationTime)
{
  mPitch = Z::gSound->SemitonesToPitch(pitch);

  if (mPitchNode)
    mPitchNode->SetPitch((int)(pitch * 100.0f), interpolationTime);
}

//**************************************************************************************************
bool SoundEmitter::GetPaused()
{
  return mIsPaused;
}

//**************************************************************************************************
void SoundEmitter::SetPaused(bool pause)
{
  if (mEmitterObject)
  {
    // Should be set to pause and is not currently paused
    if (pause && !mIsPaused)
    {
      mIsPaused = true;
      mEmitterObject->Pause();
    }
    // Should be set to un-paused and is currently paused
    else if (!pause && mIsPaused)
    {
      mIsPaused = false;
      mEmitterObject->Resume();
    }
  }
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundEmitter::PlayCue(SoundCue* cue)
{
  HandleOf<SoundInstance> instance = PlayCueInternal(cue, false);

  // If the SoundCue was successfully played, send the event
  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundEmitter::PlayCuePaused(SoundCue* cue)
{
  HandleOf<SoundInstance> instance = PlayCueInternal(cue, true);

  // If the SoundCue was successfully played, send the event
  if (instance)
  {
    SoundInstanceEvent eventToSend(instance);
    DispatchEvent(Events::SoundInstancePlayed, &eventToSend);
  }

  return instance;
}

//**************************************************************************************************
bool SoundEmitter::GetIsPlaying()
{
  if (!mEmitterObject)
    return false;

  if (CheckAttenuatorInputs() || mInputNode->GetHasInputs())
    return true;
  else
    return false;
}

//**************************************************************************************************
float SoundEmitter::GetEmitAngle()
{
  return mEmitAngle;
}

//**************************************************************************************************
void SoundEmitter::SetEmitAngle(float angleInDegrees)
{
  mEmitAngle = angleInDegrees;
  if (mEmitterObject)
    mEmitterObject->SetDirectionalAngle(angleInDegrees, mRearVolume);
}

//**************************************************************************************************
float SoundEmitter::GetRearVolume()
{
  return mRearVolume;
}

//**************************************************************************************************
void SoundEmitter::SetRearVolume(float minimumVolume)
{
  mRearVolume = minimumVolume;
  if (mEmitterObject)
    mEmitterObject->SetDirectionalAngle(mEmitAngle, minimumVolume);
}

//**************************************************************************************************
HandleOf<SoundAttenuator> SoundEmitter::GetAttenuator()
{
  return mAttenuator;
}

//**************************************************************************************************
void SoundEmitter::SetAttenuator(HandleOf<SoundAttenuator> attenuation)
{
  SetUpAttenuatorNode(attenuation);
}

//**************************************************************************************************
HandleOf<SoundNode> SoundEmitter::GetInputNode()
{
  return mInputNode;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundEmitter::GetOutputNode()
{
  return mOutputNode;
}

//**************************************************************************************************
void SoundEmitter::Update(float dt)
{
  if (!mEmitterObject)
    return;

  // Get new position data
  Vec3 newPosition = mTransform->GetWorldTranslation();
  Vec3 velocity = newPosition - mPrevPosition;
  mPrevPosition = newPosition;

  // Don't need to set positions if position hasn't changed
  if (velocity != Vec3(0, 0, 0))
  {
    //Change into m/s from m/f
    if (dt > 0.0f)
      velocity *= (1.0f / dt);

    // Update the audio engine with new data
    mEmitterObject->SetPosition(newPosition, velocity);

    // If the emitter has an attenuator, update its position
    if (mAttenuatorNode)
      mAttenuatorNode->mNode->SetPosition(newPosition);

    // Update SoundCue attenuation nodes with the new position
    forRange(InstanceAttenuation& nodeStruct, mAttenuatorList.All())
      nodeStruct.mAttenuatorNode->mNode->SetPosition(newPosition);
  }
  
  // If this is a directional emitter, set the forward direction
  if (mDirectional)
  {
    Mat4 matrix = mTransform->GetWorldMatrix();
    Vec4 bx = matrix.BasisX();
    Vec4 by = matrix.BasisY();

    Vec3 x = Vec3(bx.x, bx.y, bx.z);
    Vec3 y = Vec3(by.x, by.y, by.z);
    Vec3 forward = x.Cross(y);

    mEmitterObject->SetForwardDirection(forward);
  }

  // Check for SoundCue attenuator nodes with no input
  Array<InstanceAttenuation*> toDelete;
  forRange(InstanceAttenuation& info, mAttenuatorList.All())
  {
    if (!info.mAttenuatorNode->mNode->HasInputs())
    {
      // If this node has no inputs, tell the attenuator to remove it
      info.mAttenuator->RemoveAttenuationNode(info.mAttenuatorNode);
      // Add it to the list to remove from the map
      toDelete.PushBack(&info);
    }
  }
  forRange(InstanceAttenuation* info, toDelete.All())
  {
    mAttenuatorList.Erase(info);
    delete info;
  }
}

//**************************************************************************************************
HandleOf<SoundInstance> SoundEmitter::PlayCueInternal(SoundCue* cue, bool startPaused)
{
  if (!cue || !mEmitterObject)
    return nullptr;

  Audio::SoundNode* outputNode;
  // If the SoundCue has attenuation settings, get an attenuation node and store it
  HandleOf<SoundAttenuator> attenuatorHandle = cue->GetAttenuator();
  SoundAttenuator* attenuator = attenuatorHandle;
  if (attenuator && attenuator->Name != "DefaultNoAttenuation")
  {
    SoundAttenuatorNode* attenuatorNode = IsAttenuatorInList(attenuator);
    // If not already stored, get a new attenuation node
    if (!attenuatorNode)
    {
      attenuatorNode = attenuator->GetAttenuationNode("CueAttenuator", Z::gSound->mCounter++);
      if (attenuatorNode)
      {
        InstanceAttenuation* info = new InstanceAttenuation(attenuatorNode, attenuatorHandle);
        mAttenuatorList.PushBack(info);
        info->mAttenuatorNode->mNode->SetPosition(mPrevPosition);
        outputNode = info->mAttenuatorNode->mNode;
        mEmitterObject->AddInput(outputNode);
      }
      // If getting the attenuator node failed, use the emitter's input node
      else
        outputNode = mInputNode->mNode;
    }
    // Otherwise, use the node already in the list
    else
      outputNode = attenuatorNode->mNode;
  }
  // Use the emitter's input node (could be either attenuation or emitter node)
  else
    outputNode = mInputNode->mNode;

  // Play the instance
  HandleOf<SoundInstance> instance = cue->PlayCue(mSpace, outputNode, startPaused);

  return instance;
}

//**************************************************************************************************
bool SoundEmitter::CheckAttenuatorInputs()
{
  forRange(InstanceAttenuation& info, mAttenuatorList.All())
  {
    if (info.mAttenuatorNode->mNode->HasInputs())
      return true;
  }

  return false;
}

//**************************************************************************************************
void SoundEmitter::SetUpAttenuatorNode(HandleOf<SoundAttenuator> attenuator)
{
  if (!mEmitterObject)
    return;

  // If the input node object hasn't been created yet, create it
  if (!mInputNode)
  {
    SoundNode* newNode = new SoundNode();
    newNode->mNode = mEmitterObject;
    newNode->mCanInsertAfter = false;
    newNode->mCanInsertBefore = false;
    newNode->mCanReplace = false;
    newNode->mCanRemove = false;
    mInputNode = newNode;
  }

  // Get the pointer to the new attenuator
  SoundAttenuator* newAttenuator = attenuator;

  SoundAttenuator* oldAttenuator = mAttenuator;
  SoundAttenuatorNode* oldNode = mAttenuatorNode;

  // Set the attenuator handle
  mAttenuator = attenuator;

  // Check if the new attenuator is null or the default
  if (!newAttenuator || newAttenuator->Name == "DefaultNoAttenuation")
  {
    // Remove and delete the old attenuator node if needed
    if (mAttenuatorNode)
      oldAttenuator->RemoveAttenuationNode(mAttenuatorNode);

    // No attenuator node
    mAttenuatorNode = nullptr;
    // The input node is the emitter
    mInputNode->mNode = mEmitterObject;
  }
  else
  {
    // Check if there is already an attenuator
    if (mAttenuatorNode && oldAttenuator->Name != "DefaultNoAttenuation")
    {
      // Create a new attenuator node
      mAttenuatorNode = newAttenuator->GetAttenuationNode("EmitterAttenuator", Z::gSound->mCounter++);

      if (mAttenuatorNode)
      {
        mAttenuatorNode->mNode->SetPosition(mPrevPosition);

        // Store the new node as the input node
        mInputNode->mNode = mAttenuatorNode->mNode;
        // Swap the new node with the old node in the graph
        oldNode->mNode->ReplaceWith(mAttenuatorNode->mNode);

        // Remove and delete the old attenuator node
        oldAttenuator->RemoveAttenuationNode(oldNode);
      }
    }
    else
    {
      // Create a new attenuator node
      mAttenuatorNode = newAttenuator->GetAttenuationNode("EmitterAttenuator", Z::gSound->mCounter++);

      if (mAttenuatorNode)
      {
        mAttenuatorNode->mNode->SetPosition(mPrevPosition);

        // Store the new node as the input
        mInputNode->mNode = mAttenuatorNode->mNode;
        // Add the new node to the emitter node
        mEmitterObject->AddInput(mAttenuatorNode->mNode);
      }
    }
  }
}

//**************************************************************************************************
void SoundEmitter::SendAudioEvent(const Audio::AudioEventType eventType, void* data)
{
  if (eventType == Audio::AudioEventType::Notify_InterpolationDone)
  {
    SoundEvent event;
    DispatchEvent(Events::AudioInterpolationDone, &event);
  }

}

SoundAttenuatorNode* SoundEmitter::IsAttenuatorInList(SoundAttenuator* attenuator)
{
  forRange(InstanceAttenuation& data, mAttenuatorList.All())
  {
    SoundAttenuator* check = data.mAttenuator;
    if (attenuator == check)
      return data.mAttenuatorNode;
  }

  return nullptr;
}

}//namespace Zero
