///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracer.hpp
/// Declaration of the ray tracer class.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Geometry/Intersection.hpp"
#include "Geometry/Shapes.hpp"

namespace Zero
{

class IRayTracer;

DeclareEnum5(AntiAliasingType, Normal,
                               Uniform,
                               MonteCarlo,
                               Jittered,
                               Adaptive);

//------------------------------------------------------------- Ray Cast Results
struct RayCastResults
{
  RayCastResults(void);

  Vec4 Diffuse;
  Vec4 Attenuation;           //Color of the material inside of the object
  real SpecularScalar;        //Fraction of incident that's specularly reflected
  real SpecularPower;         //Governs the dispersion of the light
  real ElectricPermittivity;  //Amount a material distorts an electric field
  real MagneticPermeability;  //Amount a material distorts an magnetic field
  real IndexOfRefraction;
  Vec3 Normal;
  Vec2 Uv;
  Intersection::IntersectionPoint IntersectionData;
};

//----------------------------------------------------------- Light Cast Results
struct LightCastResults
{
  LightCastResults(void);
  LightCastResults(Vec3Param pointToLight, real shadowCoeff, Vec4Param color);

  Vec3 PointToLight;
  real ShadowCoefficient;
  Vec4 Color;
};

//------------------------------------------------------------------------ Pixel
///Screen-space object defined within the bounds of [-1, 1] on both axes.
struct Pixel
{
  Pixel(void);
  Pixel(Vec2Param center, Vec2Param dimensions);

  Vec2 Center;    
  Vec2 Dimensions;
};

//---------------------------------------------------------------- Pixel Sampler
class PixelSampler
{
public:
  virtual ~PixelSampler(void) {}

  virtual void GetRay(Vec2Param pixelCenter, Vec3Ptr rayStart, 
                      Vec3Ptr rayDirection) = 0;
  virtual Vec4 RayTraceColor(Vec3Param rayStart, 
                             Vec3Param rayDirection) const = 0;
};

//------------------------------------------------------------------- Shadow Ray
struct ShadowRay
{
  ShadowRay(void);
  ShadowRay(Vec3Param rayStart, Vec3Param surfaceNormal, real positionOffset);

  ///Expects a normalized vector.
  Ray GenerateRay(Vec3Param pointToLight) const;

  Vec3 Start;
  Vec3 PositionOffset;
};

typedef const ShadowRay& ShadowRayParam;

//----------------------------------------------------------------- Soft Shadows
class SoftShadow
{
public:
  SoftShadow(uint samples, ShadowRayParam ray, IRayTracer* interface);

  real operator () (Vec3Param lightPosition, real lightRadius) const;
  const Vec3& RayStart(void) const;

private:
  IRayTracer* mInterface;
  ShadowRay   mShadowRay;
  uint        mSamples;

  real Sample(Vec3Param lightPosition, real lightRadius) const;
};

typedef const SoftShadow& SoftShadowFn;

typedef Array<LightCastResults> LightArray;

//------------------------------------------------------------ Ray Tracer Config
struct RayTracerConfig
{
  RayTracerConfig(void);

  uint ShadowSamples;
  uint PixelSamples;
  Vec4 ClearColor;
  Vec4 AirAttenuation;
  Vec4 AmbientColor;
  AntiAliasingType::Enum SampleType;
};

//--------------------------------------------------------- Ray Tracer Interface
class IRayTracer
{
public:
  virtual ~IRayTracer(void) {}

  //Assumed that a value of "NULL" for "results" means that the function 
  //implemented does not bother to compute the information.
  virtual bool CastRayIntoScene(RayParam ray, bool refineResults, 
                                RayCastResults* results) = 0;
  virtual bool CastRayIntoLights(SoftShadowFn softShadowFn, bool refineResults,
                                 LightArray& results) = 0;
};

//------------------------------------------------------------------- Ray Tracer
class RayTracer
{
public:
  RayTracer(void);
  ~RayTracer(void);

  ///RayTracer is not responsible for the memory of the IRayTracer.
  void Init(IRayTracer* interface);

  void SetPixelSampler(PixelSampler* pixelSampler);
  Vec4 CalculatePixelColor(const Pixel& pixel);

  void SetProperties(const RayTracerConfig& rayTracerConfig);
  const RayTracerConfig& GetProperties(void) const;

  Vec4 RayTraceScene(const Ray& ray, int depth);

private:
  IRayTracer*     mInterface;
  RayTracerConfig mProperties;
  PixelSampler*   mPixelSampler;
  const real      cPositionOffset;

  Vec4 RayTraceScene(const Ray& ray, bool inAir, int depth);

  Vec4 CalculateLighting(RayParam ray, real reflectionCoefficient, 
                         Vec4Param materialAttenuation, 
                         const RayCastResults& results);
  real ReflectionCoefficient(real airObjRefractionIndex, 
                             real airObjMagneticPermeability, 
                             Vec3Param rayDirection, Vec3Param surfaceNormal);

  ///Calculates the ray that gets reflected off of an object.
  Ray ReflectedRay(RayParam ray, real positionOffset, 
                   const RayCastResults& results);

  ///Calculates the ray that gets transmitted through an object.
  Ray TransmittedRay(RayParam ray, real airObjRefractionIndex, 
                     real positionOffset, const RayCastResults& results);

  bool CastRayIntoScene(RayParam ray, bool refineResults, 
                        RayCastResults* results = NULL);
  bool CastRayIntoLights(SoftShadowFn softShadowFn, bool refineResults,
                         LightArray& results);

  Vec4 NormalSampling(const Pixel& pixel);
  Vec4 UniformSupersampling(const Pixel& pixel, uint sampleCount);
  Vec4 MonteCarloSupersampling(const Pixel& pixel, uint sampleCount);
  Vec4 JitteredSupersampling(const Pixel& pixel, uint sampleCount);
  Vec4 AdaptiveSupersampling(const Pixel& pixel, uint sampleCount);
};

}// namespace Zero
