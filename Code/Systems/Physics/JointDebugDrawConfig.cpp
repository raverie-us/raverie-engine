// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(JointDebugDrawConfig, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::DefaultSerialization);
  RaverieBindDocumented();
  RaverieBindDependency(Joint);

  RaverieBindGetterSetterProperty(Active)->RaverieSerialize(true);
  RaverieBindFieldProperty(mSize)->RaverieSerialize(real(1.0));
  RaverieBindFieldProperty(mDetail)->RaverieSerialize(real(10.0));
  RaverieBindGetterSetterProperty(ObjectAPerspective)->RaverieSerialize(true);
  RaverieBindGetterSetterProperty(ObjectBPerspective)->RaverieSerialize(false);

  RaverieBindTag(Tags::Physics);
  RaverieBindTag(Tags::Joint);
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

} // namespace Raverie
