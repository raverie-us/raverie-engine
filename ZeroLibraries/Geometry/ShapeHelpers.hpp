///////////////////////////////////////////////////////////////////////////////
///
/// \file ShapesHelpers.hpp
/// Contains conversion functions to and from shapes.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//--------------------------------------------------------- Conversion Functions
Ray ToRay(const Segment& segment);
Segment ToSegment(const Ray& ray, real t = real(1.0));

Aabb ToAabb(Vec3Param point);
Aabb ToAabb(const Aabb& aabb);
Aabb ToAabb(const Ray& ray, real t = real(1.0));
Aabb ToAabb(const Segment& segment);
Aabb ToAabb(const Triangle& tri);
Aabb ToAabb(const SweptTriangle& sweptTri);
Aabb ToAabb(const Obb& obb);
Aabb ToAabb(const Sphere& sphere);
Aabb ToAabb(const Ellipsoid& ellipsoid);
Aabb ToAabb(const Capsule& capsule);
Aabb ToAabb(const Cylinder& cylinder);
Aabb ToAabb(const Frustum& frustum);
Aabb ToAabb(const ConvexMeshShape& convexMesh);

Obb ToObb(Ellipsoid& ellipsoid);
Obb ToObb(const Cylinder& cylinder);
Obb ToObb(const Capsule& capsule);

Cylinder ToCylinder(const Capsule& capsule);
Capsule ToCapsule(const Cylinder& cylinder);

}//namespace Zero
