///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(JointDebugDrawConfig, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindDependency(Joint);

  ZilchBindGetterSetterProperty(Active)->ZeroSerialize(true);
  ZilchBindFieldProperty(mSize)->ZeroSerialize(real(1.0));
  ZilchBindFieldProperty(mDetail)->ZeroSerialize(real(10.0));
  ZilchBindGetterSetterProperty(ObjectAPerspective)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(ObjectBPerspective)->ZeroSerialize(false);

  ZeroBindTag(Tags::Physics);
  ZeroBindTag(Tags::Joint);
}

JointDebugDrawConfig::JointDebugDrawConfig()
{

}

JointDebugDrawConfig::~JointDebugDrawConfig()
{

}

void JointDebugDrawConfig::Serialize(Serializer& stream)
{
  // Temporarily call meta serialization until we fully switch
  MetaSerializeProperties(this, stream);
}

bool JointDebugDrawConfig::GetActive() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::Active);
}

void JointDebugDrawConfig::SetActive(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::Active, state);
}

bool JointDebugDrawConfig::GetObjectAPerspective() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::ObjectAPerspective);
}

void JointDebugDrawConfig::SetObjectAPerspective(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::ObjectAPerspective, state);
}

bool JointDebugDrawConfig::GetObjectBPerspective() const
{
  return mPerspective.IsSet(JointDebugDrawConfigFlags::ObjectBPerspective);
}

void JointDebugDrawConfig::SetObjectBPerspective(bool state)
{
  mPerspective.SetState(JointDebugDrawConfigFlags::ObjectBPerspective, state);
}

}//namespace Zero
