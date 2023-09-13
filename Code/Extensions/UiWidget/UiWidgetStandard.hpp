// MIT Licensed (see LICENSE.md).
#pragma once

// External dependencies
#include "Systems/Engine/EngineStandard.hpp"
#include "Systems/Graphics/GraphicsStandard.hpp"
#include "Extensions/Widget/WidgetStandard.hpp"
#include "Extensions/Gameplay/GameplayStandard.hpp"

namespace Zero
{

// UiWidget library
class UiWidgetLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(UiWidgetLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

// Widget Core
#include "UiWidget.hpp"
#include "UiRootWidget.hpp"
#include "UiWidgetEvents.hpp"

// Layouts
#include "UiLayout.hpp"
#include "UiStackLayout.hpp"
#include "UiFillLayout.hpp"
#include "UiDockLayout.hpp"
