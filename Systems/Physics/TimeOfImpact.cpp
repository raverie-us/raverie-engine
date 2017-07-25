///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2015-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

using namespace Zero;

namespace
{

struct GeometryData
{
  Intersection::SupportShape shape;
  Vec3 pos;
  Quat rot;
  Vec3 rel;
  Vec3 vel;
  Vec3 axs;
  real ang;
};

typedef real (*RootFunc)(GeometryData*, GeometryData*, Vec3Param, real, real);
typedef void (*TimeOfImpactFunc)(TimeOfImpactData*);

TimeOfImpactFunc sTimeOfImpactLookup[Collider::cSize][Collider::cSize] = {{0}};

void InitTimeOfImpactLookup();
struct InvokeTimeOfImpactInit
{
  InvokeTimeOfImpactInit() {InitTimeOfImpactLookup();}
};

static const real sTolerance = 0.001;

void GetGeometryData(Collider* collider, GeometryData* data, bool linearSweep)
{
  data->shape = collider->GetSupportShape(true);
  data->shape.SetDeltaPosition(Vec3::cZero);
  data->shape.SetDeltaRotation(Quat::cIdentity);

  Transform* transform = collider->GetOwner()->has(Transform);
  RigidBody* rigidbody = collider->GetOwner()->has(RigidBody);

  data->pos = collider->GetWorldTranslation();
  Quat rot = transform->GetWorldRotation();

  if (rigidbody && !linearSweep)
  {
    data->vel = rigidbody->GetVelocity();
    data->axs = rigidbody->GetAngularVelocity();
    data->axs = Math::Multiply(rot, data->axs);
    data->ang = data->axs.AttemptNormalize();
  }
  else
  {
    data->vel = Vec3::cZero;
    data->axs = Vec3::cZero;
    data->ang = 0;
  }
}

void ComputeDeepestPoint(GeometryData* data, Vec3Param normal, real dt)
{
  Quat deltaRotInv = ToQuaternion(data->axs, -data->ang * dt);
  data->shape.Support(Math::Multiply(deltaRotInv, normal), &data->rel);
  data->rel -= data->pos + data->shape.GetDeltaPosition();
  data->rel = Math::Multiply(data->shape.GetDeltaRotation().Inverted(), data->rel);
}

Vec3 PointAtTime(GeometryData* data, real dt)
{
  data->rot = ToQuaternion(data->axs, data->ang * dt);
  return data->pos + data->vel * dt + Math::Multiply(data->rot, data->rel);
}

real Separation(GeometryData* objA, GeometryData* objB, Vec3Param normal, real dt)
{
  Vec3 pA = PointAtTime(objA, dt);
  Vec3 pB = PointAtTime(objB, dt);
  return normal.Dot(pB - pA);
}

real Bisection(GeometryData* objA, GeometryData* objB, Vec3Param normal, real tMin, real tMax)
{
  return (tMin + tMax) * 0.5f;
}

real FalsePosition(GeometryData* objA, GeometryData* objB, Vec3Param normal, real tMin, real tMax)
{
  real distA = Separation(objA, objB, normal, tMin);
  real distB = Separation(objA, objB, normal, tMax);

  real u = distA / (distA - distB);
  return tMin + (tMax - tMin) * u;
}

real FindRoot(GeometryData* objA, GeometryData* objB, Vec3Param normal, real tMin, real tMax, real lowerBound, real upperBound)
{
  ErrorIf(Separation(objA, objB, normal, tMin) < upperBound ||
          Separation(objA, objB, normal, tMax) > lowerBound, "No valid root exists!");

  real t, separation;

  // RootFunc rootFunc[] = {FalsePosition, Bisection};
  // uint funcIndex = 0;

  do
  {
    // t = rootFunc[++funcIndex % 2](objA, objB, normal, tMin, tMax);
    // Just bisection
    t = (tMin + tMax) * 0.5f;
    separation = Separation(objA, objB, normal, t);

    if (upperBound <= separation)
      tMin = t;
    else
      tMax = t;
  }
  while (separation <= lowerBound || upperBound <= separation);

  return t;
}

void SetSupportShapeDeltas(GeometryData* objA, GeometryData* objB, real dt)
{
  objA->shape.SetDeltaPosition(objA->vel * dt);
  objB->shape.SetDeltaPosition(objB->vel * dt);
  objA->shape.SetDeltaRotation(ToQuaternion(objA->axs, objA->ang * dt));
  objB->shape.SetDeltaRotation(ToQuaternion(objB->axs, objB->ang * dt));
}

bool BilateralAdvancement(GeometryData* objA, GeometryData* objB, real dt, real* impact, Intersection::Manifold* manifold)
{
  Intersection::Gjk gjk;
  Intersection::Type intersection;

  // Initial intersection test
  SetSupportShapeDeltas(objA, objB, 0);
  intersection = gjk.TestDebug(&objA->shape, &objB->shape, manifold);

  // If objects' initial configuration is intersecting, then maybe return some other error code
  // ...

  *impact = dt;
  real root = 0;

  // Can only find time of impact if not intersecting
  while (intersection == Intersection::None)
  {
    // Gjk returned no intersection at dt
    if (root == dt)
      return false;

    // Current axis of separation
    Vec3 normal = gjk.mSupportVector;

    real endTime = dt;
    while (true)
    {
      ComputeDeepestPoint(objA, normal, endTime - root);
      ComputeDeepestPoint(objB, -normal, endTime - root);

      real separationPos = Separation(objA, objB, normal, root);
      real separationNeg = Separation(objA, objB, normal, endTime);

      // A root exists between (0, sTolerance)
      if (separationNeg <= 0 && sTolerance <= separationPos)
      {
        endTime = FindRoot(objA, objB, normal, root, endTime, 0, sTolerance);
      }
      // Found root within separation threshold
      else if (separationNeg > 0)
      {
        break;
      }
      // Current separation within threshold, advance forward for contact data
      else
      {
        real relativeVelocity = normal.Dot(objB->vel)
                              + objB->rel.Cross(normal).Dot(objB->axs * objB->ang)
                              - normal.Dot(objA->vel)
                              - objA->rel.Cross(normal).Dot(objA->axs * objA->ang);

        // No relative velocity, objects are not getting closer on this axis
        if (relativeVelocity >= 0)
          return false;

        // Time step required to move just the epsilon difference
        real incTime = (separationPos + Intersection::Gjk::sEpsilon) / -relativeVelocity;

        // Step forward to find intersection
        real testTime = root;
        int maxIter = 3;
        while (testTime < dt && maxIter-- > 0)
        {
          testTime += incTime;

          SetSupportShapeDeltas(objA, objB, Math::Min(testTime, dt));
          intersection = gjk.TestDebug(&objA->shape, &objB->shape, manifold);
          if (intersection != Intersection::None)
          {
            *impact = root;
            return true;
          }
        }

        // Objects do not become intersecting
        *impact = dt;
        return false;
      }
    }

    // Advance support objects to new configuration
    root = endTime;

    SetSupportShapeDeltas(objA, objB, root);
    intersection = gjk.TestDebug(&objA->shape, &objB->shape, manifold);
  }

  *impact = root;
  return true;
}

template <typename ShapeTypeA, typename ShapeTypeB>
void TimeOfImpactInternal(TimeOfImpactData* data)
{
  GeometryData objA, objB;
  GetGeometryData(data->ColliderA, &objA, data->LinearSweep);
  GetGeometryData(data->ColliderB, &objB, data->LinearSweep);
  if (data->LinearSweep)
    objA.vel = data->Velocity;

  real impactTime;
  Intersection::Manifold iManifold;
  if (BilateralAdvancement(&objA, &objB, data->Dt, &impactTime, &iManifold))
  {
    ColliderPair pair;
    pair.A = data->ColliderA;
    pair.B = data->ColliderB;

    Physics::Manifold manifold;
    manifold.SetPair(pair);
    IntersectionToPhysicsManifold<ShapeTypeA, ShapeTypeB>(&iManifold, &manifold);


    data->ImpactTimes.PushBack(impactTime);
    data->Manifolds.PushBack(manifold);
  }
}

template <typename ComplexType, typename ShapeType, typename SpaceFunctor>
void TimeOfImpactComplexInternal(TimeOfImpactData* data, bool parametersSwapped)
{
  ComplexType* complexCollider = static_cast<ComplexType*>(data->ColliderA);

  GeometryData objA, objB;
  GetGeometryData(data->ColliderA, &objA, data->LinearSweep);
  GetGeometryData(data->ColliderB, &objB, data->LinearSweep);
  if(data->LinearSweep)
  {
    //when performing a linear sweep we always sweep the first collider,
    //however the collider order might've been swapped (to make it easier to test
    //against complex colliders). In that case swap who is doing the linear sweep.
    if(!parametersSwapped)
      objA.vel = data->Velocity;
    else
      objB.vel = data->Velocity;
  }

  Vec3 deltaVel = objB.vel - objA.vel;
  Sphere sphereB = data->ColliderB->mBoundingSphere;
  Capsule sweptSphere(sphereB.mCenter, sphereB.mCenter + deltaVel * data->Dt, sphereB.mRadius);
  SpaceFunctor functor(data->ColliderA);
  //this code only works when the complex object isn't rotating
  //(you basically can't midphase when the complex object is rotating)
  Aabb localAabb = functor.ToLocalAabb(ToAabb(sweptSphere));

  AutoDeclare(range, complexCollider->GetOverlapRange(localAabb));
  for(; !range.Empty(); range.PopFront())
  {
    AutoDeclare(shapeObject, range.Front());
    AutoDeclare(worldShape, functor.ToWorldShape(shapeObject.Shape));

    objA.shape = Intersection::MakeSupport(&worldShape, true);
    objA.shape.SetDeltaPosition(Vec3::cZero);
    objA.shape.SetDeltaRotation(Quat::cIdentity);
    objA.shape.GetCenter(&objA.pos);

    real impactTime;
    Intersection::Manifold iManifold;

    if(!BilateralAdvancement(&objA, &objB, data->Dt, &impactTime, &iManifold))
      continue;

    ColliderPair pair;
    pair.A = data->ColliderA;
    pair.B = data->ColliderB;

    Physics::Manifold manifold;
    manifold.SetPair(pair);
    IntersectionToPhysicsManifold<TypeOf(worldShape), ShapeType>(&iManifold, &manifold);
    manifold.ContactId = shapeObject.Index;
    FixInternalEdges(complexCollider, &manifold, shapeObject.Index);

    data->ImpactTimes.PushBack(impactTime);
    data->Manifolds.PushBack(manifold);
  }
}

template <typename ComplexType, typename ShapeType>
void TimeOfImpactComplex(TimeOfImpactData* data, bool parametersSwapped, true_type)
{
  TimeOfImpactComplexInternal<ComplexType, ShapeType, LocalFunctor>(data, parametersSwapped);
}

template <typename ComplexType, typename ShapeType>
void TimeOfImpactComplex(TimeOfImpactData* data, bool parametersSwapped, false_type)
{
  TimeOfImpactComplexInternal<ComplexType, ShapeType, WorldFunctor>(data, parametersSwapped);
}

template <typename ComplexType, typename ShapeType>
void TimeOfImpactComplexA(TimeOfImpactData* data)
{
  TimeOfImpactComplex<ComplexType, ShapeType>(data, false, typename ComplexType::RangeInLocalSpace());
}

template <typename ComplexType, typename ShapeType>
void TimeOfImpactComplexB(TimeOfImpactData* data)
{
  Math::Swap(data->ColliderA, data->ColliderB);

  TimeOfImpactComplex<ComplexType, ShapeType>(data, true, typename ComplexType::RangeInLocalSpace());
}

template <typename ColliderType0, typename ColliderType1, typename SpaceFunctor0, typename SpaceFunctor1>
void TimeOfImpactComplexVsComplexInternal(TimeOfImpactData* data)
{
  ColliderType0* complexCollider0 = static_cast<ColliderType0*>(data->ColliderA);
  ColliderType1* complexCollider1 = static_cast<ColliderType1*>(data->ColliderB);

  GeometryData objA, objB;
  GetGeometryData(data->ColliderA, &objA, data->LinearSweep);
  GetGeometryData(data->ColliderB, &objB, data->LinearSweep);
  if(data->LinearSweep)
    objA.vel = data->Velocity;

  typedef typename ColliderType0::RangeType Range0Type;
  typedef typename ColliderType1::RangeType Range1Type;

  SpaceFunctor0 functor0(complexCollider0);
  SpaceFunctor1 functor1(complexCollider1);

  //bring the swept sphere of collider1 into collider0's local space as an aabb
  Vec3 deltaVel = objB.vel - objA.vel;
  Sphere sphereB = complexCollider1->mBoundingSphere;
  Capsule sweptSphere1(sphereB.mCenter, sphereB.mCenter + deltaVel * data->Dt, sphereB.mRadius);
  //this code only works when the objects are not rotating
  //(you basically can't midphase when the complex object is rotating)
  Aabb aabb1InSpace0 = functor0.ToLocalAabb(ToAabb(sweptSphere1));

  //iterate over all sub-shapes in collider0 that could possibly intersect with collider1
  Range0Type r0 = complexCollider0->GetOverlapRange(aabb1InSpace0);
  for(; !r0.Empty(); r0.PopFront())
  {
    AutoDeclare(item0, r0.Front());
    //convert the shape to world space to perform collision detection against
    //(if manifolds is nullptr, we could keep this in local
    //space and just convert the collider shape once...)
    AutoDeclare(worldShape0, functor0.ToWorldShape(item0.Shape));
    //need to get the type of the world shape for IntersectionToPhysicsManifold
    typedef TypeOf(worldShape0) WorldShape0Type;

    //update the sub-shape for objectA
    objA.shape = Intersection::MakeSupport(&worldShape0, true);
    objA.shape.SetDeltaPosition(Vec3::cZero);
    objA.shape.SetDeltaRotation(Quat::cIdentity);
    objA.shape.GetCenter(&objA.pos);

    //compute the swept sphere for the sub-shape of objectA (from its aabb)
    Aabb shape0WorldAabb = ToAabb(worldShape0);
    Vec3 halfExtents = shape0WorldAabb.GetHalfExtents();
    real radius = Math::Max(halfExtents.x, Math::Max(halfExtents.y, halfExtents.z));
    Vec3 shape0Center = shape0WorldAabb.GetCenter();
    Capsule sweptSphere0(shape0Center, shape0Center - deltaVel * data->Dt, radius);

    //now we need to take each sub-shape in collider0 and check to
    //see what sub-shapes in collider1 they could hit
    //(note: this doesn't work if the objects were rotating)
    Aabb aabb0InSpace1 = ToAabb(functor1.ToLocalShape(ToAabb(sweptSphere0)));
    Range1Type r1 = complexCollider1->GetOverlapRange(aabb0InSpace1);
    for(; !r1.Empty(); r1.PopFront())
    {
      AutoDeclare(item1, r1.Front());
      //convert the shape to world space to perform collision detection against
      //(if manifolds is nullptr, we could keep this in local
      //space and just convert the collider shape once...)
      AutoDeclare(worldShape1, functor1.ToWorldShape(item1.Shape));
      //need to get the type of the world shape for IntersectionToPhysicsManifold
      typedef TypeOf(worldShape1) WorldShape1Type;

      //update the sub-shape for objectB
      objB.shape = Intersection::MakeSupport(&worldShape1, true);
      objB.shape.SetDeltaPosition(Vec3::cZero);
      objB.shape.SetDeltaRotation(Quat::cIdentity);
      objB.shape.GetCenter(&objB.pos);

      real impactTime;
      Intersection::Manifold iManifold;

      if(!BilateralAdvancement(&objA, &objB, data->Dt, &impactTime, &iManifold))
        continue;

      ColliderPair pair;
      pair.A = data->ColliderA;
      pair.B = data->ColliderB;

      Physics::Manifold manifold;
      manifold.SetPair(pair);
      IntersectionToPhysicsManifold<WorldShape0Type, WorldShape1Type>(&iManifold, &manifold);
      //set the id of this item on the manifold
      //(just lexicographically pack the two indices together for now)
      manifold.ContactId = item0.Index << 16;
      manifold.ContactId |= item1.Index;
      FixInternalEdges(complexCollider0, &manifold, item0.Index);
      FixInternalEdges(complexCollider1, &manifold, item1.Index);

      data->ImpactTimes.PushBack(impactTime);
      data->Manifolds.PushBack(manifold);
    }
  }
}

template <typename ColliderType0, typename ColliderType1>
void TimeOfImpactComplexVsComplex(TimeOfImpactData* data)
{
  bool type0Local = ColliderType0::RangeInLocalSpace::value;
  bool type1Local = ColliderType1::RangeInLocalSpace::value;
  //determine which collider needs a local space functor and which needs a world space functor
  //(could do some fancy template tricks to remove the if, but worry about that later)
  if(type0Local)
  {
    if(type1Local)
      TimeOfImpactComplexVsComplexInternal<ColliderType0, ColliderType1, LocalFunctor, LocalFunctor>(data);
    else
      TimeOfImpactComplexVsComplexInternal<ColliderType0, ColliderType1, LocalFunctor, WorldFunctor>(data);
  }
  else if(type1Local)
    TimeOfImpactComplexVsComplexInternal<ColliderType0, ColliderType1, WorldFunctor, LocalFunctor>(data);
  else
    TimeOfImpactComplexVsComplexInternal<ColliderType0, ColliderType1, WorldFunctor, WorldFunctor>(data);
}

void SetLookup(TimeOfImpactFunc func, uint colliderId1, uint colliderId2)
{
  sTimeOfImpactLookup[colliderId1][colliderId2] = func;
}

template <typename ShapeType>
void SetLookups(uint colliderId)
{
  SetLookup(TimeOfImpactInternal<ShapeType, Sphere>, colliderId, Collider::cSphere);
  SetLookup(TimeOfImpactInternal<ShapeType, Obb>, colliderId, Collider::cBox);
  SetLookup(TimeOfImpactInternal<ShapeType, Cylinder>, colliderId, Collider::cCylinder);
  SetLookup(TimeOfImpactInternal<ShapeType, Ellipsoid>, colliderId, Collider::cEllipsoid);
  SetLookup(TimeOfImpactInternal<ShapeType, Capsule>, colliderId, Collider::cCapsule);
  SetLookup(TimeOfImpactInternal<ShapeType, ConvexMeshShape>, colliderId, Collider::cConvexMesh);
}

template <typename ComplexType>
void SetComplexLookups(uint colliderId)
{
  SetLookup(TimeOfImpactComplexA<ComplexType, Sphere>, colliderId, Collider::cSphere);
  SetLookup(TimeOfImpactComplexB<ComplexType, Sphere>, Collider::cSphere, colliderId);

  SetLookup(TimeOfImpactComplexA<ComplexType, Obb>, colliderId, Collider::cBox);
  SetLookup(TimeOfImpactComplexB<ComplexType, Obb>, Collider::cBox, colliderId);

  SetLookup(TimeOfImpactComplexA<ComplexType, Cylinder>, colliderId, Collider::cCylinder);
  SetLookup(TimeOfImpactComplexB<ComplexType, Cylinder>, Collider::cCylinder, colliderId);

  SetLookup(TimeOfImpactComplexA<ComplexType, Ellipsoid>, colliderId, Collider::cEllipsoid);
  SetLookup(TimeOfImpactComplexB<ComplexType, Ellipsoid>, Collider::cEllipsoid, colliderId);

  SetLookup(TimeOfImpactComplexA<ComplexType, Capsule>, colliderId, Collider::cCapsule);
  SetLookup(TimeOfImpactComplexB<ComplexType, Capsule>, Collider::cCapsule, colliderId);

  SetLookup(TimeOfImpactComplexA<ComplexType, ConvexMeshShape>, colliderId, Collider::cConvexMesh);
  SetLookup(TimeOfImpactComplexB<ComplexType, ConvexMeshShape>, Collider::cConvexMesh, colliderId);
}

template <typename ComplexType>
void SetComplexVsComplexLookups(uint colliderId)
{
  SetLookup(TimeOfImpactComplexVsComplex<ComplexType, MultiConvexMeshCollider>, colliderId, Collider::cMultiConvexMesh);
  SetLookup(TimeOfImpactComplexVsComplex<MultiConvexMeshCollider, ComplexType>, Collider::cMultiConvexMesh, colliderId);

  SetLookup(TimeOfImpactComplexVsComplex<ComplexType, MeshCollider>, colliderId, Collider::cMesh);
  SetLookup(TimeOfImpactComplexVsComplex<MeshCollider, ComplexType>, Collider::cMesh, colliderId);

  //SetLookup(TimeOfImpactComplexVsComplex<ComplexType, HeightMapCollider>, colliderId, Collider::cHeightMap);
  //SetLookup(TimeOfImpactComplexVsComplex<HeightMapCollider, ComplexType>, Collider::cHeightMap, colliderId);
}

void InitTimeOfImpactLookup()
{
  SetLookups<Sphere>(Collider::cSphere);
  SetLookups<Obb>(Collider::cBox);
  SetLookups<Cylinder>(Collider::cCylinder);
  SetLookups<Ellipsoid>(Collider::cEllipsoid);
  SetLookups<Capsule>(Collider::cCapsule);
  SetLookups<ConvexMeshShape>(Collider::cConvexMesh);

  SetComplexLookups<MultiConvexMeshCollider>(Collider::cMultiConvexMesh);
  SetComplexLookups<MeshCollider>(Collider::cMesh);
  //SetComplexLookups<HeightMapCollider>(Collider::cHeightMap);

  SetComplexVsComplexLookups<MultiConvexMeshCollider>(Collider::cMultiConvexMesh);
}

InvokeTimeOfImpactInit sInvokeInit;

} // unnamed namespace

namespace Zero
{

void TimeOfImpact(TimeOfImpactData* data)
{
  TimeOfImpactFunc function = sTimeOfImpactLookup[data->ColliderA->mType][data->ColliderB->mType];

  if(function == nullptr)
  {
    Error("Time of impact function not implemented. Cannot collide objects.");
    return;
  }

  function(data);
}

} // namespace Zero
