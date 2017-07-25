///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Standard Library Dependencies
#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Geometry/GeometryStandard.hpp"
#include "Meta/MetaStandard.hpp"
#include "Support/SupportStandard.hpp"

// Zilch Library Dependencies
#include "Zilch/Zilch.hpp"
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
}

// Core Library Dependencies
#include "Engine/EngineStandard.hpp"
#include "Physics/PhysicsStandard.hpp"
#include "Graphics/GraphicsStandard.hpp"
#include "Widget/WidgetStandard.hpp"

// Gameplay Includes
#include "Orientation.hpp"

#include "Reactive.hpp"
#include "ReactiveViewport.hpp"
#include "MouseCapture.hpp"
#include "CameraViewport.hpp"

#include "DefaultGame.hpp"
#include "PlayGame.hpp"

#include "MarchingSquares.hpp"
#include "RandomContext.hpp"

#include "MarchingCubes.hpp"

#include "TileMap.hpp"
#include "TileMapSource.hpp"
#include "SplineParticles.hpp"

#include "WebBrowser.hpp"
#include "WebBrowserWidget.hpp"
#include "MarketWidget.hpp"
#include "TileMapSourceLoadPattern.hpp"
#include "Zero.hpp"
