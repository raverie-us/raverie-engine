// MIT Licensed (see LICENSE.md).
#pragma once

// External dependencies
#include "Systems/Engine/EngineStandard.hpp"
#include "Systems/Graphics/GraphicsStandard.hpp"
#include "Extensions/Widget/WidgetStandard.hpp"
#include "Extensions/Gameplay/GameplayStandard.hpp"

namespace Raverie
{

// UiWidget library
class UiWidgetLibrary : public Raverie::StaticLibrary
{
public:
  RaverieDeclareStaticLibraryInternals(UiWidgetLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Raverie

// Widget Core
#include "UiWidget.hpp"
#include "UiRootWidget.hpp"
#include "UiWidgetEvents.hpp"

// Layouts
#include "UiLayout.hpp"
#include "UiStackLayout.hpp"
#include "UiFillLayout.hpp"
#include "UiDockLayout.hpp"
