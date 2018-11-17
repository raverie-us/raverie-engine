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
  ZilchBindGetterSetterProperty(AttenuationScale);
}

//**************************************************************************************************
SoundListener::SoundListener() : 
  mActive(true),
  mAttenuationScale(1.0f)
{
}

//**************************************************************************************************
SoundListener::~SoundListener()
{
  // Remove from space
  mSpace->mListeners.Erase(this);
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
  mPrevForward = Math::ToVector3(matrix.BasisZ());

  // Add a new listener node
  String name;
  if (!mSpace->GetOwner()->IsEditorMode())
    name = "Listener";
  else
    name = "EditorListener";

  mListenerNode = new ListenerNode(name, Z::gSound->mCounter++,
    ListenerWorldPositionInfo(mPrevPosition, Math::Vec3(0, 0, 0), Math::ToMatrix3(matrix)));

  // If not currently active, tell the audio engine
  if (!mActive)
    mListenerNode->SetActive(false);
  // Set the attenuation scale
  mListenerNode->SetAttenuationScale(mAttenuationScale);

  // Add to all existing SoundEmitters
  forRange(SoundEmitter& emitter, mSpace->mEmitters.All())
    mListenerNode->AddInputNode(emitter.GetOutputNode());

  // Add to the SoundSpace's output
  mSpace->GetInputNode()->AddInputNode(mListenerNode);

}

//**************************************************************************************************
void SoundListener::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
  SerializeNameDefault(mAttenuationScale, 1.0f);
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
  mListenerNode->SetActive(newActive);

  mActive = newActive;
}

//**************************************************************************************************
HandleOf<SoundNode> SoundListener::GetSoundNode()
{
  return mListenerNode;
}

//**************************************************************************************************
float SoundListener::GetAttenuationScale()
{
  return mAttenuationScale;
}

//**************************************************************************************************
void SoundListener::SetAttenuationScale(float scale)
{
  mAttenuationScale = scale;
  mListenerNode->SetAttenuationScale(scale);
}

//**************************************************************************************************
void SoundListener::Update(float invDt)
{
  if (mActive)
  {
    Vec3 position = mTransform->GetWorldTranslation();
    Vec3 velocity = position - mPrevPosition;
    velocity *= invDt;

    Mat4 matrix = mTransform->GetWorldMatrix();
    Vec3 basisZ = Math::ToVector3(matrix.BasisZ());

    // Only need to set position if it's changed
    if (mPrevPosition != position || mPrevForward != basisZ)
    {
      mPrevPosition = position;
      mPrevForward = basisZ;

      mListenerNode->SetPositionData(ListenerWorldPositionInfo(position, velocity, Math::ToMatrix3(matrix)));
    }
  }
}

}//namespace Zero
