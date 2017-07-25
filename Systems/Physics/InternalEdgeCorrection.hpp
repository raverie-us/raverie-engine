///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Nathan Carlson
/// Copyright 2015-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

typedef Array<Vec3> VertexArray;
typedef Array<uint> IndexArray;

///Should not be used externally. Only exposed for debugging purposes.
void ComputeEdgeInfoForTriangleA(Triangle& triA, uint indexA,
                                 Triangle& triB, TriangleInfoMap* infoMap);
///Main function for generating the triangle map info for a mesh.
///Should be called any time a mesh is changed. (n-squared version)
void GenerateInternalEdgeInfo(GenericPhysicsMesh* mesh, TriangleInfoMap* infoMap);
///Specialized function for physics meshes. Uses the internally stored
///aabb tree to filter out triangles and not do n-squared.
void GenerateInternalEdgeInfo(PhysicsMesh* mesh, TriangleInfoMap* infoMap);
///Specialized function for height maps. Goes through all patches
///in the height map and determines adjacent triangles (linear)
void GenerateInternalEdgeInfo(HeightMapCollider* collider, TriangleInfoMap* infoMap);

///Currently just a function for debugging purposes.
void CorrectInternalEdgeNormal(Physics::ManifoldPoint& point, uint objectIndex,
                               Collider* meshCollider, uint triId);
///Given a manifold, correct any normals that are
///pointing the wrong way due to internal edges.
void CorrectInternalEdgeNormal(GenericPhysicsMesh* mesh, Physics::Manifold* manifold,
                               uint objectIndex, uint contactId);
///Given a manifold, correct any normals that are
///pointing the wrong way due to internal edges.
void CorrectInternalEdgeNormal(HeightMapCollider* heightMap, Physics::Manifold* manifold,
                               uint objectIndex, uint contactId);

}//namespace Zero
