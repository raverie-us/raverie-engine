///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{
//----------------------------------------------------------------------------------- Sound Listener

//**************************************************************************************************
ZilchDefineType(SoundListener, builder, type)
{
  ZeroBindComponent();
  ZeroBindDocumented();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  ZeroBindDependency(Cog);
  ZeroBindDependency(Transform);
  ZeroBindTag(Tags::Sound);

  ZilchBindGetterSetterProperty(Active);
  ZilchBindGetter(SoundNode);
}

//**************************************************************************************************
SoundListener::SoundListener() : mActive(true)
{
}

//**************************************************************************************************
SoundListener::~SoundListener()
{
  // Remove from space
  mSpace->mListeners.Erase(this);
  // Remove from audio engine
  if (mSoundNode && mSoundNode->mNode)
  {
    mSoundNode->mNode->DisconnectInputs();
    mSoundNode->mNode->DeleteThisNode();
    mSoundNode->mNode = nullptr;
  }
}

//**************************************************************************************************
void SoundListener::Initialize(CogInitializer& initializer)
{
  // Get space and add to space's list of listeners
  mSpace = initializer.mSpace->has(SoundSpace);
  mSpace->mListeners.PushBack(this);

  // Get current position
  mTransform = GetOwner()->has(Transform);
  mPrevPosition = mTransform->GetWorldTranslation();

  Mat4 matrix = mTransform->GetWorldMatrix();
  Vec4 bx = matrix.BasisX();
  Vec4 by = matrix.BasisY();

  Vec3 x = Vec3(bx.x, bx.y, bx.z);
  Vec3 y = Vec3(by.x, by.y, by.z);
  Vec3 forward = x.Cross(y);

  // Add a new listener to audio engine 
  SoundNode* newNode = new SoundNode();
  mSoundNode = newNode;
  Status status;
  String name;
  if (!mSpace->GetOwner()->IsEditorMode())
    name = "Listener";
  else
    name = "EditorListener";

  newNode->SetNode(new Audio::ListenerNode(status, name, Z::gSound->mCounter++,
    Audio::ListenerWorldPositionInfo(mTransform->GetWorldTranslation(), Math::Vec3(0, 0, 0), 
      -forward, y), newNode), status);

  if (status.Failed())
    return;

  newNode->mCanRemove = false;
  newNode->mCanReplace = false;

  // If not currently active, tell the audio engine
  if (!mActive)
    ((Audio::ListenerNode*)newNode->mNode)->SetActive(false);

  // Add to all existing SoundEmitters
  forRange(SoundEmitter& emitter, mSpace->mEmitters.All())
    newNode->AddInputNode(emitter.GetOutputNode());

  // Add to the SoundSpace's output
  mSpace->GetInputNode()->AddInputNode(newNode);

}

//**************************************************************************************************
void SoundListener::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
}

//**************************************************************************************************
void SoundListener::DebugDraw()
{
  Transform* t = GetOwner()->has(Transform);
  Vec3 pos = t->GetWorldTranslation();
  Vec3 scale = t->GetScale();
  Vec3 axis = Math::TransformNormal(t->GetWorldMatrix(), Vec3::cXAxis);

  gDebugDraw->Add(Debug::Cone(pos, axis, 2.0f, scale.x * 0.4f).Color(Color::Red));
  gDebugDraw->Add(Debug::Cone(pos, -axis, 2.0f, scale.x * 0.4f).Color(Color::Blue));
  gDebugDraw->Add(Debug::Text(pos + (axis * 2.0f), scale.x * 0.2f, "R").Color(Color::Red)
    .Centered(true).ViewAligned(true));
  gDebugDraw->Add(Debug::Text(pos - (axis * 2.0f), scale.x * 0.2f, "L").Color(Color::Blue)
    .Centered(true).ViewAligned(true));
}

//**************************************************************************************************
bool SoundListener::GetActive()
{
  return mActive;
}

//**************************************************************************************************
void SoundListener::SetActive(bool newActive)
{
  ((Audio::ListenerNode*)mSoundNode->mNode)->SetActive(newActive);

  mActive = newActive;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundListener::GetSoundNode()
{
  return mSoundNode;
}

//**************************************************************************************************
void SoundListener::Update(float invDt)
{
  if (mActive)
  {
    Vec3 position = mTransform->GetWorldTranslation();
    Vec3 velocity = position - mPrevPosition;
    velocity *= invDt;
    mPrevPosition = position;

    // Only need to set position if it's changed
    if (velocity != Vec3(0.0f, 0.0f, 0.0f))
    {
      Mat4 matrix = mTransform->GetWorldMatrix();
      Vec4 bx = matrix.BasisX();
      Vec4 by = matrix.BasisY();
      Vec4 bz = matrix.BasisZ();

      Vec3 x = Vec3(bx.x, bx.y, bx.z);
      Vec3 y = Vec3(by.x, by.y, by.z);
      Vec3 forward = x.Cross(y);

      ((Audio::ListenerNode*)mSoundNode->mNode)->SetPositionData(Audio::ListenerWorldPositionInfo
        (position, velocity, -forward, y));
    }
  }
}

}//namespace Zero
