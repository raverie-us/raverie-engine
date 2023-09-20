// MIT Licensed (see LICENSE.md).
#pragma once

// Standard Library Dependencies
#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/Geometry/GeometryStandard.hpp"
#include "Foundation/Meta/MetaStandard.hpp"
#include "Foundation/Support/SupportStandard.hpp"

// Raverie Library Dependencies
#include "Foundation/RaverieLanguage/Precompiled.hpp"
using namespace Raverie;

namespace Raverie
{

// Gameplay Library
class GameplayLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(GameplayLibrary);

  static void Initialize();
  static void Shutdown();
};
} // namespace Raverie

// Core Library Dependencies
#include "Systems/Engine/EngineStandard.hpp"
#include "Systems/Physics/PhysicsStandard.hpp"
#include "Systems/Graphics/GraphicsStandard.hpp"
#include "Extensions/Widget/WidgetStandard.hpp"
#include "Systems/Sound/SoundStandard.hpp"

// Gameplay Includes
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

#include "TileMapSourceLoadPattern.hpp"
#include "RaverieStatic.hpp"
