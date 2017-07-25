///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

JointCreator::ConnectionInfo::ConnectionInfo(Cog* obj0, Cog* obj1, bool worldConnect)
{
  a = obj0;
  b = obj1;
  if(worldConnect)
  {
    Collider* collider = a->has(Collider);
    b = collider->mSpace->mWorldCollider->GetOwner();
  }

  // Get each object's transform
  Cog* objects[2] = {a, b};
  for(size_t i = 0; i < 2; ++i)
  {
    Collider* collider = objects[i]->has(Collider);
    if(collider != nullptr)
    {
      mTransforms[i] = collider->GetWorldTransform()->GetWorldMatrix();
      continue;
    }
    mTransforms[i] = objects[i]->has(Transform)->GetWorldMatrix();
  }
}

void JointCreator::ConnectionInfo::SetLocalPoints(Vec3Param localPoint0, Vec3Param localPoint1)
{
  mBodyRs[0] = localPoint0;
  mBodyRs[1] = localPoint1;
}

void JointCreator::ConnectionInfo::SetWorldPoint(Vec3Param worldPoint)
{
  mBodyRs[0] = Math::TransformPoint(mTransforms[0].Inverted(),worldPoint);
  mBodyRs[1] = Math::TransformPoint(mTransforms[1].Inverted(),worldPoint);
}

void JointCreator::ConnectionInfo::SetWorldPoints(Vec3Param worldPoint0, Vec3Param worldPoint1)
{
  mBodyRs[0] = Math::TransformPoint(mTransforms[0].Inverted(),worldPoint0);
  mBodyRs[1] = Math::TransformPoint(mTransforms[1].Inverted(),worldPoint1);
}

JointCreator::JointCreator()
{
  mFlags.Clear();
  mFlags.SetFlag(JointCreatorFlags::AttachToCommonParent);

  mLength = 2.0f;
  mMaxImpulse = 0.0f;
}

ZilchDefineType(JointCreator, builder, type)
{
  ZilchBindDestructor();
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();
  
  ZilchBindOverloadedMethod(Create, ZilchInstanceOverload(Cog*, Cog*, Cog*, StringParam));
  ZilchBindOverloadedMethod(CreateWorldPoints, ZilchInstanceOverload(Cog*, Cog*, Cog*, StringParam, Vec3Param));
  ZilchBindOverloadedMethod(CreateWorldPoints, ZilchInstanceOverload(Cog*, Cog*, Cog*, StringParam, Vec3Param, Vec3Param));
  ZilchBindOverloadedMethod(CreateLocalPoints, ZilchInstanceOverload(Cog*, Cog*, Cog*, StringParam, Vec3Param, Vec3Param));

  ZilchBindMethod(AddJointLimit);
  ZilchBindMethod(AddJointMotor);
  ZilchBindMethod(AddJointSpring);

  ZilchBindGetterSetterProperty(OverrideLength);
  ZilchBindGetterSetterProperty(UseCenter);
  ZilchBindGetterSetterProperty(AutoSnaps);
  ZilchBindGetterSetterProperty(AttachToWorld);
  ZilchBindGetterSetterProperty(AttachToCommonParent);
}

Cog* JointCreator::Create(Cog* objectA, Cog* objectB, StringParam jointName)
{
  if(!ObjectsValid(objectA, objectB, jointName))
    return nullptr;

  ConnectionInfo info(objectA, objectB, mFlags.IsSet(JointCreatorFlags::AttachToWorld));
  info.SetLocalPoints(Vec3::cZero, Vec3::cZero);
  info.mLength = mLength;
  return AttachInternal(info, jointName);
}

Cog* JointCreator::CreateWorldPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param bothWorldPoints)
{
  if(!ObjectsValid(objectA, objectB, jointName))
    return nullptr;

  ConnectionInfo info(objectA, objectB, mFlags.IsSet(JointCreatorFlags::AttachToWorld));
  info.SetWorldPoint(bothWorldPoints);
  info.mLength = mLength;
  return AttachInternal(info, jointName);
}

Cog* JointCreator::CreateWorldPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param worldPointA, Vec3Param worldPointB)
{
  if(!ObjectsValid(objectA, objectB, jointName))
    return nullptr;

  ConnectionInfo info(objectA, objectB, mFlags.IsSet(JointCreatorFlags::AttachToWorld));
  info.SetWorldPoints(worldPointA, worldPointB);
  info.mLength = mLength;
  return AttachInternal(info, jointName);
}

Cog* JointCreator::CreateLocalPoints(Cog* objectA, Cog* objectB, StringParam jointName, Vec3Param localPointA, Vec3Param localPointB)
{
  if(!ObjectsValid(objectA, objectB, jointName))
    return nullptr;

  ConnectionInfo info(objectA, objectB, mFlags.IsSet(JointCreatorFlags::AttachToWorld));
  info.SetLocalPoints(localPointA, localPointB);
  info.mLength = mLength;
  return AttachInternal(info, jointName);
}

Cog* JointCreator::AttachInternal(ConnectionInfo& info, StringParam jointName)
{
  ConfigureInfo(info);
  Cog* cog = CreateJoint(jointName, info);
  if(cog == nullptr)
    return nullptr;
  
  // Attach the joint to the common parent if it exists
  if(mFlags.IsSet(JointCreatorFlags::AttachToCommonParent))
  {
    Cog* cogA = info.a;
    Cog* cogB = info.b;
    Cog* commonParent = FindCommonParent(cogA, cogB);

    if(commonParent != nullptr)
      cog->AttachTo(commonParent);
  }

  Joint* joint = cog->has(Joint);

  if(jointName != "ObjectLink" && joint != nullptr)
  {
    SetBasicProperties(joint);
    CallJointFunctions(joint, info);
  }

  return cog;
}

Cog* JointCreator::FindCommonParent(Cog* cogA, Cog* cogB)
{
  Cog* commonParent = nullptr;

  if(cogA != nullptr && cogB != nullptr)
  {
    HashSet<Cog*> cogAParents;
    // Map the entire parent chain of A (including A as it could be the parent of B)
    while(cogA != nullptr)
    {
      cogAParents.Insert(cogA);
      cogA = cogA->GetParent();
    }
    // See if any node in the parent chain of B was a parent of A (including B itself)
    while(cogB != nullptr)
    {
      if(cogAParents.Contains(cogB))
      {
        commonParent = cogB;
        break;
      }
      cogB = cogB->GetParent();
    }
  }
  return commonParent;
}

JointLimit* JointCreator::AddJointLimit(Cog* joint)
{
  JointLimit* limit = new JointLimit();
  DefaultSerializer stream;
  limit->Serialize(stream);

  joint->AddComponent(limit);
  return limit;
}

JointMotor* JointCreator::AddJointMotor(Cog* joint)
{
  JointMotor* motor = new JointMotor();
  DefaultSerializer stream;
  motor->Serialize(stream);

  joint->AddComponent(motor);
  return motor;
}

JointSpring* JointCreator::AddJointSpring(Cog* joint)
{
  JointSpring* spring = new JointSpring();
  DefaultSerializer stream;
  spring->Serialize(stream);

  joint->AddComponent(spring);
  return spring;
}

bool JointCreator::GetOverrideLength() const
{
  return mFlags.IsSet(JointCreatorFlags::OverrideLength);
}

void JointCreator::SetOverrideLength(bool overrideLength)
{
  mFlags.SetState(JointCreatorFlags::OverrideLength, overrideLength);
}

bool JointCreator::GetUseCenter() const
{
  return mFlags.IsSet(JointCreatorFlags::UseCenter);
}

void JointCreator::SetUseCenter(bool useCenter)
{
  mFlags.SetState(JointCreatorFlags::UseCenter, useCenter);
}

bool JointCreator::GetAutoSnaps() const
{
  return mFlags.IsSet(JointCreatorFlags::AutoSnaps);
}

void JointCreator::SetAutoSnaps(bool autoSnaps)
{
  mFlags.SetState(JointCreatorFlags::AutoSnaps, autoSnaps);
}

bool JointCreator::GetAttachToWorld() const
{
  return mFlags.IsSet(JointCreatorFlags::AttachToWorld);
}

void JointCreator::SetAttachToWorld(bool attachToWorld)
{
  mFlags.SetState(JointCreatorFlags::AttachToWorld, attachToWorld);
}

bool JointCreator::GetAttachToCommonParent() const
{
  return mFlags.IsSet(JointCreatorFlags::AttachToCommonParent);
}

void JointCreator::SetAttachToCommonParent(bool attachToCommonParent)
{
  mFlags.SetState(JointCreatorFlags::AttachToCommonParent, attachToCommonParent);
}

bool JointCreator::ObjectsValid(Cog* a, Cog* b, StringParam jointName)
{
  if(a->GetMarkedForDestruction() || b->GetMarkedForDestruction())
    return false;

  Transform* transformA = a->has(Transform);
  Transform* transformB = b->has(Transform);
  // Need a transforms
  if(transformA == nullptr || transformB == nullptr)
    return false;

  if(jointName != "ObjectLink")
  {
    Collider* colliderA = a->has(Collider);
    Collider* colliderB = b->has(Collider);
    // Need a valid collider
    if(colliderA == nullptr || colliderB == nullptr)
      return false;
  }
  return true;
}

void JointCreator::ConfigureInfo(ConnectionInfo& info)
{
  info.mAttachToWorld = mFlags.IsSet(JointCreatorFlags::AttachToWorld);
  // Set to the center of the objects
  if(mFlags.IsSet(JointCreatorFlags::UseCenter))
  {
    info.mBodyRs[0] = Vec3::cZero;
    // Ignore the second object if we connect to the world
    if(!info.mAttachToWorld)
      info.mBodyRs[1] = Vec3::cZero;
  }

  // Get the world points
  info.mWorldPoints[0] = Math::TransformPoint(info.mTransforms[0], info.mBodyRs[0]);
  info.mWorldPoints[1] = Math::TransformPoint(info.mTransforms[1], info.mBodyRs[1]);

  // If this was a world connection, set the world's points
  // to the world point on object A
  if(info.mAttachToWorld)
    info.mBodyRs[1] = info.mWorldPoints[1] = info.mWorldPoints[0];

  mAxis = (info.mWorldPoints[1] - info.mWorldPoints[0]);

  float length = mAxis.AttemptNormalize();
  if(length == real(0.0))
  {
    mAxis = Vec3(0, 1, 0);
  }

  if(!mFlags.IsSet(JointCreatorFlags::OverrideLength))
    info.mLength = length;
}

Cog* JointCreator::CreateJoint(StringParam fileName, ConnectionInfo& info)
{
  // Create an object link, we dynamically add the appropriate joint type later
  Space* space = info.a->GetSpace();
  Cog* cog = space->Create(ArchetypeManager::Find(CoreArchetypes::ObjectLink));
  if(cog == nullptr)
    return nullptr;

  cog->ClearArchetype();
  ObjectLink* objLink = cog->has(ObjectLink);
  if(objLink == nullptr)
  {
    ErrorIf(true, "Joint data file %s did not contain a ObjectLink.", fileName.c_str());
    return nullptr;
  }

  objLink->SetCogAInternal(info.a);
  if(info.mAttachToWorld)
  {
    PhysicsSpace* physicsSpace = space->has(PhysicsSpace);
    objLink->SetObjectB(physicsSpace->mWorldCollider->GetOwner());
  }
  else
    objLink->SetObjectB(info.b);
  objLink->SetLocalPointA(info.mBodyRs[0]);
  objLink->SetLocalPointB(info.mBodyRs[1]);

  // Now that the object is fully created, we can dynamically add the appropriate
  // joint type (as long as it's not an object link). Using add component by name
  // so I don't have to figure out how to create the component from it's name.
  if(fileName != CoreArchetypes::ObjectLink)
    cog->AddComponentByName(fileName);
  return cog;
}

void JointCreator::SetBasicProperties(Joint* joint)
{
  // Set up the max impulse and whether or not it auto snaps
  joint->SetMaxImpulse(mMaxImpulse);
  joint->SetAutoSnaps(mFlags.IsSet(JointCreatorFlags::AutoSnaps));
}

#define JointType(type)                 \
case JointEnums::type##Type:            \
{                                       \
  CallJointFunctions<type>(joint,info); \
  break;                                \
}

void JointCreator::CallJointFunctions(Joint* joint, ConnectionInfo& info)
{
  //switch to determine the type of the joint
  switch(joint->GetJointType())
  {
#include "Physics/Joints/JointList.hpp"
  }
}

#undef JointType

void JointCreator::SetPointsAtLength(ConnectionInfo& info)
{
  info.mWorldPoints[0] += mAxis * real(.5) * info.mLength;
  info.mWorldPoints[1] -= mAxis * real(.5) * info.mLength;
  info.mBodyRs[0] = Math::TransformPoint(info.mTransforms[0].Inverted(), info.mWorldPoints[0]);
  info.mBodyRs[1] = Math::TransformPoint(info.mTransforms[1].Inverted(), info.mWorldPoints[1]);
}

void JointCreator::FixPoints(Joint* joint, ConnectionInfo& info)
{
  if(mFlags.IsSet(JointCreatorFlags::UseCenter))
    return;
  SetPointsAtLength(info);
}

void JointCreator::FixPoints(StickJoint* joint, ConnectionInfo& info)
{

}

void JointCreator::SpecificSetup(StickJoint* joint, ConnectionInfo& info)
{
  joint->SetLength(info.mLength);
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
}

void JointCreator::SpecificSetup(PositionJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
}

void JointCreator::SpecificSetup(ManipulatorJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPoint(info.mBodyRs[0]);
  joint->SetTargetPoint(info.mBodyRs[1]);
}

void JointCreator::SpecificSetup(RevoluteJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
  joint->SetWorldAxis(mAxis);
}

void JointCreator::SpecificSetup(PrismaticJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
  joint->SetWorldAxis(mAxis);
}

void JointCreator::SpecificSetup(WheelJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
  joint->SetWorldAxis(mAxis);
  joint->SetWorldShockAxis(Vec3::cYAxis);
}

void JointCreator::SpecificSetup(RevoluteJoint2d* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
}

void JointCreator::SpecificSetup(PrismaticJoint2d* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
  joint->SetWorldAxis(Vec3(mAxis.x, mAxis.y, 0));
}

void JointCreator::SpecificSetup(PhyGunJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPoint(info.mBodyRs[0]);
  joint->SetTargetPoint(info.mBodyRs[1]);

  Mat3 rot;
  Vec3 translation,scale;
  info.mTransforms[0].Decompose(&scale, &rot, &translation);
  joint->SetWorldRotation(Math::ToQuaternion(rot));
  joint->SetTargetRotation(Quat::cIdentity);
}

void JointCreator::SpecificSetup(WheelJoint2d* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);
  joint->SetWorldShockAxis(Vec3::cYAxis);
}

void JointCreator::SpecificSetup(WeldJoint* joint, ConnectionInfo& info)
{
  joint->SetLocalPointA(info.mBodyRs[0]);
  joint->SetLocalPointB(info.mBodyRs[1]);

  joint->SetLocalBasisA(info.a->has(Transform)->GetWorldRotation().Inverted());
  joint->SetLocalBasisB(info.b->has(Transform)->GetWorldRotation().Inverted());
}

}//namespace Zero
