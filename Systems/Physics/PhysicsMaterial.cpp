///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------- Physics Material
DefinePhysicsRuntimeClone(PhysicsMaterial);

ZilchDefineType(PhysicsMaterial, builder, type)
{
  ZeroBindTag(Tags::Physics);
  ZeroBindDocumented();

  // For runtime clone
  ZilchBindConstructor();

  ZilchBindMethod(CreateRuntime);
  ZilchBindMethod(RuntimeClone);

  ZilchBindGetterSetterProperty(Density);
  ZilchBindFieldProperty(mRestitution)->Add(new EditorRange(0, 1, real(0.001f)));
  ZilchBindGetterSetterProperty(Friction);
  ZilchBindFieldProperty(mHighPriority);
  ZilchBindMethod(UpdateAndNotifyIfModified);
}

PhysicsMaterial::PhysicsMaterial()
{
  mModified = false;
}

void PhysicsMaterial::Serialize(Serializer& stream)
{
  SerializeNameDefault(mRestitution, real(.5f));
  SerializeNameDefault(mStaticFriction, real(.5f));
  SerializeNameDefault(mDynamicFriction, real(.5f));
  SerializeNameDefault(mDensity, real(1.0f));
  SerializeNameDefault(mHighPriority, false);

  SetFriction(mDynamicFriction);
  SetDensityInternal(mDensity, false);
}

void PhysicsMaterial::ResourceModified()
{
  // If we're already modified we don't need to do any extra logic
  if(mModified)
    return;

  mModified = true;
  PhysicsMaterialManager* manager = (PhysicsMaterialManager*)GetManager();
  PhysicsMaterialManager::PhysicsMaterialReference resourceRef(this);
  manager->mModifiedResources.PushBack(resourceRef);
}

HandleOf<PhysicsMaterial> PhysicsMaterial::CreateRuntime()
{
  return PhysicsMaterialManager::CreateRuntime();
}

real PhysicsMaterial::GetFriction()
{
  return mDynamicFriction;
}

void PhysicsMaterial::SetFriction(real friction)
{
  if(friction < real(0))
  {
    friction = real(0);
    DoNotifyWarning("Invalid friction bounds.", "Friction cannot be negative.");
  }
  if(friction > real(1000000))
  {
    friction = real(1000000);
    DoNotifyWarning("Invalid friction bounds.", "Friction is too large");
  }

  mDynamicFriction = mStaticFriction = friction;
}

real PhysicsMaterial::GetDensity()
{
  return mDensity;
}

void PhysicsMaterial::SetDensity(real density)
{
  SetDensityInternal(density);
}

void PhysicsMaterial::CopyTo(PhysicsMaterial* destination)
{
  destination->mRestitution = mRestitution;
  destination->mStaticFriction = mStaticFriction;
  destination->mDynamicFriction = mDynamicFriction;
  destination->mDensity = mDensity;
  destination->mHighPriority = mHighPriority;
}

void PhysicsMaterial::UpdateAndNotifyIfModified()
{
  if(mModified == false)
    return;

  mModified = false;
  // If our values change then we need to let all collider's who are using this resource know
  // so they can re-compute any necessary values (currently only the density)
  ResourceEvent toSend;
  toSend.EventResource = this;
  DispatchEvent(Events::ResourceModified, &toSend);
}

void PhysicsMaterial::SetDensityInternal(real density, bool markModified)
{
  if(density == real(0))
    density = real(0);
  else if(density < real(.001f))
  {
    density = real(.001f);
    DoNotifyWarning("Too small of a density.", "The density being set is too small and "
      "will likely result in floating point inaccuracies. If you would like "
      "to make the object massless, please set the density to 0.");
  }
  else if(density > real(1e+9))
  {
    density = real(1e+9);
    DoNotifyWarning("Too large of a density.", "The density being set is too large and "
      "will likely result in floating point inaccuracies.");
  }

  mDensity = density;
  if(markModified)
    ResourceModified();
}

//---------------------------------------------------------- PhysicsMaterialManager
ImplementResourceManager(PhysicsMaterialManager, PhysicsMaterial);

PhysicsMaterialManager::PhysicsMaterialManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  AddLoader("PhysicsMaterial", new TextDataFileLoader<PhysicsMaterialManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.PhysicsMaterial.data"));
  DefaultResourceName = "DefaultPhysicsMaterial";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

void PhysicsMaterialManager::UpdateAndNotifyModifiedResources()
{
  for(size_t i = 0; i < mModifiedResources.Size(); ++i)
  {
    PhysicsMaterial* material = mModifiedResources[i];
    if(material != nullptr)
      material->UpdateAndNotifyIfModified();
  }
  mModifiedResources.Clear();
}

}//namespace Zero
