///////////////////////////////////////////////////////////////////////////////
///
/// \file GeodesicSphere.cpp
/// Generates a geodesic sphere mesh (sphere made of equilateral triangles).
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Geometry
{

namespace
{
struct Face
{
  void Set(Vec3Param a, Vec3Param b, Vec3Param c)
  {
    Point[0] = a;
    Point[1] = b;
    Point[2] = c;
  }

  ///If the point is in front of the face, flip the face
  void OrientToPoint(Vec3Param point)
  {
    Vec3 n = Normalized(Cross(Point[1] - Point[0], Point[2] - Point[0]));
    real d = Dot(n, Point[0]);
    real dot = Dot(point, n) - d;
    if(dot > real(0.0))
    {
      Math::Swap(Point[1], Point[2]);
    }
  }

  Vec3 Point[3];
};

typedef Face* FacePtr;

typedef Zero::PodArray<Vec3>  Vec3Buffer;
typedef Zero::PodArray<Face>  FaceBuffer;

const real cSamePoint = real(0.00001);
}// namespace

void SubdivideFace(uint subdivisionCount, const uint baseFace[3], 
                   FaceBuffer& newFaces)
{
  if(subdivisionCount < 2)
  {
    return;
  }

  // 
  //               A
  //              /\
  //             /  \
  //            /    \
  //           /      \
  //          /        \
  //         /          \
  //        /____span____\
  //       /\/\/\/\/\/\/\/\
  //      /                \
  //     /                  \
  //    /                    \
  //   /______________________\
  // B                         C
  // 

  uint endIndex = subdivisionCount;

  Vec3Buffer sideAB(subdivisionCount + 1);
  sideAB[0] = PlatonicSolids::cIcosahedronPoints[baseFace[0]];
  sideAB[endIndex] = PlatonicSolids::cIcosahedronPoints[baseFace[1]];

  Vec3Buffer sideAC(subdivisionCount + 1);
  sideAC[0] = PlatonicSolids::cIcosahedronPoints[baseFace[0]];
  sideAC[endIndex] = PlatonicSolids::cIcosahedronPoints[baseFace[2]];

  Vec3Buffer span(subdivisionCount * subdivisionCount);

  real subdivisions = real(subdivisionCount);
  for(uint i = 1; i < subdivisionCount; ++i)
  {
    real I = real(i);
    sideAB[i] = Math::Lerp(sideAB[0], sideAB[endIndex], I / subdivisions);
    sideAC[i] = Math::Lerp(sideAC[0], sideAC[endIndex], I / subdivisions);
  }

  for(uint i = 0; i < subdivisionCount + 1; ++i)
  {
    for(uint j = 1; j < subdivisionCount - i; ++j)
    {
      real J = real(j);
      real subdivisionDiff = real(subdivisionCount - i);
      Vec3& start = sideAB[subdivisionCount - i];
      Vec3& end = sideAC[subdivisionCount - i];
      span[i * subdivisionCount + j] = Lerp(start, end, J / subdivisionDiff);
    }
  }

  newFaces.Resize(subdivisionCount * subdivisionCount);
  uint faceCount = 0;

  newFaces[faceCount].Set(sideAB[0], sideAC[1], sideAB[1]);
  ++faceCount;

  uint indexA = (subdivisionCount - 2) * subdivisionCount + 1;
  newFaces[faceCount].Set(sideAB[1], sideAB[2], span[indexA]);
  ++faceCount;

  newFaces[faceCount].Set(sideAB[1], span[indexA], sideAC[1]);
  ++faceCount;

  newFaces[faceCount].Set(sideAC[1], sideAC[2], span[indexA]);
  ++faceCount;

  for(uint i = 2; i < subdivisionCount; ++i)
  {
    //Side AB
    indexA = (subdivisionCount - i - 1) * subdivisionCount + 1;
    newFaces[faceCount].Set(sideAB[i], sideAB[i + 1], span[indexA]);
    ++faceCount;

    uint indexB = (subdivisionCount - i) * subdivisionCount + 1;
    newFaces[faceCount].Set(sideAB[i], span[indexB], span[indexA]);
    ++faceCount;

    //Center
    for(uint j = 1; j < subdivisionCount - i + 1; ++j)
    {
      indexA = (i - 2) * subdivisionCount + j + 1;
      indexB = (i - 1) * subdivisionCount + j;
      newFaces[faceCount].Set(span[indexA], span[indexA - 1], span[indexB]);
      ++faceCount;

      if(i > 2)
      {
        indexB = (i - 3) * subdivisionCount + j + 1;
        newFaces[faceCount].Set(span[indexA], span[indexA - 1], span[indexB]);
        ++faceCount;
      }
    }

    //Side AC
    indexB = (subdivisionCount - i - 1) * subdivisionCount + i;
    newFaces[faceCount].Set(sideAC[i], sideAC[i + 1], span[indexB]);
    ++faceCount;

    indexA = (subdivisionCount - i) * subdivisionCount + i - 1;
    newFaces[faceCount].Set(sideAC[i], span[indexA], span[indexB]);
    ++faceCount;
  }

  //Orient according to a given point, ensuring that all of the faces share the
  //same orientation with respect to the origin
  for(uint i = 0; i < faceCount; ++i)
  {
    newFaces[i].OrientToPoint(Vec3::cZero);
  }
}

int PointExists(Vec3Param point, const Vec3Ptr pointArray, uint pointCount)
{
  for(uint i = 0; i < pointCount; ++i)
  {
    Vec3 difference = point - pointArray[i];
    if(LengthSq(difference) < cSamePoint)
    {
      return i;
    }
  }
  return -1;
}

void GenerateTextureCoordinate(Vec3Param normal, Vec2Ref uvCoords)
{
  real normalizedX = real(0.0);
  real normalizedZ = real(-1.0);
  real sqNormalX = normal.x * normal.x;
  real sqNormalZ = normal.z * normal.z;
  if((sqNormalX + sqNormalZ) > real(0.0))
  {
    normalizedX = Math::Sqrt(sqNormalX / (sqNormalX + sqNormalZ));
    if(normal.x < real(0.0))
    {
      normalizedX = -normalizedX;
    }
    normalizedZ = Math::Sqrt(sqNormalZ / (sqNormalX + sqNormalZ));

    if(normal.z < real(0.0))
    {
      normalizedZ = -normalizedZ;
    }
  }

  if(normalizedZ == real(0.0))
  {
    uvCoords[0] = Math::cPi * real(0.5) * normalizedX;
  }
  else 
  {
     uvCoords[0] = Math::ArcTan(normalizedX / normalizedZ);
     if(normalizedZ < real(0.0))
     {
        uvCoords[0] += Math::cPi;
     }
  }
  if(uvCoords[0] < real(0.0))
  {
     uvCoords[0] += real(2.0) * Math::cPi;
  }
  uvCoords[0] /= real(2.0) * Math::cPi;
  uvCoords[1] = (real(1.0) - normal.y) / real(2.0);
}

void BuildIcosahedron(Vec3Ptr& vertices, uint& vertexCount, uint*& indices,
                      uint& indexCount, Vec3Ptr* normals, 
                      Vec2Ptr* textureCoords)
{
  vertexCount = 12;
  indexCount = 20;
  vertices = new Vec3[vertexCount];
  indices = new uint[indexCount * 3];

  //Steal points from array
  for(uint i = 0; i < vertexCount; ++i)
  {
    vertices[i] = PlatonicSolids::cIcosahedronPoints[i];
  }

  //Steal indices from array
  for(uint i = 0; i < indexCount; ++i)
  {
    indices[i * 3] = PlatonicSolids::cIcosahedronFaces[i][0];
    indices[(i * 3) + 1] = PlatonicSolids::cIcosahedronFaces[i][1];
    indices[(i * 3) + 2] = PlatonicSolids::cIcosahedronFaces[i][2];
  }

  //Compute the normals for the icosahedron
  if(normals != nullptr)
  {
    *normals = new Vec3[vertexCount];
    for(uint i = 0; i < vertexCount; ++i)
    {
      (*normals)[i] = Normalized(vertices[i]);
    }
  }

  //Compute the texture coordinates for the icosahedron, using a spherical map
  if(textureCoords != nullptr)
  {
    *textureCoords = new Vec2[vertexCount];
    for(uint i = 0; i < vertexCount; ++i)
    {
      Vec3 normalizedPoint = Normalized(vertices[i]);
      GenerateTextureCoordinate(normalizedPoint, (*textureCoords)[i]);
    }
  }
}

void BuildIcoSphere(uint subdivisionCount, Vec3Ptr& vertices, uint& vertexCount,
                    uint*& indices, uint& indexCount, Vec3Ptr* normals, 
                    Vec2Ptr* textureCoords)
{
  if(subdivisionCount == 1)
  {
    BuildIcosahedron(vertices, vertexCount, indices, indexCount, normals, 
                     textureCoords);
    return;
  }
  else if(subdivisionCount == 0)
  {
    vertexCount = 0;
    indexCount = 0;
    return;
  }

  //Pointers to the new subdivided face arrays
  FaceBuffer faces[20];
  
  //Subdivide all of the faces
  for(uint i = 0; i < 20; ++i)
  {
    SubdivideFace(subdivisionCount, PlatonicSolids::cIcosahedronFaces[i], 
                  faces[i]);
  }

  //Number of points for all of the faces, after duplicated points are removed
  uint maximumPointCount = 10 * (subdivisionCount * subdivisionCount) + 2;

  //Point array
  vertexCount = maximumPointCount;
  vertices = new Vec3[maximumPointCount];

  //Current number of points that have been saved
  uint currentPoint = 0;

  //Number of new faces per original face
  uint newFaceCount = subdivisionCount * subdivisionCount;

  //Go through all of the faces and store their points, taking care not to store
  //any duplicated points
  for (uint i = 0; i < 20; ++i)
  {
    for(uint j = 0; j < newFaceCount; ++j)
    {
      Face& currFace = faces[i][j];
      for(uint k = 0; k < 3; ++k)
      {
        if(PointExists(currFace.Point[k], vertices, currentPoint) == -1)
        {
          vertices[currentPoint] = currFace.Point[k];
          ++currentPoint;
        }
      }
    }
  }

  indexCount = newFaceCount * 20;
  indices = new uint[indexCount * 3];
  uint currentFace = 0;

  //Go through all of the points of all of the faces and locate their index 
  //offset in the point array. This can probably be done as the same time as
  //adding all of the points to the point array.
  for(uint i = 0; i < 20; ++i)
  {
    for(uint j = 0; j < newFaceCount; ++j)
    {
      Face& currFace = faces[i][j];
      int pointIndex = PointExists(currFace.Point[0], vertices, currentPoint);
      indices[currentFace] = pointIndex;
      ++currentFace;
      
      pointIndex = PointExists(currFace.Point[1], vertices, currentPoint);
      indices[currentFace] = pointIndex;
      ++currentFace;

      pointIndex = PointExists(currFace.Point[2], vertices, currentPoint);
      indices[currentFace] = pointIndex;
      ++currentFace;
    }
  }

  for(uint i = 0; i < maximumPointCount; ++i)
  {
    Normalize(vertices[i]);
  }

  if(normals != nullptr)
  {
    *normals = new Vec3[maximumPointCount];
    for(uint i = 0; i < maximumPointCount; ++i)
    {
      (*normals)[i] = vertices[i];
    }
  }

  if(textureCoords != nullptr)
  {
    *textureCoords = new Vec2[maximumPointCount];
    for(uint i = 0; i < maximumPointCount; ++i)
    {
      GenerateTextureCoordinate(vertices[i], (*textureCoords)[i]);
    }
  }
}

}// namespace Geometry
