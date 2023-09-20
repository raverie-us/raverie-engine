// MIT Licensed (see LICENSE.md).
#pragma once

// Standard includes
#include "Foundation/Common/CommonStandard.hpp"

#include "Foundation/RaverieLanguage/Precompiled.hpp"
using namespace Raverie;

namespace Geometry
{

using namespace Math;
using Raverie::Array;

} // namespace Geometry

namespace Raverie
{
#include "Math/MathImports.hpp"
using Geometry::Vec3Array;

typedef Array<Array<Vec2>> ContourArray;

namespace Csg
{
using Raverie::ContourArray;
} // namespace Csg

// Geometry library
class GeometryLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(GeometryLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

namespace Intersection
{
using namespace Geometry;
using namespace Math;
} // namespace Intersection

namespace Csg
{
using namespace Geometry;
using namespace Math;
} // namespace Csg

// Project includes
#include "Solids.hpp"
#include "Geometry.hpp"
#include "Intersection.hpp"
#include "Shapes.hpp"
#include "Aabb.hpp"
#include "Polygon.hpp"
#include "Clipper.hpp"
#include "Plane.hpp"
#include "Frustum.hpp"
#include "Sphere.hpp"
#include "DebugDraw.hpp"
#include "DebugDrawStack.hpp"
#include "IndexPool.hpp"
#include "TrapezoidMap.hpp"
#include "Triangulator.hpp"
#include "Shape2D.hpp"
#include "ConvexMeshShape.hpp"
#include "ConvexMeshDecomposition.hpp"
#include "ShapeHelpers.hpp"
#include "Foundation/Geometry/Simplex.hpp"
#include "Foundation/Geometry/Epa.hpp"
#include "Gjk.hpp"
#include "ExtendedCollision.hpp"
#include "ExtendedIntersection.hpp"
#include "GeodesicSphere.hpp"
#include "Hull2D.hpp"
#include "QuickHull3D.hpp"
#include "ToString.hpp"
