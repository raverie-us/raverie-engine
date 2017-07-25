///////////////////////////////////////////////////////////////////////////////
///
/// \file RayTracedPhysicsProvider.cpp
///  Implementation of the ray-traced physics provider class.
///
/// Authors: Trevor Sundberg
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "RayTracedPhysicsProvider.hpp"
#include "Physics/PhysicsSpace.hpp"
#include "Engine/Noise.hpp"

namespace Zero
{
ZeroDefineType(RayTracedPhysicsProvider);

// Bind any needed properties
void RayTracedPhysicsProvider::InitializeMeta(MetaType* meta)
{
  //BindBase(RayTracedProvider);

  ZeroBindSetup(SetupMode::DefaultConstructor);

  ZilchBindFieldProperty(mBasicLighting);
  ZilchBindFieldProperty(mDrawTransparent);
  ZilchBindFieldProperty(mObjectColorMode);  //METAREFACTOR enum
}

// Constructor
RayTracedPhysicsProvider::RayTracedPhysicsProvider()
{
  // By default, objects are opaque
  mDrawTransparent = false;

  // Use the physics modes by default for object color
  mObjectColorMode = ObjectColorMode::PhysicsModes;

  // We do want basic lighting on
  mBasicLighting = true;
}

// Perform a ray-trace given a ray, and output the color that was determined
Vec4 RayTracedPhysicsProvider::RayTraceColor(Vec4Param backgroundColor, 
                                             Vec3Param start,
                                             Vec3Param direction,
                                             IntVec2Param pixelPosition)
{
  return RayTraceColor(backgroundColor, start, direction, pixelPosition, 0);
}

// Perform a ray-trace given a ray, and output the color that was determined
Vec4 RayTracedPhysicsProvider::RayTraceColor(Vec4Param backgroundColor, 
                                             Vec3Param start,
                                             Vec3Param direction,
                                             IntVec2Param pixelPosition,
                                             int depth)
{
//   {
//     float noise = PerlinNoise((pixelPosition.x - 400.0f) * 0.1f, (pixelPosition.y - 400.0f) * 0.1f);
//     //float noise = PerlinNoise(direction.x * 50.0f, direction.y * 50.0f);
//     //float noise = InterpolatedNoise((pixelPosition.x - 400.0f) * 0.1f, (pixelPosition.y - 400.0f) * 0.1f);
//     //float noise = SmoothNoise(pixelPosition.x - 400, pixelPosition.y - 400);
//     //float noise = Noise(pixelPosition.x - 400, pixelPosition.y - 400);
// 
//     // Move from [-1, 1] to [0, 1]
//     noise = (noise + 1.0f) / 2.0f;
// 
//     return Vec4(noise, noise, noise, 1.0f);
//   }

  PhysicsSpace* physics = this->GetSpace()->has(PhysicsSpace);
    
  if (physics == NULL)
    return Vec4(0, 0, 0, 1);

  mResults.Clear();
  Ray worldRay(start, direction);
  physics->CastRay(worldRay, mResults);

  Vec4 finalColor = backgroundColor;

  forRange(CastResult& hit, mResults.All())
  {
    float lighting;
          
    if (mBasicLighting)
    {
      lighting= Math::Clamp(-Math::Dot(hit.GetNormal(), direction));
    }
    else
    {
      lighting = 1.0f;
    }

    Collider* collider = hit.GetCollider();

    Vec4 color = GetObjectColor(hit);
    
    if (mDrawTransparent)
    {
      finalColor += color * lighting;
    }
    else
    {
      finalColor = color * lighting;
    }

    if (depth > 0)
    {
      Vec3 newDirection = -direction.ReflectAcrossVector(hit.GetNormal());
      finalColor += RayTraceColor(backgroundColor, hit.GetWorldPosition(), 
                                  newDirection, pixelPosition, depth - 1);
    }
  }

  return finalColor;
}

//Vec4 RayTracedPhysicsProvider::RayTraceColor(const Pixel& pixel, 
//                                             PixelSampler* pixelSampler)
//{
//  Vec3 rayStart, rayDirection;
//  pixelSampler->GetRay(pixel.Center, &rayStart, &rayDirection);
//  return pixelSampler->RayTraceColor(rayStart, rayDirection);
//}

// An overridable method that is called when we begin rendering to a frame
// Do any setup work for the frame here (gathering lights, etc)
//void RayTracedPhysicsProvider::BeginFrame(Space* space, PixelBuffer* buffer)
//{
//  if (mDrawTransparent)
//  {
//    mResults.Resize(500);
//  }
//  else
//  {
//    mResults.Resize(1);
//  }
//}

// Get the color of an object
Vec4 RayTracedPhysicsProvider::GetObjectColor(CastResult& hit)
{
  switch (mObjectColorMode)
  {
    //case ObjectColorMode::ModelColor:
    //{
    //  Model* model = hit.GetObjectHit()->has(Model);

    //  if (model)
    //  {
    //    return model->mColor;
    //  }
    //  else
    //  {
    //    return Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //  }
    //}

    case ObjectColorMode::PhysicsModes:
    {
      Collider* collider = hit.GetCollider();

      if (collider->IsStatic())
      {
        return Vec4(0.1f, 1.0f, 0.1f, 1.0f);
      }
      else if (collider->IsDynamic())
      {
        Vec4 color = Vec4(1.0f, 0.1f, 0.1f, 1.0f);

        if (collider->GetGhost())
        {
          color.y = 1.0f;
        }

        if (collider->IsAsleep())
        {
          color.z = 1.0f;
        }

        return color;
      }
      else if (collider->IsKinematic())
      {
        return Vec4(0.1f, 0.1f, 1.0f, 1.0f);
      }
      else
      {
        return Vec4(0.5f, 0.5f, 0.5f, 1.0f);
      }
    }

    case ObjectColorMode::Normals:
    {
      Vec3 normal = hit.GetNormal();
      return Vec4(normal.x, normal.y, normal.z, 1.0f);
    }

    case ObjectColorMode::InverseNormals:
    {
      Vec3 normal = -hit.GetNormal();
      return Vec4(normal.x, normal.y, normal.z, 1.0f);
    }

    case ObjectColorMode::AbsNormals:
    {
      Vec3 normal = Math::Abs(hit.GetNormal());
      return Vec4(normal.x, normal.y, normal.z, 1.0f);
    }
  }

  return Vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

}//namespace Zero
