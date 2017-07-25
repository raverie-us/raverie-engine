///////////////////////////////////////////////////////////////////////////////
///
/// \file ExtendedCollision.hpp
/// NSquared intersection functions for the shape primitives. Wraps the
/// internal intersection functions for ease of use.
///
/// Authors: Joshua Davis, Auto-Generated
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

void FlipManifoldInfo(Intersection::Manifold* manifold);
void FlipSupportShapeManifoldInfo(const Intersection::SupportShape& a,
                                  const Intersection::SupportShape& b,
                                  Intersection::Manifold* manifold);
bool SupportShapeCollide(const Intersection::SupportShape& a,
                         const Intersection::SupportShape& b,
                         Intersection::Manifold* manifold);


bool Collide(const Ray& ray, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Ray& ray, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Segment& segment, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Segment& segment, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Aabb& aabb, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb1, const Aabb& aabb2, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Aabb& aabb, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Capsule& capsule, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule1, const Capsule& capsule2, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Capsule& capsule, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Cylinder& cylinder, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder1, const Cylinder& cylinder2, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Cylinder& cylinder, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Ellipsoid& ellipsoid, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid1, const Ellipsoid& ellipsoid2, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Ellipsoid& ellipsoid, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Frustum& frustum, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum1, const Frustum& frustum2, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Frustum& frustum, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Obb& obb, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Obb& obb1, const Obb& obb2, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Obb& obb, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Plane& plane, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Plane& plane1, const Plane& plane2, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Plane& plane, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Sphere& sphere, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere1, const Sphere& sphere2, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Sphere& sphere, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Triangle& triangle, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle1, const Triangle& triangle2, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Triangle& triangle, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const Tetrahedron& tetrahedron, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron1, const Tetrahedron& tetrahedron2, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const Tetrahedron& tetrahedron, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const ConvexMeshShape& supportShape, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape1, const ConvexMeshShape& supportShape2, Intersection::Manifold* manifold);
bool Collide(const ConvexMeshShape& supportShape, const SweptTriangle& sweptTri, Intersection::Manifold* manifold);

bool Collide(const SweptTriangle& sweptTri, const Ray& ray, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Segment& segment, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Aabb& aabb, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Capsule& capsule, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Cylinder& cylinder, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Ellipsoid& ellipsoid, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Frustum& frustum, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Obb& obb, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Plane& plane, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Sphere& sphere, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Triangle& triangle, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const Tetrahedron& tetrahedron, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri, const ConvexMeshShape& supportShape, Intersection::Manifold* manifold);
bool Collide(const SweptTriangle& sweptTri1, const SweptTriangle& sweptTri2, Intersection::Manifold* manifold);


}//namespace Zero
