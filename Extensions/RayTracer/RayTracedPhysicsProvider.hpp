///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedPhysicsProvider.hpp
///  Declaration of the ray-traced physics provider class.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
//#include "RayTracedRenderer.hpp"
#include "Physics/RayCast.hpp"

namespace Zero
{
//-------------------------------------------------- Ray Traced Physics Provider
// A ray-tracer that uses physics ray-casting
class RayTracedPhysicsProvider : public RayTracedProvider
{
public:
  // Declare the ray trace provider
  ZeroDeclareType(RayTracedPhysicsProvider);

  // Bind any needed properties
  static void InitializeMeta(MetaType* meta);

  // Constructor
  RayTracedPhysicsProvider();

  // Perform a ray-trace given a ray, and output the color that was determined
  Vec4 RayTraceColor(Vec4Param backgroundColor, Vec3Param start, 
                     Vec3Param direction, IntVec2Param pixelPosition) override;

  // Perform a ray-trace given a ray, and output the color that was determined
  Vec4 RayTraceColor(Vec4Param backgroundColor, Vec3Param start, 
                     Vec3Param direction, IntVec2Param pixelPosition, int depth);

  //Vec4 RayTraceColor(const Pixel& pixel, PixelSampler* pixelSampler) override;

  // An overridable method that is called when we begin rendering to a frame
  //void BeginFrame(Space* space, PixelBuffer* buffer) override;

private:

  // Get the color of an object
  Vec4 GetObjectColor(CastResult& hit);

private:

  // Do we want to see all shapes as transparent
  bool mDrawTransparent;

  // How do we color the object?
  ObjectColorMode::Type mObjectColorMode;

  // Do we want basic lighting?
  bool mBasicLighting;

  // Store the cast results
  CastResults mResults;
};

}//namespace Zero
