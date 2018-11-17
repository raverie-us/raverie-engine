///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineStaticLibrary(SpatialPartitionLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(IBroadPhase);
  ZilchInitializeType(NSquaredBroadPhase);
  ZilchInitializeType(BoundingBoxBroadPhase);
  ZilchInitializeType(BoundingSphereBroadPhase);
  ZilchInitializeType(StaticAabbTreeBroadPhase);
  ZilchInitializeType(SapBroadPhase);
  ZilchInitializeType(DynamicAabbTreeBroadPhase);
  ZilchInitializeType(AvlDynamicAabbTreeBroadPhase);
  ZilchInitializeType(DynamicBroadphasePropertyExtension);
  ZilchInitializeType(StaticBroadphasePropertyExtension);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void SpatialPartitionLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void SpatialPartitionLibrary::Shutdown()
{

}

}// namespace Zero
