// MIT Licensed (see LICENSE.md).
#pragma once

// External dependencies
#include "Engine/EngineStandard.hpp"
#include "Graphics/GraphicsStandard.hpp"
#include "Widget/WidgetStandard.hpp"
#include "Gameplay/GameplayStandard.hpp"

namespace Zero
{

// UiWidget library
class ZeroNoImportExport UiWidgetLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(UiWidgetLibrary, "ZeroEngine");

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
