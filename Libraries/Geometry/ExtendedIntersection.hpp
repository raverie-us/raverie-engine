///////////////////////////////////////////////////////////////////////////////
///
/// \file ExtendedIntersection.hpp
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

bool SupportShapeOverlap(const Intersection::SupportShape& a, const Intersection::SupportShape& b);

bool Overlap(Vec3Param point1, Vec3Param point2);
bool Overlap(Vec3Param point, const Ray& ray);
bool Overlap(Vec3Param point, const Segment& segment);
bool Overlap(Vec3Param point, const Aabb& aabb);
bool Overlap(Vec3Param point, const Capsule& capsule);
bool Overlap(Vec3Param point, const Cylinder& cylinder);
bool Overlap(Vec3Param point, const Ellipsoid& ellipsoid);
bool Overlap(Vec3Param point, const Frustum& frustum);
bool Overlap(Vec3Param point, const Obb& obb);
bool Overlap(Vec3Param point, const Plane& plane);
bool Overlap(Vec3Param point, const Sphere& sphere);
bool Overlap(Vec3Param point, const Triangle& triangle);
bool Overlap(Vec3Param point, const Tetrahedron& tetrahedron);
bool Overlap(Vec3Param point, const ConvexMeshShape& supportShape);
bool Overlap(Vec3Param point, const SweptTriangle& sweptTri);

bool Overlap(const Ray& ray, Vec3Param point);
bool Overlap(const Ray& ray, const Aabb& aabb);
bool Overlap(const Ray& ray, const Capsule& capsule);
bool Overlap(const Ray& ray, const Cylinder& cylinder);
bool Overlap(const Ray& ray, const Ellipsoid& ellipsoid);
bool Overlap(const Ray& ray, const Obb& obb);
bool Overlap(const Ray& ray, const Plane& plane);
bool Overlap(const Ray& ray, const Sphere& sphere);
bool Overlap(const Ray& ray, const Triangle& triangle);
bool Overlap(const Ray& ray, const Tetrahedron& tetrahedron);

bool Overlap(const Segment& segment, Vec3Param point);
bool Overlap(const Segment& segment, const Aabb& aabb);
bool Overlap(const Segment& segment, const Capsule& capsule);
bool Overlap(const Segment& segment, const Cylinder& cylinder);
bool Overlap(const Segment& segment, const Obb& obb);
bool Overlap(const Segment& segment, const Plane& plane);
bool Overlap(const Segment& segment, const Sphere& sphere);
bool Overlap(const Segment& segment, const Triangle& triangle);

bool Overlap(const Aabb& aabb, Vec3Param point);
bool Overlap(const Aabb& aabb, const Ray& ray);
bool Overlap(const Aabb& aabb, const Segment& segment);
bool Overlap(const Aabb& aabb1, const Aabb& aabb2);
bool Overlap(const Aabb& aabb, const Capsule& capsule);
bool Overlap(const Aabb& aabb, const Cylinder& cylinder);
bool Overlap(const Aabb& aabb, const Ellipsoid& ellipsoid);
bool Overlap(const Aabb& aabb, const Frustum& frustum);
bool Overlap(const Aabb& aabb, const Obb& obb);
bool Overlap(const Aabb& aabb, const Plane& plane);
bool Overlap(const Aabb& aabb, const Sphere& sphere);
bool Overlap(const Aabb& aabb, const Triangle& triangle);
bool Overlap(const Aabb& aabb, const Tetrahedron& tetrahedron);
bool Overlap(const Aabb& aabb, const ConvexMeshShape& supportShape);
bool Overlap(const Aabb& aabb, const SweptTriangle& sweptTri);

bool Overlap(const Capsule& capsule, Vec3Param point);
bool Overlap(const Capsule& capsule, const Ray& ray);
bool Overlap(const Capsule& capsule, const Segment& segment);
bool Overlap(const Capsule& capsule, const Aabb& aabb);
bool Overlap(const Capsule& capsule1, const Capsule& capsule2);
bool Overlap(const Capsule& capsule, const Cylinder& cylinder);
bool Overlap(const Capsule& capsule, const Ellipsoid& ellipsoid);
bool Overlap(const Capsule& capsule, const Frustum& frustum);
bool Overlap(const Capsule& capsule, const Obb& obb);
bool Overlap(const Capsule& capsule, const Sphere& sphere);
bool Overlap(const Capsule& capsule, const Triangle& triangle);
bool Overlap(const Capsule& capsule, const Tetrahedron& tetrahedron);
bool Overlap(const Capsule& capsule, const ConvexMeshShape& supportShape);
bool Overlap(const Capsule& capsule, const SweptTriangle& sweptTri);

bool Overlap(const Cylinder& cylinder, Vec3Param point);
bool Overlap(const Cylinder& cylinder, const Ray& ray);
bool Overlap(const Cylinder& cylinder, const Segment& segment);
bool Overlap(const Cylinder& cylinder, const Aabb& aabb);
bool Overlap(const Cylinder& cylinder, const Capsule& capsule);
bool Overlap(const Cylinder& cylinder1, const Cylinder& cylinder2);
bool Overlap(const Cylinder& cylinder, const Ellipsoid& ellipsoid);
bool Overlap(const Cylinder& cylinder, const Frustum& frustum);
bool Overlap(const Cylinder& cylinder, const Obb& obb);
bool Overlap(const Cylinder& cylinder, const Sphere& sphere);
bool Overlap(const Cylinder& cylinder, const Triangle& triangle);
bool Overlap(const Cylinder& cylinder, const Tetrahedron& tetrahedron);
bool Overlap(const Cylinder& cylinder, const ConvexMeshShape& supportShape);
bool Overlap(const Cylinder& cylinder, const SweptTriangle& sweptTri);

bool Overlap(const Ellipsoid& ellipsoid, Vec3Param point);
bool Overlap(const Ellipsoid& ellipsoid, const Ray& ray);
bool Overlap(const Ellipsoid& ellipsoid, const Aabb& aabb);
bool Overlap(const Ellipsoid& ellipsoid, const Capsule& capsule);
bool Overlap(const Ellipsoid& ellipsoid, const Cylinder& cylinder);
bool Overlap(const Ellipsoid& ellipsoid1, const Ellipsoid& ellipsoid2);
bool Overlap(const Ellipsoid& ellipsoid, const Frustum& frustum);
bool Overlap(const Ellipsoid& ellipsoid, const Obb& obb);
bool Overlap(const Ellipsoid& ellipsoid, const Sphere& sphere);
bool Overlap(const Ellipsoid& ellipsoid, const Triangle& triangle);
bool Overlap(const Ellipsoid& ellipsoid, const Tetrahedron& tetrahedron);
bool Overlap(const Ellipsoid& ellipsoid, const ConvexMeshShape& supportShape);
bool Overlap(const Ellipsoid& ellipsoid, const SweptTriangle& sweptTri);

bool Overlap(const Frustum& frustum, Vec3Param point);
bool Overlap(const Frustum& frustum, const Aabb& aabb);
bool Overlap(const Frustum& frustum, const Capsule& capsule);
bool Overlap(const Frustum& frustum, const Cylinder& cylinder);
bool Overlap(const Frustum& frustum, const Ellipsoid& ellipsoid);
bool Overlap(const Frustum& frustum1, const Frustum& frustum2);
bool Overlap(const Frustum& frustum, const Obb& obb);
bool Overlap(const Frustum& frustum, const Sphere& sphere);
bool Overlap(const Frustum& frustum, const Triangle& triangle);
bool Overlap(const Frustum& frustum, const Tetrahedron& tetrahedron);
bool Overlap(const Frustum& frustum, const ConvexMeshShape& supportShape);
bool Overlap(const Frustum& frustum, const SweptTriangle& sweptTri);

bool Overlap(const Obb& obb, Vec3Param point);
bool Overlap(const Obb& obb, const Ray& ray);
bool Overlap(const Obb& obb, const Segment& segment);
bool Overlap(const Obb& obb, const Aabb& aabb);
bool Overlap(const Obb& obb, const Capsule& capsule);
bool Overlap(const Obb& obb, const Cylinder& cylinder);
bool Overlap(const Obb& obb, const Ellipsoid& ellipsoid);
bool Overlap(const Obb& obb, const Frustum& frustum);
bool Overlap(const Obb& obb1, const Obb& obb2);
bool Overlap(const Obb& obb, const Plane& plane);
bool Overlap(const Obb& obb, const Sphere& sphere);
bool Overlap(const Obb& obb, const Triangle& triangle);
bool Overlap(const Obb& obb, const Tetrahedron& tetrahedron);
bool Overlap(const Obb& obb, const ConvexMeshShape& supportShape);
bool Overlap(const Obb& obb, const SweptTriangle& sweptTri);

bool Overlap(const Plane& plane, Vec3Param point);
bool Overlap(const Plane& plane, const Ray& ray);
bool Overlap(const Plane& plane, const Segment& segment);
bool Overlap(const Plane& plane, const Aabb& aabb);
bool Overlap(const Plane& plane, const Obb& obb);
bool Overlap(const Plane& plane, const Sphere& sphere);

bool Overlap(const Sphere& sphere, Vec3Param point);
bool Overlap(const Sphere& sphere, const Ray& ray);
bool Overlap(const Sphere& sphere, const Segment& segment);
bool Overlap(const Sphere& sphere, const Aabb& aabb);
bool Overlap(const Sphere& sphere, const Capsule& capsule);
bool Overlap(const Sphere& sphere, const Cylinder& cylinder);
bool Overlap(const Sphere& sphere, const Ellipsoid& ellipsoid);
bool Overlap(const Sphere& sphere, const Frustum& frustum);
bool Overlap(const Sphere& sphere, const Obb& obb);
bool Overlap(const Sphere& sphere, const Plane& plane);
bool Overlap(const Sphere& sphere1, const Sphere& sphere2);
bool Overlap(const Sphere& sphere, const Triangle& triangle);
bool Overlap(const Sphere& sphere, const Tetrahedron& tetrahedron);
bool Overlap(const Sphere& sphere, const ConvexMeshShape& supportShape);
bool Overlap(const Sphere& sphere, const SweptTriangle& sweptTri);

bool Overlap(const Triangle& triangle, Vec3Param point);
bool Overlap(const Triangle& triangle, const Ray& ray);
bool Overlap(const Triangle& triangle, const Segment& segment);
bool Overlap(const Triangle& triangle, const Aabb& aabb);
bool Overlap(const Triangle& triangle, const Capsule& capsule);
bool Overlap(const Triangle& triangle, const Cylinder& cylinder);
bool Overlap(const Triangle& triangle, const Ellipsoid& ellipsoid);
bool Overlap(const Triangle& triangle, const Frustum& frustum);
bool Overlap(const Triangle& triangle, const Obb& obb);
bool Overlap(const Triangle& triangle, const Sphere& sphere);
bool Overlap(const Triangle& triangle1, const Triangle& triangle2);
bool Overlap(const Triangle& triangle, const Tetrahedron& tetrahedron);
bool Overlap(const Triangle& triangle, const ConvexMeshShape& supportShape);
bool Overlap(const Triangle& triangle, const SweptTriangle& sweptTri);

bool Overlap(const Tetrahedron& tetrahedron, Vec3Param point);
bool Overlap(const Tetrahedron& tetrahedron, const Ray& ray);
bool Overlap(const Tetrahedron& tetrahedron, const Aabb& aabb);
bool Overlap(const Tetrahedron& tetrahedron, const Capsule& capsule);
bool Overlap(const Tetrahedron& tetrahedron, const Cylinder& cylinder);
bool Overlap(const Tetrahedron& tetrahedron, const Ellipsoid& ellipsoid);
bool Overlap(const Tetrahedron& tetrahedron, const Frustum& frustum);
bool Overlap(const Tetrahedron& tetrahedron, const Obb& obb);
bool Overlap(const Tetrahedron& tetrahedron, const Sphere& sphere);
bool Overlap(const Tetrahedron& tetrahedron, const Triangle& triangle);
bool Overlap(const Tetrahedron& tetrahedron1, const Tetrahedron& tetrahedron2);
bool Overlap(const Tetrahedron& tetrahedron, const ConvexMeshShape& supportShape);
bool Overlap(const Tetrahedron& tetrahedron, const SweptTriangle& sweptTri);

bool Overlap(const ConvexMeshShape& supportShape, Vec3Param point);
bool Overlap(const ConvexMeshShape& supportShape, const Aabb& aabb);
bool Overlap(const ConvexMeshShape& supportShape, const Capsule& capsule);
bool Overlap(const ConvexMeshShape& supportShape, const Cylinder& cylinder);
bool Overlap(const ConvexMeshShape& supportShape, const Ellipsoid& ellipsoid);
bool Overlap(const ConvexMeshShape& supportShape, const Frustum& frustum);
bool Overlap(const ConvexMeshShape& supportShape, const Obb& obb);
bool Overlap(const ConvexMeshShape& supportShape, const Sphere& sphere);
bool Overlap(const ConvexMeshShape& supportShape, const Triangle& triangle);
bool Overlap(const ConvexMeshShape& supportShape, const Tetrahedron& tetrahedron);
bool Overlap(const ConvexMeshShape& supportShape1, const ConvexMeshShape& supportShape2);
bool Overlap(const ConvexMeshShape& supportShape, const SweptTriangle& sweptTri);

bool Overlap(const SweptTriangle& sweptTri, Vec3Param point);
bool Overlap(const SweptTriangle& sweptTri, const Aabb& aabb);
bool Overlap(const SweptTriangle& sweptTri, const Capsule& capsule);
bool Overlap(const SweptTriangle& sweptTri, const Cylinder& cylinder);
bool Overlap(const SweptTriangle& sweptTri, const Ellipsoid& ellipsoid);
bool Overlap(const SweptTriangle& sweptTri, const Frustum& frustum);
bool Overlap(const SweptTriangle& sweptTri, const Obb& obb);
bool Overlap(const SweptTriangle& sweptTri, const Sphere& sphere);
bool Overlap(const SweptTriangle& sweptTri, const Triangle& triangle);
bool Overlap(const SweptTriangle& sweptTri, const Tetrahedron& tetrahedron);
bool Overlap(const SweptTriangle& sweptTri, const ConvexMeshShape& supportShape);
bool Overlap(const SweptTriangle& sweptTri1, const SweptTriangle& sweptTri2);


}//namespace Zero
