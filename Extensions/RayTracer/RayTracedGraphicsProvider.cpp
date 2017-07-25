///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedGraphicsProvider.cpp
///  Implementation of the ray-traced graphics provider class.
///
/// Authors: Trevor Sundberg, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "RayTracer/RayTracedGraphicsProvider.hpp"
#include "Engine/Space.hpp"
#include "RayTracer/RayTraceProperties.hpp"

namespace Zero
{

namespace
{
typedef GraphicsBroadPhase::BaseTreeRange<Ray> RayRange;
typedef LightList::range                       LightRange;

}//namespace

//--------------------------------------------------------- File Scope Functions
void FillRayCastResults(Cog* object, RayParam ray, bool refineResults,
                        Vec3Param normal, Vec2Param uv, real closestT,
                        RayCastResults* results);


//-------------------------------------------------------- Ray Tracing Functions
RayTracingFunctions::RayTracingFunctions(void)
  : mSpace(NULL)
{
  //
}

RayTracingFunctions::RayTracingFunctions(Space* space)
  : mSpace(space)
{
  //
}

RayTracingFunctions::~RayTracingFunctions(void)
{
  //
}

void RayTracingFunctions::SetSpace(Space* space)
{
  mSpace = space;
}

bool RayTracingFunctions::CastRayIntoScene(RayParam ray, bool refineResults,
                                           RayCastResults* results)
{
  //Get the graphical space from the sceneData.
  GraphicsSpace* graphicsSpace = mSpace->has(GraphicsSpace);
  if(graphicsSpace == NULL)
  {
    return false;
  }

  //Find the closest point of intersection
  real closestHit = Math::PositiveMax();
  Cog* objectHit = NULL;
  Vec3 normal = Vec3::cZero;
  Vec2 uv = Vec2::cZero;

  forRangeBroadphaseTree(GraphicsBroadPhase, graphicsSpace->BroadPhase, Ray, ray)
  {
    Graphical* object = range.Front();

    GraphicsRayCast rayCast;
    rayCast.CastRay = ray;

    if(object->TestRay(rayCast))
    {
      if(rayCast.T < closestHit)
      {
        closestHit = rayCast.T;
        objectHit = object->GetOwner();
        normal = rayCast.Normal;
        uv = rayCast.Uv;
      }
    }
  }

  //----------------------------------------------------------------------------
  if(closestHit < Math::PositiveMax())
  {
    FillRayCastResults(objectHit, ray, refineResults, normal, uv, closestHit,
                       results);
    return true;
  }
  return false;
}

bool RayTracingFunctions::CastRayIntoLights(SoftShadowFn softShadowFn,
                                            bool refineResults,
                                            LightArray& results)
{
  //Get the graphical space from the space.
  GraphicsSpace* graphicsSpace = mSpace->has(GraphicsSpace);
  if(graphicsSpace == NULL)
  {
    return false;
  }

  results.Clear();

  //Intersect all of the lights in the scene.
  // LightRange ranges[4] = { graphicsSpace->Objects.Visible.Lights.All(),
  //                          graphicsSpace->Objects.ToProcess.Lights.All(),
  //                          graphicsSpace->Objects.NotVisible.Lights.All(),
  //                          graphicsSpace->Objects.LastVisible.Lights.All() };
  // for(uint i = 0; i < 4; ++i)
  // {
  //   LightRange& range = ranges[i];
  //   forRange(Light& light, range)
  //   {
  //     if(light.mActive)
  //     {
  //       const Vec3& lightPos = light.mTransform->GetTranslation();
  //       real shadowCoef = softShadowFn(lightPos, light.mRange);
  //       if(shadowCoef > real(0.0))
  //       {
  //         //Point can see the light, add
  //         results.PushBack(LightCastResults(lightPos - softShadowFn.RayStart(),
  //                                            shadowCoef, light.mColor));
  //       }
  //     }
  //     range.PopFront();
  //   }
  // }
  return true;
}

//------------------------------------------------- Ray Traced Graphics Provider
ZeroDefineType(RayTracedGraphicsProvider);

// Bind any needed properties
void RayTracedGraphicsProvider::InitializeMeta(MetaType* meta)
{
  BindBase(RayTracedProvider);
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindFieldProperty(mDepth);
  ZilchBindGetterSetterProperty(ShadowSamples);
  ZilchBindGetterSetterProperty(PixelSamples);
  ZilchBindGetterSetterProperty(AmbientColor);
  ZilchBindGetterSetterProperty(AirAttenuation);
  ZilchBindGetterSetterProperty(PixelSampleType);  //METAREFACTOR enum AntiAliasingType
  //BindEnumProperty(SoftwareSampleMode, TextureSampleType);
}

// Constructor
RayTracedGraphicsProvider::RayTracedGraphicsProvider(void)
  : mDepth(1), mRayTracingFunctions(mSpace)
{
  mShadowSamples = mRayTracerConfig.ShadowSamples;
  mPixelSamples = mRayTracerConfig.PixelSamples;
  mAmbientColor = mRayTracerConfig.AmbientColor;
  mAirAttenuation = mRayTracerConfig.AirAttenuation;
}

RayTracedGraphicsProvider::~RayTracedGraphicsProvider(void)
{
  //
}

// Perform a ray-trace given a ray, and output the color that was determined
Vec4 RayTracedGraphicsProvider::RayTraceColor(Vec4Param backgroundColor,
                                              Vec3Param start,
                                              Vec3Param direction,
                                              IntVec2Param pixelPosition)
{
  return RayTraceColor(backgroundColor, start, direction, mDepth);
}

// Perform a ray-trace given a ray, and output the color that was determined
Vec4 RayTracedGraphicsProvider::RayTraceColor(Vec4Param backgroundColor,
                                              Vec3Param start,
                                              Vec3Param direction, int depth)
{
  mRayTracerConfig.ClearColor = backgroundColor;
  mRayTracer.SetProperties(mRayTracerConfig);

  Ray ray = Ray(start, direction);
  return mRayTracer.RayTraceScene(ray, depth);
}

// An overridable method that is called when we begin rendering to a frame
void RayTracedGraphicsProvider::BeginFrame(Space* space, PixelBuffer* buffer)
{
  mSpace = space;
  mRayTracingFunctions.SetSpace(mSpace);
  mRayTracer.Init(&mRayTracingFunctions);
}

Vec4 RayTracedGraphicsProvider::RayTraceColor(const Pixel& pixel,
                                              PixelSampler* pixelSampler)
{
  mRayTracer.SetPixelSampler(pixelSampler);
  return mRayTracer.CalculatePixelColor(pixel);
}

// An overridable method that is called when we end rendering to a frame
void RayTracedGraphicsProvider::EndFrame(Space* space, PixelBuffer* buffer)
{
  int i = 0;
  i = 3;
}

uint RayTracedGraphicsProvider::GetShadowSamples(void)
{
  return mShadowSamples;
}

void RayTracedGraphicsProvider::SetShadowSamples(uint shadowSamples)
{
  mRayTracerConfig.ShadowSamples = mShadowSamples = shadowSamples;
  mRayTracer.SetProperties(mRayTracerConfig);
}

uint RayTracedGraphicsProvider::GetPixelSamples(void)
{
  return mPixelSamples;
}

void RayTracedGraphicsProvider::SetPixelSamples(uint pixelSamples)
{
  mRayTracerConfig.PixelSamples = mPixelSamples = pixelSamples;
  mRayTracer.SetProperties(mRayTracerConfig);
}

Vec4 RayTracedGraphicsProvider::GetAirAttenuation(void)
{
  return mAirAttenuation;
}

void RayTracedGraphicsProvider::SetAirAttenuation(Vec4 airAttenuation)
{
  mRayTracerConfig.AirAttenuation = mAirAttenuation = airAttenuation;
  mRayTracer.SetProperties(mRayTracerConfig);
}

Vec4 RayTracedGraphicsProvider::GetAmbientColor(void)
{
  return mAmbientColor;
}

void RayTracedGraphicsProvider::SetAmbientColor(Vec4 ambientColor)
{
  mRayTracerConfig.AmbientColor = mAmbientColor = ambientColor;
  mRayTracer.SetProperties(mRayTracerConfig);
}

uint RayTracedGraphicsProvider::GetPixelSampleType(void)
{
  return uint(mPixelSampleType);
}
void RayTracedGraphicsProvider::SetPixelSampleType(uint sampleType)
{
  mRayTracerConfig.SampleType = mPixelSampleType = AntiAliasingType::Enum(sampleType);
  mRayTracer.SetProperties(mRayTracerConfig);
}

uint RayTracedGraphicsProvider::GetTextureSampleType(void)
{
  return uint(mTextureSampleType);
}

void RayTracedGraphicsProvider::SetTextureSampleType(uint sampleType)
{
  mTextureSampleType = SoftwareSampleMode::Enum(sampleType);
}

//---------------------------------------------- Ray Tracing Functions - Helpers
void FillRayCastResults(Cog* object, RayParam ray, bool refineResults,
                        Vec3Param normal, Vec2Param uv, real closestT,
                        RayCastResults* results)
{
  if(results != NULL)
  {
    RayTraceProperties* properties = object->has(RayTraceProperties);

    //Intersection point. Could be recalculated based on the value of the
    //"refineResults" parameter.
    Vec3 refinedPoint = ray.GetPoint(closestT);

    //Copy over the normal from the
    results->Normal = normal;
    results->Uv = uv;

    //Get the normal from the object if it has a model, also re-intersect the
    //object if we were told to do so.
    Model* model = object->has(Model);
    if(model)
    {
      //Re-intersect the object with a closer starting position to counteract
      //floating point imprecision.
      if(refineResults)
      {
        //Start the ray 90% of the way closer to the object.
        const real cCloserStartingPoint = closestT * real(0.90);
        Ray closeRay = Ray(ray.Start + (ray.Direction * cCloserStartingPoint),
                           ray.Direction);

        //Using the new closer ray, intersect the object again to get more
        //precise intersection information.
        GraphicsRayCast graphicsRay;
        graphicsRay.CastRay = closeRay;
        if(model->TestRay(graphicsRay))
        {
          refinedPoint = closeRay.GetPoint(graphicsRay.T);
          results->Normal = graphicsRay.Normal;
          results->Uv = graphicsRay.Uv;
        }
      }

      //Get the color of the intersected point from the model.
      results->Diffuse = model->mColor;
      if(properties != NULL && properties->GetUseTexture())
      {
        Material* material = model->GetMaterial();

        if(material != NULL)
        {
          TextureDiffuse* textureDiffuse = material->has(TextureDiffuse);

          if(textureDiffuse != NULL)
          {
            real mipmapLevel = real(0.0);
            Texture* diffuse = textureDiffuse->Diffuse;

            Vec4 byteColor = ToFloatColor(diffuse->Sample(SoftwareSampleMode::Linear,
                                                          mipmapLevel, results->Uv));
            results->Diffuse = byteColor;
          }
        }
      }
    }

    //Get all of the extra ray tracing specific information from the
    //RayTraceProperties component, if it exists.
    if(properties)
    {
      results->Attenuation = properties->GetAttenuation();
      results->SpecularScalar = properties->GetSpecularScalar();
      results->SpecularPower = properties->GetSpecularPower();
      results->ElectricPermittivity = properties->GetElectricPermittivity();
      results->MagneticPermeability = properties->GetMagneticPermeability();
      results->IndexOfRefraction = properties->GetIndexOfRefraction();
    }

    //Store the results of the ray cast and recompute the t-value (in case the
    //refinement step took place).
    results->IntersectionData.Points[0] = refinedPoint;
    results->IntersectionData.T = Dot(refinedPoint - ray.Start, ray.Direction);
  }
}

}//namespace Zero
