///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//-------------------------------------------------------------------PhysicsCar
ZilchDefineType(PhysicsCar, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();

  ZeroBindDependency(RigidBody);

  ZilchBindFieldProperty(mActive);
  ZilchBindGetterSetterProperty(DebugDraw);

  ZilchBindFieldProperty(mMaxSpeed);
  ZilchBindFieldProperty(mMaxTorque);
  ZilchBindFieldProperty(mGripScalar);
  ZilchBindFieldProperty(mAntiLockBrakes);
  ZilchBindFieldProperty(mTorqueGovernor);
  ZilchBindFieldProperty(mWheelFrictionSideRollCoef)->Add(new EditorRange(0, 1, 0.001));
  ZilchBindFieldProperty(mWheelFrictionFrontRollCoef)->Add(new EditorRange(0, 1, 0.001));

  ZilchBindGetterSetter(Steer);
  ZilchBindGetterSetter(Gas);
  ZilchBindGetterSetter(Brake);

  ZilchBindFieldGetter(mWheelCogs);
  ZilchBindMethod(NumberOfWheelsInContact);

  ZeroBindTag(Tags::Physics);
}

PhysicsCar::PhysicsCar()
{
  mBody = nullptr;
  mSpace = nullptr;
  mWheelCogs.mCarBody = this;

  mSteerInput = real(0.0);
  mGasInput = real(0.0);
  mBrakeInput = real(0.0);

  mFlags.Clear();
}

void PhysicsCar::Serialize(Serializer& stream)
{
  SerializeNameDefault(mWheelFrictionSideRollCoef, real(0));
  SerializeNameDefault(mWheelFrictionFrontRollCoef, real(0));

  SerializeNameDefault(mMaxSpeed, real(30));
  SerializeNameDefault(mMaxTorque, real(300));
  SerializeNameDefault(mGripScalar, real(1));
  SerializeNameDefault(mAntiLockBrakes, false);
  SerializeNameDefault(mTorqueGovernor, false);
  SerializeNameDefault(mActive, true);

  uint mask = CarFlags::InEditor | CarFlags::OnAllObjectsCreatedCalled;
  SerializeBits(stream, mFlags, CarFlags::Names, mask, 0);
}

void PhysicsCar::Initialize(CogInitializer& initializer)
{
  mSpace = initializer.mSpace->has(PhysicsSpace);
  ErrorIf(!mSpace, "PhysicsCar parent has no physics space.");

  mSpace->AddComponent(this);

  mBody = GetOwner()->has(RigidBody);

  if(initializer.mSpace->IsEditorMode())
    mFlags.SetFlag(CarFlags::InEditor);

  Transform* transform = GetOwner()->has(Transform);
  mWorldTransform.Set(transform->GetWorldMatrix());
}

void PhysicsCar::OnAllObjectsCreated(CogInitializer& initializer)
{
  mFlags.SetFlag(CarFlags::OnAllObjectsCreatedCalled);
}

void PhysicsCar::OnDestroy(uint flags)
{
  // When we get destroyed, unlink each wheel from us
  ClearWheels();
  mSpace->RemoveComponent(this);
}

void PhysicsCar::DebugDraw()
{
  //debug draw each wheel as long as it is valid
  // WheelCollection::WheelArray& wheels = mWheelCollection.mWheels;
  // for(uint i = 0; i < wheels.Size(); ++i)
  // {
  //   if(wheels[i].mCarWheel != nullptr)
  //     wheels[i].mCarWheel->DebugDraw();
  // }
}

void PhysicsCar::TransformUpdate(TransformUpdateInfo& info)
{
  // We want to move our wheels if we were moved in the editor,
  // but we do not want to do this if we are actually running in the game
  if(!mFlags.IsSet(CarFlags::InEditor) || !mFlags.IsSet(CarFlags::OnAllObjectsCreatedCalled))
    return;

  // If we have wheels to update, forcibly update the physics transform
  // so that the wheels will be properly updated
  if(mWheelRefs.Size() != 0)
  {
    Transform* transform = GetOwner()->has(Transform);
    mWorldTransform.Set(transform->GetWorldMatrix());
  }

  // Update each wheel's position
  for(uint i = 0; i < mWheelRefs.Size(); ++i)
  {
    PhysicsCarWheel* carWheel = mWheelRefs[i].GetCarWheel();
    if(carWheel != nullptr)
      carWheel->UpdateTransformRelativeToParent();
  }
}

void PhysicsCar::AddWheelCog(Cog* wheelCog)
{
  CarWheelRef& wheelRef = mWheelRefs.PushBack();
  wheelRef.mWheelId = wheelCog;
  PhysicsCarWheel* carWheel = wheelRef.GetCarWheel();
  if(carWheel != nullptr)
  {
    carWheel->mCarBody = this;
    carWheel->UpdateLocalPointOnCar(true);
    carWheel->UpdatePreRotationOnCar(true);
  }
}

void PhysicsCar::RemoveWheelCog(Cog* wheelCog)
{
  if(wheelCog == nullptr)
    return;

  for(size_t i = 0; i < mWheelRefs.Size(); ++i)
  {
    Cog* checkingCog = mWheelRefs[i].GetCog();
    if(checkingCog != wheelCog)
      continue;

    PhysicsCarWheel* carWheel = checkingCog->has(PhysicsCarWheel);
    if(carWheel != nullptr)
      carWheel->mCarBody = nullptr;

    mWheelRefs.EraseAt(i);
    break;
  }
}

void PhysicsCar::ClearWheels()
{
  for(size_t i = 0; i < mWheelRefs.Size(); ++i)
  {
    PhysicsCarWheel* carWheel = mWheelRefs[i].GetCarWheel();
    if(carWheel != nullptr)
      carWheel->mCarBody = nullptr;
  }
  mWheelRefs.Clear();
}

void PhysicsCar::Update(real dt)
{
  if(mActive == false)
    return;

  Transform* transform = GetOwner()->has(Transform);
  mWorldTransform.Set(transform->GetWorldMatrix());

  if(!mBody->IsDynamic())
    return;

  CacheActiveWheels();
  UpdateWheelData();
  CastWheelPositions();
  CalculateSpringForces();
  ApplySpringForces(dt);

  // Calculate and apply friction. This is a solver, so there are multiple
  // iterations internally. A begin and end step are needed so that we can
  // setup before and then calculated some end results.
  BeginFrictionCalculations(dt);
  CalculateFrictionImpulses(dt);
  EndFrictionCalculations(dt);

  if(mFlags.IsSet(CarFlags::DebugDraw))
    DebugDraw();
}

void PhysicsCar::UpdatePositions(real dt)
{
  if(mActive == false)
    return;

  UpdateWheelTransforms(dt);
}

void PhysicsCar::CacheActiveWheels()
{
  mActiveWheels.Clear();
  for(size_t i = 0; i < mWheelRefs.Size(); ++i)
  {
    PhysicsCarWheel* carWheel = mWheelRefs[i].GetCarWheel();
    if(carWheel == nullptr)
      continue;

    carWheel->mCarBody = this;
    if(carWheel->GetActive())
      mActiveWheels.PushBack(carWheel);
  }
}

void PhysicsCar::UpdateWheelData()
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->UpdateWheelData();
}

void PhysicsCar::CastWheelPositions()
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->CastWheelPosition();
}

void PhysicsCar::CalculateSpringForces()
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->CalculateSpringForce();
}

void PhysicsCar::ApplySpringForces(real dt)
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->ApplySpringForce(dt);
}

void PhysicsCar::BeginFrictionCalculations(real dt)
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->BeginIteration();
}

void PhysicsCar::CalculateFrictionImpulses(real dt)
{
  // Iterate over all the wheels multiple times to converge
  // to a global answer (hardcoded 5 for now...)
  for(uint iteration = 0; iteration < 5; ++iteration)
  {
    for(size_t i = 0; i < mActiveWheels.Size(); ++i)
      mActiveWheels[i]->SolveFrictionImpulse(dt);
  }
}

void PhysicsCar::EndFrictionCalculations(real dt)
{
  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->FinishedIteration(dt);
}

void PhysicsCar::UpdateWheelTransforms(real dt)
{
  // Use the transform's world matrix here because our body's cached transform
  // is a frame old since this happened after position integration.
  Transform* t = GetOwner()->has(Transform);
  mWorldTransform.Set(t->GetWorldMatrix());

  for(size_t i = 0; i < mActiveWheels.Size(); ++i)
    mActiveWheels[i]->UpdateWheelTransform(dt);
}

bool PhysicsCar::GetDebugDraw()
{
  return mFlags.IsSet(CarFlags::DebugDraw);
}

void PhysicsCar::SetDebugDraw(bool state)
{
  mFlags.SetState(CarFlags::DebugDraw,state);
}

real PhysicsCar::GetSteer()
{
  return mSteerInput;
}

void PhysicsCar::SetSteer(real steer)
{
  // Steer values range from -1 to 1 (left to right)
  mSteerInput = Math::Clamp(steer, real(-1.0), real(1.0));
}

real PhysicsCar::GetGas()
{
  return mGasInput;
}

void PhysicsCar::SetGas(real gas)
{
  // Gas values range from -1 to 1 (reverse to forward)
  mGasInput = Math::Clamp(gas, real(-1.0), real(1.0));
}

real PhysicsCar::GetBrake()
{
  return mBrakeInput;
}

void PhysicsCar::SetBrake(real brake)
{
  // Brake values range from 0 to 1 (no brake to full brake)
  mBrakeInput = Math::Clamp(brake, real(0.0), real(1.0));
}

uint PhysicsCar::NumberOfWheelsInContact()
{
  uint numberInContact = 0;
  for(uint i = 0; i < mActiveWheels.Size(); ++i)
  {
    if(mActiveWheels[i]->GetIsInContact())
      ++numberInContact;
  }
  return numberInContact;
}

//-------------------------------------------------------------------CarWheelRef
ZilchDefineType(PhysicsCar::CarWheelRef, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
}

Cog* PhysicsCar::CarWheelRef::GetCog()
{
  return mWheelId;
}

PhysicsCarWheel* PhysicsCar::CarWheelRef::GetCarWheel()
{
  Cog* cog = GetCog();
  if(cog == nullptr)
    return nullptr;

  return cog->has(PhysicsCarWheel);
}

//-------------------------------------------------------------------CarWheelArray
ZilchDefineType(PhysicsCar::CarWheelArray, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(Get);
  ZilchBindGetter(Count);
}

PhysicsCar::CarWheelArray::CarWheelArray()
{
  mCarBody = nullptr;
}

Cog* PhysicsCar::CarWheelArray::Get(int index)
{
  int count = mCarBody->mWheelRefs.Size();
  if(index >= count)
  {
    String msg = String::Format("Index %d is invalid. Array only contains %d elements.", index, count);
    DoNotifyException("Invalid index", msg);
    return nullptr;
  }
  return mCarBody->mWheelRefs[index].mWheelId;
}

int PhysicsCar::CarWheelArray::GetCount()
{
  return mCarBody->mWheelRefs.Size();
}

}//namespace Zero
