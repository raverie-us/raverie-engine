// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"
#include "QuickHull3DBindings.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(OrientationBases);
ZilchDefineEnum(SplineAnimatorMode);
ZilchDefineEnum(PathFinderStatus);

ZilchDefineRange(IndexedHalfEdgeMeshVertexArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeMeshEdgeArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeFaceEdgeIndexArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeMeshFaceArray::RangeType);

ZilchDefineStaticLibrary(GameplayLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(OrientationBases);
  ZilchInitializeEnum(SplineAnimatorMode);
  ZilchInitializeEnum(PathFinderStatus);

  // Ranges
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshVertexArray::RangeType, "IndexedHalfEdgeMeshVertexArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshEdgeArray::RangeType, "IndexedHalfEdgeMeshEdgeArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeFaceEdgeIndexArray::RangeType, "IndexedHalfEdgeFaceEdgeIndexArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshFaceArray::RangeType, "IndexedHalfEdgeMeshFaceArrayRange");

  // Events
  ZilchInitializeType(MouseEvent);
  ZilchInitializeType(MouseFileDropEvent);
  ZilchInitializeType(ViewportMouseEvent);

  ZilchInitializeType(Viewport);
  ZilchInitializeType(ReactiveViewport);
  ZilchInitializeType(GameWidget);

  ZilchInitializeType(TileMapSource);
  ZilchInitializeType(Reactive);
  ZilchInitializeType(ReactiveSpace);
  ZilchInitializeType(MouseCapture);
  ZilchInitializeType(Orientation);
  ZilchInitializeType(TileMap);
  ZilchInitializeType(RandomContext);
  ZilchInitializeType(CameraViewport);
  ZilchInitializeType(DefaultGameSetup);
  ZilchInitializeType(PathFinderBaseEvent);
  ZilchInitializeTypeAs(PathFinderEvent<Vec3>, "PathFinderEvent");
  ZilchInitializeTypeAs(PathFinderEvent<IntVec3>, "PathFinderGridEvent");
  ZilchInitializeType(PathFinder);
  ZilchInitializeType(PathFinderRequest);
  ZilchInitializeType(PathFinderGrid);
  ZilchInitializeType(PathFinderMesh);

  ZilchInitializeType(SplineParticleEmitter);
  ZilchInitializeType(SplineParticleAnimator);

  ZilchInitializeType(IndexedHalfEdgeMeshVertexArray);
  ZilchInitializeType(IndexedHalfEdgeMeshEdgeArray);
  ZilchInitializeType(IndexedHalfEdgeFaceEdgeIndexArray);
  ZilchInitializeType(IndexedHalfEdgeMeshFaceArray);
  ZilchInitializeType(IndexedHalfEdge);
  ZilchInitializeType(IndexedHalfEdgeFace);
  ZilchInitializeType(IndexedHalfEdgeMesh);
  ZilchInitializeTypeAs(QuickHull3DInterface, "QuickHull3D");

  ZilchInitializeTypeAs(ZeroStatic, "Zero");

  // @trevor.sundberg: The Gameplay and Editor libraries are co-dependent
  ZilchTypeId(Editor)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void GameplayLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(TileMapSourceManager);
}

void GameplayLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
