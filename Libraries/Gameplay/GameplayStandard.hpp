// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"
#include "Geometry/GeometryStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "Support/SupportStandard.hpp"

// Zilch Library Dependencies
#include "Zilch/Precompiled.hpp"
using namespace Zilch;

namespace Zero
{

// Gameplay Library
class ZeroNoImportExport GameplayLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(GameplayLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};
} // namespace Zero

// Core Library Dependencies
#include "Engine/EngineStandard.hpp"
#include "Physics/PhysicsStandard.hpp"
#include "Graphics/GraphicsStandard.hpp"
#include "Widget/WidgetStandard.hpp"
#include "Sound/SoundStandard.hpp"

// Gameplay Includes
#include "UnitTestSystem.hpp"
#include "Orientation.hpp"

#include "Reactive.hpp"
#include "ReactiveViewport.hpp"
#include "MouseCapture.hpp"
#include "CameraViewport.hpp"

#include "DefaultGame.hpp"
#include "PlayGame.hpp"

#include "PriorityQueue.hpp"
#include "PathFinder.hpp"
#include "PathFinderGrid.hpp"
#include "PathFinderMesh.hpp"

#include "MarchingSquares.hpp"
#include "RandomContext.hpp"

#include "MarchingCubes.hpp"

#include "TileMap.hpp"
#include "TileMapSource.hpp"
#include "SplineParticles.hpp"

#include "WebBrowser.hpp"
#include "WebBrowserWidget.hpp"
#include "TileMapSourceLoadPattern.hpp"
#include "Zero.hpp"

#include "Importer.hpp"
#include "Exporter.hpp"
