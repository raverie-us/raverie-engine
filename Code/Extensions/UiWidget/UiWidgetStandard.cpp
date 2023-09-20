// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(UiWidgetComponentHierarchy::ChildListRange);

// Enums
RaverieDefineEnum(UiSizePolicy);
RaverieDefineEnum(UiVerticalAlignment);
RaverieDefineEnum(UiHorizontalAlignment);
RaverieDefineEnum(UiDockMode);
RaverieDefineEnum(UiFocusDirection);
RaverieDefineEnum(UiStackLayoutDirection);

RaverieDefineStaticLibrary(UiWidgetLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRangeAs(UiWidgetComponentHierarchy::ChildListRange, "UiWidgetRange");

  // Enums
  RaverieInitializeEnum(UiSizePolicy);
  RaverieInitializeEnum(UiVerticalAlignment);
  RaverieInitializeEnum(UiHorizontalAlignment);
  RaverieInitializeEnum(UiDockMode);
  RaverieInitializeEnum(UiFocusDirection);
  RaverieInitializeEnum(UiStackLayoutDirection);

  // Events
  RaverieInitializeType(UiFocusEvent);
  RaverieInitializeType(UiTransformUpdateEvent);

  RaverieInitializeType(UiWidgetCastResultsRange);
  RaverieInitializeType(UiWidgetComponentHierarchy);
  RaverieInitializeType(UiWidget);
  RaverieInitializeType(UiRootWidget);
  RaverieInitializeType(UiLayout);
  RaverieInitializeType(UiStackLayout);
  RaverieInitializeType(UiFillLayout);
  RaverieInitializeType(UiDockLayout);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void UiWidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void UiWidgetLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Raverie
