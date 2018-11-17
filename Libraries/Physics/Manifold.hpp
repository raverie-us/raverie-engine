///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

const uint cMaxContacts = 4;

namespace Zero
{

namespace Memory{ class Pool; }
class CollisionEvent;
class PhysicsSolverConfig;

namespace Physics
{

struct Manifold;

struct ManifoldPoint
{
  ManifoldPoint();

  Vec3 BodyPoints[2];
  Vec3 WorldPoints[2];

  Vec3 Normal;

  Vec3 AccumulatedImpulse;

  real Penetration;
};

DeclareBitField3(AddingPolicy, FullManifold, 
                               PersistentManifold, 
                               NormalManifold);

struct Manifold
{
  Manifold();

  static Memory::Pool* sManifoldPool;
  OverloadedNew();
  
  ManifoldPoint GetPoint(uint index);
  
  int FindWorldPoint(Vec3Param worldPointA);
  int FindLocalPoint(Vec3Param localPointA);

  void SetPair(const ColliderPair& pair);
  void SwapPair();
  ///Used to get the separating velocity of the two points along the normal.
  real GetSeparatingVelocity(uint contactIndex);
  Vec3 GetSeparatingTangentVelocity(uint contactIndex);
  void GetTangents(PhysicsSolverConfig* config, uint contactIndex, 
                   Vec3Ref t1, Vec3Ref t2);

  void SetNormal(Vec3Param normal);
  void Clear();

  //would like to make different add policies or something
  void SetPolicy(AddingPolicy::Enum policy);
  void AddPoints(ManifoldPoint* points, uint count);

  bool GetSendsMessages() const;
  void ReplaceNormal(uint contactIndex, Vec3Param normal);
  //Removes the z portion of the normals if this is a 2d case. Removes any
  //degenerate contacts that are left after removing their z axis. Returns
  //false if there are no points left and this manifold should be removed.
  bool CorrectFor2D();

private:

  void AddPoint(ManifoldPoint& point);
  void SetPoint(uint insertIndex, ManifoldPoint& point);
  void RemovePoint(uint index);
  real ComputeQuadArea(Vec3Param A, Vec3Param B, Vec3Param C, Vec3Param D);
  uint SortCachedPoints(Vec3Param localPointA);
  void RefreshPoints();
  void NormalAdd(ManifoldPoint* points, uint count);
  void FullAdd(ManifoldPoint* points, uint count);
  void PersistentAdd(ManifoldPoint* points, uint count);

public:

  BitField<AddingPolicy::Enum> ManifoldPolicy;
  ColliderPair Objects;

  uint ContactCount;
  uint ContactId;
  ManifoldPoint Contacts[cMaxContacts];
  real DynamicFriction;
  real Restitution;
};

typedef PodArray<Manifold> ManifoldArray;

}//namespace Physics

}//namespace Zero
