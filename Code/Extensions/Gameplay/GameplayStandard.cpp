// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

#include "IndexedHalfEdgeMesh.hpp"
#include "QuickHull3DBindings.hpp"

namespace Raverie
{

// Enums
RaverieDefineEnum(OrientationBases);
RaverieDefineEnum(SplineAnimatorMode);
RaverieDefineEnum(PathFinderStatus);

RaverieDefineRange(IndexedHalfEdgeMeshVertexArray::RangeType);
RaverieDefineRange(IndexedHalfEdgeMeshEdgeArray::RangeType);
RaverieDefineRange(IndexedHalfEdgeFaceEdgeIndexArray::RangeType);
RaverieDefineRange(IndexedHalfEdgeMeshFaceArray::RangeType);

RaverieDefineStaticLibrary(GameplayLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  RaverieInitializeEnum(OrientationBases);
  RaverieInitializeEnum(SplineAnimatorMode);
  RaverieInitializeEnum(PathFinderStatus);

  // Ranges
  RaverieInitializeRangeAs(IndexedHalfEdgeMeshVertexArray::RangeType, "IndexedHalfEdgeMeshVertexArrayRange");
  RaverieInitializeRangeAs(IndexedHalfEdgeMeshEdgeArray::RangeType, "IndexedHalfEdgeMeshEdgeArrayRange");
  RaverieInitializeRangeAs(IndexedHalfEdgeFaceEdgeIndexArray::RangeType, "IndexedHalfEdgeFaceEdgeIndexArrayRange");
  RaverieInitializeRangeAs(IndexedHalfEdgeMeshFaceArray::RangeType, "IndexedHalfEdgeMeshFaceArrayRange");

  // Events
  RaverieInitializeType(MouseEvent);
  RaverieInitializeType(MouseFileDropEvent);
  RaverieInitializeType(ViewportMouseEvent);

  RaverieInitializeType(Viewport);
  RaverieInitializeType(ReactiveViewport);
  RaverieInitializeType(GameWidget);

  RaverieInitializeType(TileMapSource);
  RaverieInitializeType(Reactive);
  RaverieInitializeType(ReactiveSpace);
  RaverieInitializeType(MouseCapture);
  RaverieInitializeType(Orientation);
  RaverieInitializeType(TileMap);
  RaverieInitializeType(RandomContext);
  RaverieInitializeType(CameraViewport);
  RaverieInitializeType(DefaultGameSetup);
  RaverieInitializeType(PathFinderBaseEvent);
  RaverieInitializeTypeAs(PathFinderEvent<Vec3>, "PathFinderEvent");
  RaverieInitializeTypeAs(PathFinderEvent<IntVec3>, "PathFinderGridEvent");
  RaverieInitializeType(PathFinder);
  RaverieInitializeType(PathFinderRequest);
  RaverieInitializeType(PathFinderGrid);
  RaverieInitializeType(PathFinderMesh);

  RaverieInitializeType(SplineParticleEmitter);
  RaverieInitializeType(SplineParticleAnimator);

  RaverieInitializeType(IndexedHalfEdgeMeshVertexArray);
  RaverieInitializeType(IndexedHalfEdgeMeshEdgeArray);
  RaverieInitializeType(IndexedHalfEdgeFaceEdgeIndexArray);
  RaverieInitializeType(IndexedHalfEdgeMeshFaceArray);
  RaverieInitializeType(IndexedHalfEdge);
  RaverieInitializeType(IndexedHalfEdgeFace);
  RaverieInitializeType(IndexedHalfEdgeMesh);
  RaverieInitializeTypeAs(QuickHull3DInterface, "QuickHull3D");

  RaverieInitializeTypeAs(RaverieStatic, "Raverie");

  // @trevor.sundberg: The Gameplay and Editor libraries are co-dependent
  RaverieTypeId(Editor)->AssertOnInvalidBinding = &IgnoreOnInvalidBinding;

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

} // namespace Raverie
