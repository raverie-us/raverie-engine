///////////////////////////////////////////////////////////////////////////////
///
/// \file ConvexMeshShape.cpp
/// Implementation of the ConvexMeshShape which is a wrapper
/// shape around a SupportShape.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ConvexMeshShape::ConvexMeshShape()
{
  mWorldAabb.Zero();
}

ConvexMeshShape::ConvexMeshShape(const Intersection::SupportShape& support)
  : mSupport(support)
{
  mWorldAabb.Zero();
}

void ConvexMeshShape::Support(Vec3Param direction, Vec3Ptr support) const
{
  mSupport.Support(direction, support);
}

void ConvexMeshShape::GetCenter(Vec3Ref center) const
{
  return mSupport.GetCenter(&center);
}

Obb ConvexMeshShape::Transform(Mat4Param transformation) const
{
  return mWorldAabb.Transform(transformation);
}

}//namespace Zero
