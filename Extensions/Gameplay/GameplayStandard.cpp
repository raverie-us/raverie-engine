///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "WebBrowserChrome.hpp"

#include "IndexedHalfEdgeMesh.hpp"
#include "QuickHull3DBindings.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(WebBrowserModifiers);
ZilchDefineEnum(OrientationBases);
ZilchDefineEnum(SplineAnimatorMode);

ZilchDefineRange(IndexedHalfEdgeMeshVertexArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeMeshEdgeArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeFaceEdgeIndexArray::RangeType);
ZilchDefineRange(IndexedHalfEdgeMeshFaceArray::RangeType);

//**************************************************************************************************
ZilchDefineStaticLibrary(GameplayLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  // Enums
  ZilchInitializeEnum(WebBrowserModifiers);
  ZilchInitializeEnum(OrientationBases);
  ZilchInitializeEnum(SplineAnimatorMode);

  // Ranges
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshVertexArray::RangeType, "IndexedHalfEdgeMeshVertexArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshEdgeArray::RangeType, "IndexedHalfEdgeMeshEdgeArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeFaceEdgeIndexArray::RangeType, "IndexedHalfEdgeFaceEdgeIndexArrayRange");
  ZilchInitializeRangeAs(IndexedHalfEdgeMeshFaceArray::RangeType, "IndexedHalfEdgeMeshFaceArrayRange");

  // Meta Components
  ZilchInitializeType(RaycasterMetaComposition);

  // Events
  ZilchInitializeType(MouseEvent);
  ZilchInitializeType(ViewportMouseEvent);
  ZilchInitializeType(WebBrowserEvent);
  ZilchInitializeType(WebBrowserConsoleEvent);
  ZilchInitializeType(WebBrowserCursorEvent);
  ZilchInitializeType(WebBrowserDownloadEvent);
  ZilchInitializeType(WebBrowserPopupCreateEvent);
  ZilchInitializeType(WebBrowserPointQueryEvent);
  ZilchInitializeType(WebBrowserTextEvent);
  ZilchInitializeType(WebBrowserUrlEvent);

  ZilchInitializeType(Viewport);
  ZilchInitializeType(ReactiveViewport);
  ZilchInitializeType(GameWidget);

  ZilchInitializeType(WebBrowser);
  ZilchInitializeType(WebBrowserSetup);
  WebBrowserManager::PlatformInitializeMeta<ZilchLibrary>();
  ZilchInitializeType(WebBrowserManager);
  ZilchInitializeType(WebBrowserWidget);
  ZilchInitializeType(MarketWidget);
  ZilchInitializeType(TileMapSource);
  ZilchInitializeType(Reactive);
  ZilchInitializeType(ReactiveSpace);
  ZilchInitializeType(MouseCapture);
  ZilchInitializeType(Orientation);
  ZilchInitializeType(TileMap);
  ZilchInitializeType(RandomContext);
  ZilchInitializeType(CameraViewport);
  ZilchInitializeType(DefaultGameSetup);

  ZilchInitializeType(SplineParticleEmitter);
  ZilchInitializeType(SplineParticleAnimator);

  ZilchInitializeType(UnitTestSystem);
  ZilchInitializeType(UnitTestEvent);
  ZilchInitializeType(UnitTestEndFrameEvent);
  ZilchInitializeType(UnitTestBaseMouseEvent);
  ZilchInitializeType(UnitTestMouseEvent);
  ZilchInitializeType(UnitTestMouseDropEvent);
  ZilchInitializeType(UnitTestKeyboardEvent);
  ZilchInitializeType(UnitTestKeyboardTextEvent);
  ZilchInitializeType(UnitTestWindowEvent);

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

//**************************************************************************************************
void GameplayLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(TileMapSourceManager);
}

//**************************************************************************************************
void GameplayLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Zero
