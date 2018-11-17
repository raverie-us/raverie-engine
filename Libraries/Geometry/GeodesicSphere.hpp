///////////////////////////////////////////////////////////////////////////////
///
/// \file GeodesicSphere.hpp
/// Generates a geodesic sphere mesh (sphere made of equilateral triangles).
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Geometry
{

///Builds an icosahedron

///Builds an icosahedron-based sphere with a radius of 1. The memory allocated
///is your responsibility.
void BuildIcoSphere(uint subdivisionCount, Vec3Ptr& vertices, uint& vertexCount,
                    uint*& indices, uint& indexCount, Vec3Ptr* normals = nullptr,
                    Vec2Ptr* textureCoordinates = nullptr);

}// namespace Geometry
