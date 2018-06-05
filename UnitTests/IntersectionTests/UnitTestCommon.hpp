///////////////////////////////////////////////////////////////////////////////
///
///  \file UnitTestCommon.hpp
///  Common constants used throughout the Intersection library's unit tests.
///	
///  Authors: Benjamin Strukus
///  Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once


#include "Geometry/GeometryStandard.hpp"
#include "Geometry/Mpr.hpp"


namespace
{
using namespace Intersection;
//using Geometry::Vec3Array;
const real zero = real(0.0);
const real epsilon[] = { real(0.1), 
                         real(0.01), 
                         real(0.001), 
                         real(0.0001), 
                         real(0.00001), 
                         real(0.000001),
                         real(0.0000001) 
                       };
const uint last = (sizeof(epsilon) / sizeof(real)) - 1;

Vec4 Vec3ToVec4(Vec3Param vec3, real w)
{
  return Vec4(vec3.x, vec3.y, vec3.z, w);
}
}// namespace
