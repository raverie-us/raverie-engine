///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracer.cpp
/// Definition of the Ray Tracer class and methods.
/// 
/// Authors: Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "RayTracer.hpp"
#include "Math/Random.hpp"

namespace Zero
{

namespace
{

Math::Random gRandom;
const real cColorTolerance = real(0.1);

//--------------------------------------------------------- File-Scope Functions
Vec3 GaussianSample(real sigma)
{
  real u;
  do 
  {
    u = gRandom.Float();
  } while (u < real(1.0e-4));
  real theta = gRandom.Float() * Math::cTwoPi;
  u = Math::Sqrt(real(-2.0) * Math::Sq(sigma) * Math::Log(u));
  return Vec3(u * Math::Cos(theta), u * Math::Sin(theta), real(0.0));
}

bool ColorComparison(const Vec4* colors, uint colorCount)
{
  for(uint i = 0; i < colorCount; ++i)
  {
    for(uint j = i + 1; j < colorCount; ++j)
    {
      const Vec4& colorA = colors[i];
      const Vec4& colorB = colors[j];
      for(uint k = 0; k < 3; ++k)
      {
        if(Math::Abs(colorA[k] - colorB[k]) > cColorTolerance)
        {
          return false;
        }
      }
    }
  }
  return true;
}

}//namespace

//------------------------------------------------------------- Ray Cast Results
RayCastResults::RayCastResults(void)
{
  Diffuse = Vec4::cZero;
  SpecularScalar = real(0.0);
  SpecularPower = real(0.0);
  ElectricPermittivity = real(0.0);
  MagneticPermeability = real(0.0);
  IndexOfRefraction = real(0.0);
}

//----------------------------------------------------------- Light Cast Results
LightCastResults::LightCastResults(void)
{
  //
}

LightCastResults::LightCastResults(Vec3Param pointToLight, real shadowCoeff, 
                                   Vec4Param color)
  : PointToLight(pointToLight), ShadowCoefficient(shadowCoeff), Color(color)
{
  //
}

//------------------------------------------------------------------------ Pixel
Pixel::Pixel(void)
{
  //
}

Pixel::Pixel(Vec2Param center, Vec2Param dimensions)
  : Center(center), Dimensions(dimensions)
{
  //
}

//------------------------------------------------------------------- Shadow Ray
ShadowRay::ShadowRay(void)
{
  //
}

ShadowRay::ShadowRay(Vec3Param rayStart, Vec3Param surfaceNormal, 
                     real positionOffset)
  : Start(rayStart)
{
  PositionOffset = Normalized(surfaceNormal) * positionOffset;
}

//Expects a normalized vector.
Ray ShadowRay::GenerateRay(Vec3Param pointToLight) const
{
  return Ray(Start + PositionOffset, pointToLight);
}

//----------------------------------------------------------------- Soft Shadows
SoftShadow::SoftShadow(uint samples, ShadowRayParam ray, IRayTracer* interface)
  : mSamples(samples), mShadowRay(ray), mInterface(interface)
{
  //
}

real SoftShadow::operator () (Vec3Param lightPosition, real lightRadius) const
{
  if(mSamples == 0)
  {
    return real(1.0);
  }
  else if(mSamples == 1)
  {
    const bool cRefineResults = false;
    Vec3 pointToLight = lightPosition - mShadowRay.Start;
    real lightDistance = Normalize(&pointToLight);

    //Check to see if there is anything between the light and the point.
    RayCastResults temp;
    Ray shadowRay = mShadowRay.GenerateRay(pointToLight);
    if(mInterface->CastRayIntoScene(shadowRay, cRefineResults, &temp))
    {
      //There is something along the ray's direction! Check to see if that
      //something is closer to the point than the light is.
      if(Math::InRange(temp.IntersectionData.T, real(0.0), lightDistance))
      {
        return real(0.0);  
      }
    }
    return real(1.0);
  }
  else
  {
    //Soft shadows!
    return Sample(lightPosition, lightRadius);
  }
}

const Vec3& SoftShadow::RayStart(void) const
{
  return mShadowRay.Start;
}

real SoftShadow::Sample(Vec3Param lightPosition, real lightRadius) const
{
  Vec3 pointToLight = Normalized(lightPosition - mShadowRay.Start);

  Vec3 uAxis, vAxis;
  GenerateOrthonormalBasis(-pointToLight, &uAxis, &vAxis);

  real total = real(mSamples);
  RayCastResults testResults;
  for(uint i = 0; i < mSamples; ++i)
  {
    Vec3 offset = GaussianSample(real(1.0)) * lightRadius;

    Vec3 planePoint = lightPosition + (offset.x * uAxis) + (offset.y * vAxis);
    
    Vec3 pointToLight = planePoint - mShadowRay.Start;
    real lightDistance = Normalize(&pointToLight);
    Ray shadowRay = mShadowRay.GenerateRay(pointToLight);
    
    if(mInterface->CastRayIntoScene(shadowRay, true, &testResults))
    {
      const real& tValue = testResults.IntersectionData.T;
      if(Math::InRange(tValue, real(0.0), lightDistance))
      {
        total -= real(1.0);
      }
    }
  }
  return total / real(mSamples);
}

//------------------------------------------------------------ Ray Tracer Config
RayTracerConfig::RayTracerConfig(void)
{
  ShadowSamples = 1;
  PixelSamples = 1;
  ClearColor = Vec4(real(0.0), real(0.0), real(0.0), real(1.0));
  AirAttenuation = Vec4(real(1.0), real(1.0), real(1.0), real(1.0));
  AmbientColor = Vec4(real(0.0), real(0.0), real(0.0), real(0.0));
  SampleType = AntiAliasingType::Normal;
}

//------------------------------------------------------------------- Ray Tracer
RayTracer::RayTracer(void)
  : cPositionOffset(real(1.0e-3))
{
  mInterface = NULL;
}

RayTracer::~RayTracer(void)
{
  //delete mInterface;
}

void RayTracer::Init(IRayTracer* interface)
{
  mInterface = interface;
}

void RayTracer::SetPixelSampler(PixelSampler* pixelSampler)
{
  mPixelSampler = pixelSampler;
}

Vec4 RayTracer::CalculatePixelColor(const Pixel& pixel)
{
  switch(mProperties.SampleType)
  {
    case AntiAliasingType::Uniform:
    {
      return UniformSupersampling(pixel, mProperties.PixelSamples);
    }
    break;

    case AntiAliasingType::MonteCarlo:
    {
      return MonteCarloSupersampling(pixel, mProperties.PixelSamples);
    }
    break;

    case AntiAliasingType::Jittered:
    {
      return JitteredSupersampling(pixel, mProperties.PixelSamples);
    }
    break;

    case AntiAliasingType::Adaptive:
    {
      return AdaptiveSupersampling(pixel, mProperties.PixelSamples);
    }
    break;

    case AntiAliasingType::Normal:
    default:
    {
      return NormalSampling(pixel);
    }
    break;
  }
}

void RayTracer::SetProperties(const RayTracerConfig& rayTracerConfig)
{
  mProperties = rayTracerConfig;
}

const RayTracerConfig& RayTracer::GetProperties(void) const
{
  return mProperties;
}

Vec4 RayTracer::RayTraceScene(RayParam ray, int depth)
{
  return RayTraceScene(ray, true, depth);
}

//-------------------------------------------------------------------- Privates
Vec4 RayTracer::RayTraceScene(const Ray& ray, bool inAir, int depth)
{
  if(depth < 0)
  {
    return mProperties.ClearColor;
  }

  //----------------------------------------------------------------------------
  //Find first point of intersection.
  RayCastResults results;
  if(!CastRayIntoScene(ray, true, &results))
  {
    return Vec4::cZero;
  }

  //----------------------------------------------------------------------------
  //Relative material properties
  real nInT = results.IndexOfRefraction;    //Ratio of indices of refraction for 
                                            //air and the object.
  real uIuT = results.MagneticPermeability; //Ratio of magnetic permeability for
                                            //air and the object.
  real positionOffset = -cPositionOffset;   //Offset scale used when computing
                                            //reflection and transmission rays.
  Vec4 attenuation = results.Attenuation;   //Color inside the object.
  if(inAir)
  {
    //Going from air to object.
    nInT = real(1.0) / nInT;
    uIuT = real(1.0) / uIuT;
    positionOffset = -positionOffset;
    attenuation = mProperties.AirAttenuation;
  }

  //----------------------------------------------------------------------------
  //Calculate reflection and transmission coefficients.
  real reflectionCoefficient = ReflectionCoefficient(nInT, uIuT, ray.Direction, 
                                                     results.Normal);
  real transmissionCoefficient = real(1.0) - reflectionCoefficient;
  reflectionCoefficient *= results.SpecularScalar;
  transmissionCoefficient *= results.SpecularScalar;

  //----------------------------------------------------------------------------
  //Local illumination.
  Vec4 color = Vec4::cZero;
  if(inAir)
  {
    color = CalculateLighting(ray, reflectionCoefficient, attenuation, results);
  }

  //----------------------------------------------------------------------------
  //Reflection.
  if(reflectionCoefficient != real(0.0))
  {
    Ray reflectedRay = ReflectedRay(ray, positionOffset, results);
    Vec4 reflectedColor = RayTraceScene(reflectedRay, inAir, depth - 1);
    color.AddScaledVector(reflectedColor, reflectionCoefficient);
  }

  //----------------------------------------------------------------------------
  //Transmission.
  if(transmissionCoefficient != real(0.0))
  {
    Ray transmittedRay = TransmittedRay(ray, nInT, -positionOffset, results);
    Negate(&transmittedRay.Direction);

    Vec4 transmittedColor = RayTraceScene(transmittedRay, !inAir, depth - 1);
    color.AddScaledVector(transmittedColor, transmissionCoefficient);
  }

  //----------------------------------------------------------------------------
  //Attenuation.
  {
    const real distance = results.IntersectionData.T;
    for(int i = 0; i < 4; ++i)
    {
      real attenuationScale = Math::Pow(attenuation[i], distance);
      if(attenuationScale > real(0.0))
      {
        color[i] *= attenuationScale;
      }
    }    
  }

  return color;
}

Vec4 RayTracer::CalculateLighting(RayParam ray, real reflectionCoefficient,
                                  Vec4Param materialAttenuation,
                                  const RayCastResults& results)
{
  //return Vec4(results.Normal.x, results.Normal.y, results.Normal.z, real(1.0));

  //Get all of the object's material information
  const Vec4& Kd = results.Diffuse;      //Object diffuse
  const real m = results.SpecularPower;  //Object specular power
  const real Ks = reflectionCoefficient; //Object specular scalar
  const Vec3 v = -ray.Direction;

  //Lighting is initially black.
  Vec4 color = Vec4::cZero;

  //Go through all of the visible lights and determine how much they contribute
  //to the current point's illumination and color.
  LightArray lightResults;
  ShadowRay shadowRay(results.IntersectionData.Points[0], results.Normal, 
                      cPositionOffset);
  SoftShadow softShadowFn(mProperties.ShadowSamples, shadowRay, mInterface);
  if(CastRayIntoLights(softShadowFn, true, lightResults))
  {
    forRange(LightCastResults& light, lightResults.All())
    {
      //Start this light's contribution off at black.
      Vec4 lightContribution = Vec4::cZero;

      const Vec3& normal = results.Normal;
      const Vec4& lightColor = light.Color;
      Vec3& pointToLight = light.PointToLight;
      real lightDistance = AttemptNormalize(&pointToLight);
      if(lightDistance == real(0.0))
      {
        continue;
      }

      //Diffuse
      {
        real dot = Dot(normal, pointToLight);
        if(dot > real(0.0))
        {
          lightContribution.AddScaledVector(Kd.ScaledByVector(lightColor), dot);
        }
      }

      //Specular
      {
        Vec3 r = real(2.0) * Dot(pointToLight, normal) * normal - pointToLight;
        real dot = Dot(r, v);
        if(dot > real(0.0))
        {
          Normalize(&r);
          dot = Math::Pow(Dot(r, v), m) * Ks;
          lightContribution.AddScaledVector(lightColor, dot);
        }
      }

      //Shadow effect
      lightContribution *= light.ShadowCoefficient;

      //Attenuation
      for(int i = 0; i < 4; ++i)
      {
        lightContribution[i] *= Math::Pow(materialAttenuation[i], 
                                          lightDistance);
      }

      color += lightContribution;
    }
  }

  //Ambient
  color += Kd.ScaledByVector(mProperties.AmbientColor);

  return color;
}

real RayTracer::ReflectionCoefficient(real airObjRefractionIndex, 
                                      real airObjMagneticPermeability, 
                                      Vec3Param rayDirection, 
                                      Vec3Param surfaceNormal)
{
  /*
     n = surface normal
     i = ray direction (incident vector)
     r = reflected direction
     t = transmission direction (transmitting vector)
    Oi = angle of incidence
    Ot = angle of transmission(?)
    Ni = index of refraction of the incident material
    Nt = index of refraction of the transmitting material
    Ui = magnetic permeability of the incident material
    Ut = magnetic permeability of the transmitting material

                    r      n      i
                     \     |     /
                      \    |    /
                       \   |   /
                        \  |Oi/  
       Ni, Ui            \ | /
      ____________________\|/______________________ this line represents surface
                          /|
       Nt, Ut            / |
                        /Ot|
                       /   |
                      /    |
                     t
  */
  const real nInT = airObjRefractionIndex;
  const real uIuT = airObjMagneticPermeability;

  //Cosine of the angle of incidence.
  real cosThetaI = Math::Abs(Dot(rayDirection, surfaceNormal));

  //Cosine of the angle of transmission.
  real cosThetaT = real(1.0) - Math::Sq(nInT) * 
                   (real(1.0) - Math::Sq(cosThetaI));

  //To avoid total internal reflection...
  if(cosThetaT < real(0.0))
  {
    return real(1.0);
  }

  //Complete the calculation of cos(Ot)  (can't sqrt negatives!)
  cosThetaT = Math::Sqrt(cosThetaT);

  //Just straight up math, not sure how to describe without a wall of text.
  real orthogonalRatio = (nInT * cosThetaI - uIuT * cosThetaT) /
                         (nInT * cosThetaI + uIuT * cosThetaT);
  real parallelRatio = (uIuT * cosThetaI - nInT * cosThetaT) /
                       (uIuT * cosThetaI + nInT * cosThetaT);
  return real(0.5) * (Math::Sq(orthogonalRatio) + Math::Sq(parallelRatio));
}

Ray RayTracer::ReflectedRay(RayParam ray, real positionOffset, 
                            const RayCastResults& results)
{
  const Vec3& p = results.IntersectionData.Points[0];
  const Vec3& n = results.Normal;
  Vec3 d = ray.Direction - real(2.0) * Dot(ray.Direction, n) * n;
  return Ray(p + (n * positionOffset), d);
}

Ray RayTracer::TransmittedRay(RayParam ray, real airObjRefractionIndex, 
                              real positionOffset, 
                              const RayCastResults& results)
{
  const Vec3& i = ray.Direction;
  const Vec3& n = results.Normal;
  const Vec3& surfacePoint = results.IntersectionData.Points[0];
  const real nInT = airObjRefractionIndex;

  real iDotN = Dot(i, n);
  real tDotN = Math::Sqrt(real(1.0) - Math::Sq(nInT) * 
                          (real(1.0) - Math::Sq(iDotN)));
  if(iDotN >= real(0.0))
  {
    tDotN = -tDotN;
  }

  Ray transmittedRay;
  transmittedRay.Start = surfacePoint + (n * positionOffset);
  transmittedRay.Direction = i * -nInT;
  transmittedRay.Direction.AddScaledVector(n, tDotN + nInT * iDotN);
  Normalize(&transmittedRay.Direction);
  return transmittedRay;
}

bool RayTracer::CastRayIntoScene(RayParam ray, bool refineResults, 
                                 RayCastResults* results)
{
  return mInterface->CastRayIntoScene(ray, refineResults, results);
}

bool RayTracer::CastRayIntoLights(SoftShadowFn softShadowFn, bool refineResults,
                                  LightArray& results)
{
  return mInterface->CastRayIntoLights(softShadowFn, refineResults, results);
}

Vec4 RayTracer::NormalSampling(const Pixel& pixel)
{
  Vec3 start, direction;
  mPixelSampler->GetRay(pixel.Center, &start, &direction);
  return mPixelSampler->RayTraceColor(start, direction);
}

Vec4 RayTracer::UniformSupersampling(const Pixel& pixel, uint sampleCount)
{
  const uint cSampleCount = sampleCount;
  const real cSubPixelWidth = pixel.Dimensions.x / real(cSampleCount);
  const real cSubPixelHeight = pixel.Dimensions.y / real(cSampleCount);
  const real cColorWeight = real(1.0) / real(cSampleCount * cSampleCount);

  Vec2 screenPos = pixel.Center;

  //Move to upper left subpixel center
  if(sampleCount % 2)
  {
    screenPos.x -= cSubPixelWidth / real(2.0);
    screenPos.y -= cSubPixelHeight / real(2.0);
    sampleCount--;
  }

  //Move to upper left most subpixel
  screenPos.x -= cSubPixelWidth * real(sampleCount / 2);
  screenPos.y += cSubPixelHeight * real(sampleCount / 2);

  Vec4 finalColor = Vec4::cZero;
  for(uint yStep = 0; yStep < cSampleCount; ++yStep)
  {
    Vec2 subPixelCenter = screenPos;
    for(uint xStep = 0; xStep < cSampleCount; ++xStep)
    {
      Vec3 start, direction;
      mPixelSampler->GetRay(subPixelCenter, &start, &direction);

      Vec4 subPixelColor = mPixelSampler->RayTraceColor(start, direction);
      finalColor += subPixelColor * cColorWeight;

      subPixelCenter.x += cSubPixelWidth;
    }
    screenPos.y -= cSubPixelHeight;
  }
  return finalColor;
}

Vec4 RayTracer::MonteCarloSupersampling(const Pixel& pixel, uint sampleCount)
{
  const real cColorWeight = real(1.0) / real(sampleCount);
  const real cPixelHalfWidth = pixel.Dimensions.x * real(0.5);
  const real cPixelHalfHeight = pixel.Dimensions.y * real(0.5);

  Vec3 start, direction;
  Vec4 color = Vec4::cZero;
  for(uint i = 0; i < sampleCount; ++i)
  {
    Vec2 randPosition = pixel.Center;
    {
      real randX = gRandom.FloatRange(-cPixelHalfWidth, cPixelHalfWidth);
      real randY = gRandom.FloatRange(-cPixelHalfHeight, cPixelHalfHeight);
      randPosition.x += randX;
      randPosition.y += randY;
    }
    mPixelSampler->GetRay(randPosition, &start, &direction);
    color += mPixelSampler->RayTraceColor(start, direction);
  }
  return color * cColorWeight;
}

Vec4 RayTracer::JitteredSupersampling(const Pixel& pixel, uint sampleCount)
{
  const uint cSampleCount = sampleCount;
  const real cSubPixelWidth = pixel.Dimensions.x / real(cSampleCount);
  const real cSubPixelHeight = pixel.Dimensions.y / real(cSampleCount);
  const real cHalfSubPixelWidth = real(0.5) * cSubPixelWidth;
  const real cHalfSubPixelHeight = real(0.5) * cSubPixelHeight;
  const real cColorWeight = real(1.0) / real(cSampleCount * cSampleCount);

  Vec2 screenPos = pixel.Center;

  //Move to upper left subpixel center
  if(sampleCount % 2)
  {
    screenPos.x -= cSubPixelWidth / real(2.0);
    screenPos.y -= cSubPixelHeight / real(2.0);
    sampleCount--;
  }

  //Move to upper left most subpixel
  screenPos.x -= cSubPixelWidth * real(sampleCount / 2);
  screenPos.y += cSubPixelHeight * real(sampleCount / 2);

  Vec4 finalColor = Vec4::cZero;
  for(uint yStep = 0; yStep < cSampleCount; ++yStep)
  {
    Vec2 subPixelCenter = screenPos;
    for(uint xStep = 0; xStep < cSampleCount; ++xStep)
    {
      Vec2 randSubPixelCenter = subPixelCenter;
      {
        real randX = gRandom.FloatRange(-cHalfSubPixelWidth, cHalfSubPixelWidth);
        real randY = gRandom.FloatRange(-cHalfSubPixelHeight, cHalfSubPixelHeight);
        randSubPixelCenter.x += randX;
        randSubPixelCenter.y += randY;
      }

      Vec3 start, direction;
      mPixelSampler->GetRay(randSubPixelCenter, &start, &direction);

      Vec4 subPixelColor = mPixelSampler->RayTraceColor(start, direction);
      finalColor += subPixelColor * cColorWeight;

      subPixelCenter.x += cSubPixelWidth;
    }
    screenPos.y -= cSubPixelHeight;
  }
  return finalColor;
}

Vec4 RayTracer::AdaptiveSupersampling(const Pixel& pixel, uint sampleCount)
{
  //Test the four corners of the pixel.
  {
    Vec4 colors[4];
    Vec3 rayStart, rayDirection;

    Vec2 pixelCorner = pixel.Center;
    pixelCorner.AddScaledVector(Vec2::cXAxis, pixel.Dimensions[0] * real(0.5));
    pixelCorner.AddScaledVector(Vec2::cYAxis, pixel.Dimensions[1] * real(0.5));
    mPixelSampler->GetRay(pixelCorner, &rayStart, &rayDirection);
    colors[0] = mPixelSampler->RayTraceColor(rayStart, rayDirection);

    pixelCorner = pixel.Center;
    pixelCorner.AddScaledVector(Vec2::cXAxis, pixel.Dimensions[0] * real(-0.5));
    pixelCorner.AddScaledVector(Vec2::cYAxis, pixel.Dimensions[1] * real(0.5));
    mPixelSampler->GetRay(pixelCorner, &rayStart, &rayDirection);
    colors[1] = mPixelSampler->RayTraceColor(rayStart, rayDirection);

    pixelCorner = pixel.Center;
    pixelCorner.AddScaledVector(Vec2::cXAxis, pixel.Dimensions[0] * real(0.5));
    pixelCorner.AddScaledVector(Vec2::cYAxis, pixel.Dimensions[1] * real(-0.5));
    mPixelSampler->GetRay(pixelCorner, &rayStart, &rayDirection);
    colors[2] = mPixelSampler->RayTraceColor(rayStart, rayDirection);

    pixelCorner = pixel.Center;
    pixelCorner.AddScaledVector(Vec2::cXAxis, pixel.Dimensions[0] * real(-0.5));
    pixelCorner.AddScaledVector(Vec2::cYAxis, pixel.Dimensions[1] * real(-0.5));
    mPixelSampler->GetRay(pixelCorner, &rayStart, &rayDirection);
    colors[3] = mPixelSampler->RayTraceColor(rayStart, rayDirection);

    //Make sure all of the colors are within a given tolerance of each other.
    if((sampleCount == 0) || ColorComparison(colors, 4))
    {
      colors[0] += colors[1] + colors[2] + colors[3];
      colors[0] *= real(0.25);
      return colors[0];
    }
  }

  //Subdivide the current pixel into 4 pixels and try again.
  Vec4 color;

  //Top right?
  {
    Pixel subPixel = Pixel(pixel.Center, pixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cXAxis * (subPixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cYAxis * (subPixel.Dimensions * real(0.5));
    color = AdaptiveSupersampling(subPixel, sampleCount - 1);
  }

  //Top left?
  {
    Pixel subPixel = Pixel(pixel.Center, pixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cXAxis * (subPixel.Dimensions * real(-0.5));
    subPixel.Center += Vec2::cYAxis * (subPixel.Dimensions * real(0.5));
    color += AdaptiveSupersampling(subPixel, sampleCount - 1);
  }
  
  //Bottom left?
  {
    Pixel subPixel = Pixel(pixel.Center, pixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cXAxis * (subPixel.Dimensions * real(-0.5));
    subPixel.Center += Vec2::cYAxis * (subPixel.Dimensions * real(-0.5));
    color += AdaptiveSupersampling(subPixel, sampleCount - 1);
  }

  //Bottom right?
  {
    Pixel subPixel = Pixel(pixel.Center, pixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cXAxis * (subPixel.Dimensions * real(0.5));
    subPixel.Center += Vec2::cYAxis * (subPixel.Dimensions * real(-0.5));
    color += AdaptiveSupersampling(subPixel, sampleCount - 1);
  }

  color *= real(0.25);

  return color;
}

}// namespace Zero
