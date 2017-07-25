///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

namespace
{
  Math::Random gRandom;
}

real contactBreakingThreshold = real(.02);

ManifoldPoint::ManifoldPoint()
{
  BodyPoints[0] = Vec3::cZero;
  BodyPoints[1] = Vec3::cZero;
  WorldPoints[0] = Vec3::cZero;
  WorldPoints[1] = Vec3::cZero;
  AccumulatedImpulse = Vec3::cZero;
  Penetration = real(0.0);
  Normal.ZeroOut();
}

Manifold::Manifold()
{
  ManifoldPolicy = AddingPolicy::NormalManifold;
  ContactCount = 0;
  ContactId = 0;
  DynamicFriction = real(0.0);
  Restitution = real(0.0);
}

Memory::Pool* Manifold::sManifoldPool = new Memory::Pool("Manifolds", Memory::GetNamedHeap("Physics"),
                                                         sizeof(Manifold), 2000);

void* Manifold::operator new(size_t size)
{
  return sManifoldPool->Allocate(size);
}
void Manifold::operator delete(void* pMem, size_t size)
{
  return sManifoldPool->Deallocate(pMem, size);
}

ManifoldPoint Manifold::GetPoint(uint index)
{
  return Contacts[index];
}

void Manifold::SetPair(const ColliderPair& pair)
{
  Objects = pair;

  Restitution = Objects.GetMixedRestiution();
  DynamicFriction = Objects.GetMixedFriction();
}

void Manifold::SwapPair()
{
  Math::Swap(Objects[0],Objects[1]);
  
  for(uint i = 0; i < ContactCount; ++i)
  {
    Contacts[i].Normal = -Contacts[i].Normal;
    Math::Swap(Contacts[i].BodyPoints[0], Contacts[i].BodyPoints[1]);
    Math::Swap(Contacts[i].WorldPoints[0], Contacts[i].WorldPoints[1]);
  }
}

real Manifold::GetSeparatingVelocity(uint contactIndex)
{
  ManifoldPoint& point = Contacts[contactIndex];
  Vec3Ref worldPointA = point.WorldPoints[0];
  return Math::Dot(point.Normal, Objects.GetPointSeperatingVelocity(worldPointA));
}

Vec3 Manifold::GetSeparatingTangentVelocity(uint contactIndex)
{
  ManifoldPoint& point = Contacts[contactIndex];
  Vec3Ref worldPointA = point.WorldPoints[0];
  Vec3 separatingVelocity = Objects.GetPointSeperatingVelocity(worldPointA);
  return separatingVelocity - Dot(separatingVelocity, point.Normal) * point.Normal;
}

void Manifold::GetTangents(PhysicsSolverConfig* config, uint contactIndex, 
                           Vec3Ref t1, Vec3Ref t2)
{
  ManifoldPoint& point = Contacts[contactIndex];
  if(config->mTangentType == PhysicsContactTangentTypes::OrthonormalTangents)
    GenerateOrthonormalBasis(point.Normal, &t1, &t2);
  else if(config->mTangentType == PhysicsContactTangentTypes::VelocityTangents)
  {
    t1 = GetSeparatingTangentVelocity(contactIndex);
    AttemptNormalize(t1);

    if(t1.LengthSq() < real(0.01))
    {
      GenerateOrthonormalBasis(point.Normal, &t1, &t2);
      return;
    }

    //compute the second tangent vector
    t2 = Cross(point.Normal, t1);
    AttemptNormalize(t2);
  }
  else if(config->mTangentType == PhysicsContactTangentTypes::RandomTangents)
  {
    do 
    {
      Vec3 randVec(gRandom.Float() - .5f, gRandom.Float() - .5f, gRandom.Float() - .5f);
      t1 = Math::Cross(randVec, point.Normal);
    } while(!t1.Length());

    t2 = Math::Cross(point.Normal, t1);

    t1.Normalize();
    t2.Normalize();

    ErrorIf(Math::Abs(Math::Dot(point.Normal, t1)) > real(0.01), "");
    ErrorIf(Math::Abs(Math::Dot(t1, t2)) > real(0.01),"");
    ErrorIf(Math::Abs(Math::Dot(point.Normal, t2)) > real(0.01), "");
  }
  else
    ErrorIf(true, "Invalid Tangent type specified");
}

int Manifold::FindWorldPoint(Vec3Param worldPointA)
{
  Vec3 localPoint = JointHelpers::WorldPointToBodyR(Objects[0], worldPointA);
  return FindLocalPoint(localPoint);
}

int Manifold::FindLocalPoint(Vec3Param localPointA)
{
  int closestIndex = -1;
  real closestDistance = real(.005);//cachingThreshold;

  //find the point with the closest distance
  for(uint i = 0; i < ContactCount; ++i)
  {
    real sqDistance = (localPointA - Contacts[i].BodyPoints[0]).LengthSq();
    if(sqDistance < closestDistance)
    {
      closestDistance = sqDistance;
      closestIndex = static_cast<int>(i);
    }
  }
  return closestIndex;
}

void Manifold::SetNormal(Vec3Param normal)
{
  for(uint i = 0; i < ContactCount; ++i)
    Contacts[i].Normal = normal;
}

void Manifold::Clear()
{
  ContactCount = 0;

  for(uint i = 0; i < cMaxContacts; ++i)
    Contacts[i] = ManifoldPoint();
}

void Manifold::SetPolicy(AddingPolicy::Enum policy)
{
  ManifoldPolicy.Clear();
  ManifoldPolicy.SetFlag(policy);
}

void TestWorldToBodyToWorld(Vec3Param point, Collider* collider)
{
  Vec3 bodyPoint = JointHelpers::WorldPointToBodyR(collider, point);
  Vec3 worldPoint = JointHelpers::BodyRToWorldPoint(collider, bodyPoint);
  ErrorIf((worldPoint - point).LengthSq() > real(.01),
          "World to body to world transform failed.");
}

void Manifold::AddPoints(ManifoldPoint* points, uint count)
{
  for(uint i = 0; i < count; ++i)
  {
    points[i].BodyPoints[0] = JointHelpers::WorldPointToBodyR(Objects[0], points[i].WorldPoints[0]);
    points[i].BodyPoints[1] = JointHelpers::WorldPointToBodyR(Objects[1], points[i].WorldPoints[1]);
  }

  if(ManifoldPolicy == AddingPolicy::NormalManifold)
    NormalAdd(points, count);
  else if(ManifoldPolicy == AddingPolicy::FullManifold)
    FullAdd(points, count);
  else if(ManifoldPolicy == AddingPolicy::PersistentManifold)
    PersistentAdd(points, count);
  
}

bool Manifold::GetSendsMessages() const
{
  return Objects[0]->GetSendsEvents() | Objects[1]->GetSendsEvents();
}

void Manifold::ReplaceNormal(uint contactIndex, Vec3Param normal)
{
  Vec3 offset = Objects[1]->GetWorldTranslation() - Objects[0]->GetWorldTranslation();
  Vec3 alteredNormal = normal;
  if(Math::Dot(offset, normal) < real(0.0))
    alteredNormal *= -1;

  Contacts[contactIndex].Normal = alteredNormal;
}

bool Manifold::CorrectFor2D()
{
  Collider* collider1 = Objects[0];
  Collider* collider2 = Objects[1];

  //If either collider is ghost, we don't have to worry about having bad normals.
  //In fact, it's more important that we don't remove contact points and
  //end up with a manifold of 0 points.
  if(collider1->GetGhost() || collider2->GetGhost())
    return true;

  //we want to correct the normal between two objects if they are both
  //2d or one of them is 2d and the other is not dynamic.
  if((collider1->Is2D() && collider2->Is2D()) ||
     (collider1->Is2D() && !collider2->IsDynamic()) ||
     (collider2->Is2D() && !collider1->IsDynamic()) )
  {
    //now loop through all of the contacts
    //(loop backwards because it's easier to deal with removing some). 
    for(int i = ContactCount - 1; i >= 0; --i)
    {
      //remove the z axis (hardcoded for 2d right now) 
      //and see how long the new vector is
      Contacts[i].Normal.z = real(0.0);
      real length = Contacts[i].Normal.AttemptNormalize();

      //if the new vector was almost all in the z, we want to just remove this
      //contact point so that we don't have a close to zero vector during
      //resolution. When removing it though, we need to make sure to shuffle down
      //all good contact points so we don't have any gaps of empty contacts.
      if(length < real(.03))
      {
        for(uint j = i + 1; j < ContactCount; ++j)
          Contacts[j - 1] = Contacts[j];
        //we now have 1 less contact since we removed one
        --ContactCount;
      }
    }

    //if we have no points left, then this is an invalid
    //manifold that should be removed, so return false.
    if(ContactCount == 0)
      return false;
  }

  //one way or another, this manifold has valid points left, so don't remove it
  return true;
}

void Manifold::AddPoint(ManifoldPoint& point)
{
  //find if this point already exists
  int insertIndex = FindLocalPoint(point.BodyPoints[0]);
  if(insertIndex == -1)
  {
    //if it doesn't exist, check if we have no more room for contacts
    if(ContactCount == cMaxContacts)
    {
      //if we have no more room, find the point to remove
      insertIndex = SortCachedPoints(point.BodyPoints[0]);
    }
    else
      insertIndex = ContactCount++;

    //set the new point position and send the message that a new one was added
    SetPoint(insertIndex, point);
  }
  else
  {
    //otherwise the point existed, so just update the
    //old point while keeping the cached impulse
    Vec3 impulses = Contacts[insertIndex].AccumulatedImpulse;
    SetPoint(insertIndex, point);
    Contacts[insertIndex].AccumulatedImpulse = impulses;
  }
}

void Manifold::SetPoint(uint insertIndex, ManifoldPoint& point)
{
  Contacts[insertIndex].WorldPoints[0] = point.WorldPoints[0];
  Contacts[insertIndex].WorldPoints[1] = point.WorldPoints[1];
  Contacts[insertIndex].BodyPoints[0] = point.BodyPoints[0];
  Contacts[insertIndex].BodyPoints[1] = point.BodyPoints[1];
  Contacts[insertIndex].Normal = point.Normal;
  Contacts[insertIndex].Penetration = point.Penetration;
  Contacts[insertIndex].AccumulatedImpulse.ZeroOut();
}

void Manifold::RemovePoint(uint index)
{
  if(index != ContactCount - 1)
    Contacts[index] = Contacts[ContactCount - 1];

  Contacts[ContactCount - 1] = ManifoldPoint();
  --ContactCount;
}

real Manifold::ComputeQuadArea(Vec3Param point1, Vec3Param point2, Vec3Param point3, Vec3Param point4)
{
  Vec3 A = point1;
  Vec3 B = point2;
  Vec3 C = point3;
  Vec3 D = point4;

  /*Vec3 signedArea1 = Math::Cross(B - A,D - A);
  Vec3 signedArea2 = Math::Cross(B - C,D - C);
  real sign1 = Math::Dot(Normals[0], signedArea1);
  real sign2 = Math::Dot(Normals[0], signedArea2);

  if(sign1 < 0 && sign2 > 0 || sign2 < 0 && sign1 > 0)
  {
    Vec3 temp = A;
    A = B;
    B = temp;
  }*/

  Vec3 AC = (C - A) * real(.5);
  Vec3 BD = (D - B) * real(.5);
  return Math::Cross(AC, BD).LengthSq();
}

uint Manifold::SortCachedPoints(Vec3Param bodyPointA)
{
  int maxPenetrationIndex = -1;
  real maxPenetration = real(0.0);
  //Find and keep the deepest point
  for(uint i = 0; i < cMaxContacts; ++i)
  {
    if(Contacts[i].Penetration > maxPenetration)
    {
      maxPenetration = Contacts[i].Penetration;
      maxPenetrationIndex = i;
    }
  }

  Vec4 areas = Vec4::cZero;

  //calculate the areas of each quad to determine which points to keep
  if(maxPenetrationIndex != 0)
  {
    areas[0] = ComputeQuadArea(bodyPointA, Contacts[1].BodyPoints[0],
                               Contacts[2].BodyPoints[0], Contacts[3].BodyPoints[0]);
  }
  if(maxPenetrationIndex != 1)
  {
    areas[1] = ComputeQuadArea(bodyPointA, Contacts[0].BodyPoints[0],
      Contacts[2].BodyPoints[0], Contacts[3].BodyPoints[0]);
  }
  if(maxPenetrationIndex != 2)
  {
    areas[2] = ComputeQuadArea(bodyPointA, Contacts[0].BodyPoints[0],
      Contacts[1].BodyPoints[0], Contacts[3].BodyPoints[0]);
  }
  if(maxPenetrationIndex != 3)
  {
    areas[3] = ComputeQuadArea(bodyPointA, Contacts[0].BodyPoints[0],
      Contacts[1].BodyPoints[0], Contacts[2].BodyPoints[0]);
  }

  uint index;
  if(areas[0] > areas[1])
  {
    if(areas[0] > areas[2])
    {
      if(areas[0] > areas[3])
        index = 0;
      else
        index = 3;
    }
    else if(areas[2] > areas[3])
      index = 2;
    else
      index = 3;
  }
  else if(areas[1] > areas[2])
  {
    if(areas[1] > areas[3])
      index = 1;
    else
      index = 3;
  }
  else if(areas[2] > areas[3])
    index = 2;
  else
    index = 3;
  return index;
}

void Manifold::RefreshPoints()
{
  for(int i = ContactCount - 1; i >= 0; --i)
  {
    ManifoldPoint& point = Contacts[i];
    point.WorldPoints[0] = JointHelpers::BodyRToWorldPoint(Objects[0], point.BodyPoints[0]);
    point.WorldPoints[1] = JointHelpers::BodyRToWorldPoint(Objects[1], point.BodyPoints[1]);
    point.Penetration = Math::Dot(point.WorldPoints[0] - point.WorldPoints[1], point.Normal);

    if(point.Penetration < -contactBreakingThreshold)
      RemovePoint(i);
    else
    {
      Vec3 projectedPoint = point.WorldPoints[0] - point.Normal * point.Penetration;
      Vec3 projectedDist = projectedPoint - point.WorldPoints[1];
      real perpDist = projectedDist.LengthSq();
      if(perpDist > contactBreakingThreshold * contactBreakingThreshold)
        RemovePoint(i);
    }
  }
}

void Manifold::NormalAdd(ManifoldPoint* points, uint count)
{
  ContactCount = count;
  for(uint i = 0; i < count; ++i)
    Contacts[i] = points[i];
}

void Manifold::FullAdd(ManifoldPoint* points, uint count)
{
  //cache the old points
  ManifoldPoint cachedPoints[cMaxContacts];
  uint cachedPointCount = ContactCount;
  memcpy(cachedPoints, Contacts, ContactCount * sizeof(ManifoldPoint));

  //keep track of which points have persisted for knowing what was 
  //removed and also for quit outs in checking for a cached point
  bool persisted[cMaxContacts] = {false};

  ContactCount = count;
  for(uint i = 0; i < ContactCount; ++i)
  {
    bool found = false;//marks if this new point is being persisted
    ManifoldPoint& manifoldPoint = Contacts[i];
    SetPoint(i,points[i]);

    //loop over the cached points to see if we have a persistent point
    for(uint cachedId = 0; cachedId < cachedPointCount; ++cachedId)
    {
      //if this cached value has already been persisted, this can't be the point we're looking for
      if(persisted[cachedId])
        continue;

      ManifoldPoint& cachedPoint = cachedPoints[cachedId];
      //if the new point equals the cached point

      real distanceSq = (cachedPoint.WorldPoints[0] - manifoldPoint.WorldPoints[0]).LengthSq();

      if(distanceSq < real(.005))
      {
        //mark this point as persisted
        persisted[cachedId] = true;
        //found marks that it was persisted but from the new point's view
        found = true;

        manifoldPoint.AccumulatedImpulse = cachedPoint.AccumulatedImpulse;
        break;
      }
    }
  }
}

void Manifold::PersistentAdd(ManifoldPoint* points, uint count)
{
  RefreshPoints();

  uint maxDepthIndex = static_cast<uint>(-1);
  real maxDepth = -1.f;
  for(uint i = 0; i < count; ++i)
  {
    int insertIndex = FindLocalPoint(points[i].BodyPoints[0]);
    if(insertIndex == -1)
    {
      if(points[i].Penetration > maxDepth)
      {
        maxDepth = points[i].Penetration;
        maxDepthIndex = i;
      }
      //if(ContactCount == cMaxContacts)
      //  insertIndex = SortCachedPoints(point.BodyPoints[0]);
      //else
      //  insertIndex = ContactCount++;

      //SetPoint(insertIndex, point);
    }
    else
    {
      Vec3 impulses = Contacts[insertIndex].AccumulatedImpulse;
      SetPoint(insertIndex, points[i]);
      Contacts[insertIndex].AccumulatedImpulse = impulses;
    }
  }

  if(maxDepthIndex != static_cast<uint>(-1))
    AddPoint(points[maxDepthIndex]);
  //AddPoint(points[maxDepthIndex]);
}

}//namespace Physics

}//namespace Zero
