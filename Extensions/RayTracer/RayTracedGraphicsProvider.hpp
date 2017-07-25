///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedGraphicsProvider.hpp
///  Declaration of the ray-traced graphics provider class.
///
/// Authors: Trevor Sundberg, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
//#include "RayTracedRenderer.hpp"
#include "RayTracer/RayTracer.hpp"

namespace Zero
{
  
//-------------------------------------------------------- Ray Tracing Functions
class RayTracingFunctions : public IRayTracer
{
public:
  RayTracingFunctions(void);
  RayTracingFunctions(Space* space);
  ~RayTracingFunctions(void);

  void SetSpace(Space* space);

  bool CastRayIntoScene(RayParam ray, bool refineResults, 
                        RayCastResults* results = NULL) override;
  bool CastRayIntoLights(SoftShadowFn softShadowFn, bool refineResults, 
                         LightArray& results) override;

private:
  Space* mSpace;
};

//------------------------------------------------- Ray Traced Graphics Provider
/// A ray-tracer that outputs beautiful graphics
class RayTracedGraphicsProvider : public RayTracedProvider
{
public:
  // Declare the ray trace provider
  ZeroDeclareType(RayTracedGraphicsProvider);

  // Bind any needed properties
  static void InitializeMeta(MetaType* meta);

  // Constructor
  RayTracedGraphicsProvider();
  ~RayTracedGraphicsProvider();

  // Perform a ray-trace given a ray, and output the color that was determined
  Vec4 RayTraceColor(Vec4Param backgroundColor, Vec3Param start,  
                     Vec3Param direction, IntVec2Param pixelPosition) override;

  // Perform a ray-trace given a ray, and output the color that was determined
  Vec4 RayTraceColor(Vec4Param backgroundColor, Vec3Param start, 
                     Vec3Param direction, int depth);

  Vec4 RayTraceColor(const Pixel& pixel, PixelSampler* pixelSampler) override;

  // An overridable method that is called when we begin rendering to a frame
  void BeginFrame(Space* space, PixelBuffer* buffer) override;

  // An overridable method that is called when we end rendering to a frame
  void EndFrame(Space* space, PixelBuffer* buffer) override;

  uint GetShadowSamples(void);
  void SetShadowSamples(uint shadowSamples);
  uint GetPixelSamples(void);
  void SetPixelSamples(uint pixelSamples);
  Vec4 GetAirAttenuation(void);
  void SetAirAttenuation(Vec4 airAttenuation);
  Vec4 GetAmbientColor(void);
  void SetAmbientColor(Vec4 ambientColor);
  uint GetPixelSampleType(void);
  void SetPixelSampleType(uint sampleType);
  uint GetTextureSampleType(void);
  void SetTextureSampleType(uint sampleType);

private:
  Space*                   mSpace;
  uint                     mDepth;
  uint                     mShadowSamples;
  uint                     mPixelSamples;
  Vec4                     mAmbientColor;
  Vec4                     mAirAttenuation;
  AntiAliasingType::Enum   mPixelSampleType;
  SoftwareSampleMode::Enum mTextureSampleType;
  RayTracer                mRayTracer;
  RayTracerConfig          mRayTracerConfig;
  RayTracingFunctions      mRayTracingFunctions;
};

}//namespace Zero
