///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(UiSizePolicy);
ZilchDefineEnum(UiVerticalAlignment);
ZilchDefineEnum(UiHorizontalAlignment);
ZilchDefineEnum(UiDockMode);
ZilchDefineEnum(UiFocusDirection);
ZilchDefineEnum(UiStackLayoutDirection);

//**************************************************************************************************
ZilchDefineStaticLibrary(UiWidgetLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  // Enums
  ZilchInitializeEnum(UiSizePolicy);
  ZilchInitializeEnum(UiVerticalAlignment);
  ZilchInitializeEnum(UiHorizontalAlignment);
  ZilchInitializeEnum(UiDockMode);
  ZilchInitializeEnum(UiFocusDirection);
  ZilchInitializeEnum(UiStackLayoutDirection);

  // Events
  ZilchInitializeType(UiFocusEvent);
  ZilchInitializeType(UiTransformUpdateEvent);

  ZilchInitializeType(UiWidgetCastResultsRange);
  ZilchInitializeType(UiWidget);
  ZilchInitializeType(UiRootWidget);
  ZilchInitializeType(UiRenderer);
  ZilchInitializeType(UiRootInputRouter);
  ZilchInitializeType(UiLayout);
  ZilchInitializeType(UiStackLayout);
  ZilchInitializeType(UiFillLayout);
  ZilchInitializeType(UiDockLayout);

  ZilchInitializeType(UiWidget);
  ZilchInitializeType(UiRootWidget);
  ZilchInitializeType(UiRootInputRouter);
  ZilchInitializeType(UiRenderer);
  ZilchInitializeType(UiLayout);
  ZilchInitializeType(UiStackLayout);
  ZilchInitializeType(UiFillLayout);
  ZilchInitializeType(UiDockLayout);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void UiWidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void UiWidgetLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}//namespace Zero
