// MIT Licensed (see LICENSE.md).
#pragma once

namespace Geometry
{

/// Builds an icosahedron

/// Builds an icosahedron-based sphere with a radius of 1. The memory allocated
/// is your responsibility.
void BuildIcoSphere(uint subdivisionCount,
                    Vec3Ptr& vertices,
                    uint& vertexCount,
                    uint*& indices,
                    uint& indexCount,
                    Vec3Ptr* normals = nullptr,
                    Vec2Ptr* textureCoordinates = nullptr);

} // namespace Geometry
