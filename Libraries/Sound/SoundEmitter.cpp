///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

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

  ZilchBindGetterSetterProperty(Volume)->Add(new EditorSlider(0.0f, 2.0f, 0.01f));
  ZilchBindGetterSetterProperty(Decibels)->Add(new EditorSlider(-32.0f, 6.0f, 0.1f));
  ZilchBindGetterSetterProperty(Pitch)->Add(new EditorSlider(-2.0f, 2.0f, 0.1f));
  ZilchBindGetterSetterProperty(Semitones)->Add(new EditorSlider(-24.0f, 24.0f, 0.1f));
  ZilchBindFieldProperty(mDirectional)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindGetterSetterProperty(EmitAngle)->Add(new EditorSlider(0.0f, 360.0f, 1.0f))->
    ZeroFilterBool(mDirectional);
  ZilchBindGetterSetterProperty(RearVolume)->Add(new EditorSlider(0.0f, 1.0f, 0.1f))->
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
  ZilchBindGetter(InputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(OutputNode)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetter(SoundNodeInput);
  ZilchBindGetter(SoundNodeOutput);

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
  mSoundNodeInput(nullptr), 
  mSoundNodeOutput(nullptr),
  mDebugArcDistance(1.0f)
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
  if (mSoundNodeOutput)
    mSoundNodeOutput->DisconnectThisAndAllInputs();
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
    // Add emitter to the space
    mSpace->mEmitters.PushBack(this);
    // Add a new emitter to the audio engine
    mEmitterObject = new EmitterNode("Emitter", mNodeID, mPrevPosition, Math::Vec3(0, 0, 0));

    // If directional, set directional information in audio engine
    if (mDirectional)
    {
      // Okay to call the threaded functions directly because it's not attached to anything yet
      mEmitterObject->SetDirectionalAngleThreaded(mEmitAngle, mRearVolume);

      mEmitterObject->SetForwardDirectionThreaded(Math::ToVector3(mTransform->GetWorldMatrix().BasisZ()));
    }

    // Set the emitter node as the input
    mSoundNodeInput = mEmitterObject;

    // Add a new pitch node below the emitter node
    mPitchNode = new PitchNode("Emitter", mNodeID);
    mPitchNode->SetPitch(mPitch);
    mPitchNode->AddInputNode(mEmitterObject);

    // Add a new volume node below the pitch node
    mVolumeNode = new VolumeNode("Emitter", mNodeID);
    mVolumeNode->SetVolume(mVolume);
    mVolumeNode->AddInputNode(mPitchNode);

    // Set the volume node as the output
    mSoundNodeOutput = mVolumeNode;

    // Set up attenuation 
    SetUpAttenuatorNode(mAttenuator);

    // Add to all existing listeners in the space
    forRange(SoundListener& listener, mSpace->GetListeners()->All())
      listener.GetSoundNode()->AddInputNode(mSoundNodeOutput);
  }

}

//**************************************************************************************************
void SoundEmitter::DebugDraw()
{
  Transform* t = GetOwner()->has(Transform);
  Vec3 pos = t->GetWorldTranslation();
  Vec3 scale = t->GetScale();
  float multiplier = Math::Max(scale.x, scale.z);
  float spacing = 0.3f * multiplier;
  float minDist = 1.0f * multiplier;
  float maxDist = 2.5f * multiplier;

  mDebugArcDistance += 0.01f;
  if (mDebugArcDistance > maxDist)
    mDebugArcDistance = minDist;

  float distance = mDebugArcDistance * multiplier;

  Mat4 worldMat = t->GetWorldMatrix();
  Vec3 axisZ = Math::TransformNormal(worldMat, Vec3::cZAxis);
  Vec3 axisY = Math::TransformNormal(worldMat, Vec3::cYAxis);
  
  // Draw three lines
  for (int i = 0; i < 3; ++i)
  {
    // Only draw this line if it's above the minimum distance
    if (distance > minDist)
    {
      // If directional, draw an arc indicating the EmitAngle of the emitter
      if (mDirectional)
      {
        float radians = Math::DegToRad(mEmitAngle * 0.5f);

        Vec3 endPoint = Math::RotateVector(-axisZ * distance, axisY, radians);
        Vec3 startPoint = Math::RotateVector(-axisZ * distance, axisY, -radians);
        Vec3 midPoint = pos - (axisZ * distance);

        gDebugDraw->Add(Debug::Arc(pos + startPoint, midPoint, pos + endPoint).Color(Color::Blue));
      }
      // If not directional, draw a circle
      else
        gDebugDraw->Add(Debug::Circle(pos, axisY, distance).Color(Color::Blue));
    }

    // Move the distance down for the next line
    distance -= spacing;
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
  mVolume = Math::Clamp(volume, 0.0f, cMaxVolumeValue);

  mVolumeNode->InterpolateVolume(mVolume, interpolationTime);
}

//**************************************************************************************************
float SoundEmitter::GetDecibels()
{
  return VolumeToDecibels(mVolume);
}

//**************************************************************************************************
void SoundEmitter::SetDecibels(float decibels)
{
  InterpolateDecibels(decibels, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolateDecibels(float decibels, float interpolationTime)
{
  mVolume = Math::Clamp(DecibelsToVolume(decibels), 0.0f, cMaxVolumeValue);

  mVolumeNode->InterpolateVolume(mVolume, interpolationTime);
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
  mPitch = Math::Clamp(pitch, cMinPitchValue, cMaxPitchValue);

  mPitchNode->InterpolatePitch(mPitch, interpolationTime);
}

//**************************************************************************************************
float SoundEmitter::GetSemitones()
{
  return PitchToSemitones(mPitch);
}

//**************************************************************************************************
void SoundEmitter::SetSemitones(float pitch)
{
  InterpolateSemitones(pitch, 0.0f);
}

//**************************************************************************************************
void SoundEmitter::InterpolateSemitones(float pitch, float interpolationTime)
{
  mPitch = Math::Clamp(SemitonesToPitch(pitch), cMinPitchValue, cMaxPitchValue);

  mPitchNode->InterpolatePitch(mPitch, interpolationTime);
}

//**************************************************************************************************
bool SoundEmitter::GetPaused()
{
  return mIsPaused;
}

//**************************************************************************************************
void SoundEmitter::SetPaused(bool pause)
{
  mIsPaused = pause;
  EmitterNode* node = mEmitterObject;
  Z::gSound->Mixer.AddTask(CreateFunctor(&EmitterNode::SetPausedThreaded, node, mIsPaused), node);
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

  if (CheckAttenuatorInputs() || mSoundNodeInput->GetHasInputs())
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
  mEmitAngle = Math::Clamp(angleInDegrees, 1.0f, 360.0f);
  EmitterNode* node = mEmitterObject;
  Z::gSound->Mixer.AddTask(CreateFunctor(&EmitterNode::SetDirectionalAngleThreaded, node,
    mEmitAngle, mRearVolume), node);
}

//**************************************************************************************************
float SoundEmitter::GetRearVolume()
{
  return mRearVolume;
}

//**************************************************************************************************
void SoundEmitter::SetRearVolume(float minimumVolume)
{
  mRearVolume = Math::Clamp(minimumVolume, 0.0f, cMaxVolumeValue);
  EmitterNode* node = mEmitterObject;
  Z::gSound->Mixer.AddTask(CreateFunctor(&EmitterNode::SetDirectionalAngleThreaded, node,
    mEmitAngle, mRearVolume), node);
}

//**************************************************************************************************
SoundAttenuator* SoundEmitter::GetAttenuator()
{
  return mAttenuator;
}

//**************************************************************************************************
void SoundEmitter::SetAttenuator(SoundAttenuator* attenuation)
{
  SetUpAttenuatorNode(attenuation);
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundEmitter::GetSoundNodeInput()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundEmitter::GetInputNode()
{
  return mSoundNodeInput;
}

//**************************************************************************************************
Zilch::HandleOf<Zero::SoundNode> SoundEmitter::GetSoundNodeOutput()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundEmitter::GetOutputNode()
{
  return mSoundNodeOutput;
}

//**************************************************************************************************
void SoundEmitter::Update(float dt)
{
  EmitterNode* node = mEmitterObject;

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

    // Update the emitter node with new data
    Z::gSound->Mixer.AddTask(CreateFunctor(&EmitterNode::SetPositionThreaded, node,
      newPosition, velocity), node);

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

    Z::gSound->Mixer.AddTask(CreateFunctor(&EmitterNode::SetForwardDirectionThreaded, node,
      forward), node);
  }

  // Check for SoundCue attenuator nodes with no input
  Array<InstanceAttenuation*> toDelete;
  forRange(InstanceAttenuation& info, mAttenuatorList.All())
  {
    if (!info.mAttenuatorNode->mNode->GetHasInputs())
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
  if (!cue)
    return nullptr;

  HandleOf<SoundNode> outputNode;
  // If the SoundCue has attenuation settings, get an attenuation node and store it
  SoundAttenuator* attenuator = cue->GetAttenuator();
  if (attenuator && attenuator->Name != "DefaultNoAttenuation")
  {
    SoundAttenuatorNode* attenuatorNode = IsAttenuatorInList(attenuator);
    // If not already stored, get a new attenuation node
    if (!attenuatorNode)
    {
      attenuatorNode = attenuator->GetAttenuationNode("CueAttenuator", Z::gSound->mCounter++);
      if (attenuatorNode)
      {
        InstanceAttenuation* info = new InstanceAttenuation(attenuatorNode, attenuator);
        mAttenuatorList.PushBack(info);
        info->mAttenuatorNode->mNode->SetPosition(mPrevPosition);
        outputNode = info->mAttenuatorNode->mNode;
        mEmitterObject->AddInputNode(outputNode);
      }
      // If getting the attenuator node failed, use the emitter's input node
      else
        outputNode = mSoundNodeInput;
    }
    // Otherwise, use the node already in the list
    else
      outputNode = attenuatorNode->mNode;
  }
  // Use the emitter's input node (could be either attenuation or emitter node)
  else
    outputNode = mSoundNodeInput;

  // Play the instance
  HandleOf<SoundInstance> instance = cue->PlayCue(mSpace, outputNode, startPaused);

  return instance;
}

//**************************************************************************************************
bool SoundEmitter::CheckAttenuatorInputs()
{
  forRange(InstanceAttenuation& info, mAttenuatorList.All())
  {
    if (info.mAttenuatorNode->mNode->GetHasInputs())
      return true;
  }

  return false;
}

//**************************************************************************************************
void SoundEmitter::SetUpAttenuatorNode(SoundAttenuator* newAttenuator)
{
  SoundAttenuator* oldAttenuator = mAttenuator;
  SoundAttenuatorNode* oldNode = mAttenuatorNode;

  // Set the attenuator handle
  mAttenuator = newAttenuator;

  // Check if the new attenuator is null or the default
  if (!newAttenuator || newAttenuator->Name == "DefaultNoAttenuation")
  {
    // Remove and delete the old attenuator node if needed
    if (mAttenuatorNode)
      oldAttenuator->RemoveAttenuationNode(mAttenuatorNode);

    // No attenuator node
    mAttenuatorNode = nullptr;
    // The input node is the emitter
    mSoundNodeInput = mEmitterObject;
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
        mSoundNodeInput = mAttenuatorNode->mNode;
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
        mSoundNodeInput = mAttenuatorNode->mNode;
        // Add the new node to the emitter node
        mEmitterObject->AddInputNode(mAttenuatorNode->mNode);
      }
    }
  }
}

//**************************************************************************************************
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
