// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineStaticLibrary(SpatialPartitionLibrary)
{
  builder.CreatableInScriptDefault = false;

  RaverieInitializeType(IBroadPhase);
  RaverieInitializeType(NSquaredBroadPhase);
  RaverieInitializeType(BoundingBoxBroadPhase);
  RaverieInitializeType(BoundingSphereBroadPhase);
  RaverieInitializeType(StaticAabbTreeBroadPhase);
  RaverieInitializeType(SapBroadPhase);
  RaverieInitializeType(DynamicAabbTreeBroadPhase);
  RaverieInitializeType(AvlDynamicAabbTreeBroadPhase);
  RaverieInitializeType(DynamicBroadphasePropertyExtension);
  RaverieInitializeType(StaticBroadphasePropertyExtension);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void SpatialPartitionLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void SpatialPartitionLibrary::Shutdown()
{
}

} // namespace Raverie
