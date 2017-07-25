///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Enums
ZilchDefineEnum(VerticalAlignment);
ZilchDefineEnum(HorizontalAlignment);

//**************************************************************************************************
ZilchDefineStaticLibrary(WidgetLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Enums
  ZilchInitializeEnum(VerticalAlignment);
  ZilchInitializeEnum(HorizontalAlignment);

  // Events
  ZilchInitializeType(FocusEvent);
  ZilchInitializeType(MouseEvent);
  ZilchInitializeType(CommandEvent);
  ZilchInitializeType(CommandCaptureContextEvent);
  ZilchInitializeType(HighlightBorderEvent);
  ZilchInitializeType(TabModifiedEvent);
  ZilchInitializeType(QueryModifiedSaveEvent);
  ZilchInitializeType(HandleableEvent);
  ZilchInitializeType(WindowTabEvent);
  ZilchInitializeType(MouseDragEvent);
  ZilchInitializeType(ModalConfirmEvent);
  ZilchInitializeType(ModalButtonEvent);
  ZilchInitializeType(SearchViewEvent);
  ZilchInitializeType(TagEvent);

  ZilchInitializeType(LayoutArea);
  ZilchInitializeType(Layout);
  ZilchInitializeType(FillLayout);
  ZilchInitializeType(StackLayout);
  ZilchInitializeType(EdgeDockLayout);
  ZilchInitializeType(DockLayout);
  ZilchInitializeType(RatioLayout);
  ZilchInitializeType(GridLayout);
  ZilchInitializeType(Thickness);
  ZilchInitializeType(Rect);
  ZilchInitializeType(SizePolicies);
  ZilchInitializeType(Widget);
  ZilchInitializeType(Composite);
  ZilchInitializeType(MultiDock);
  ZilchInitializeType(RootWidget);
  ZilchInitializeType(MainWindow);
  ZilchInitializeType(MouseManipulation);
  ZilchInitializeType(BaseScrollArea);
  ZilchInitializeType(ScrollArea);
  ZilchInitializeType(ButtonBase);
  ZilchInitializeType(TextButton);
  ZilchInitializeType(IconButton);
  ZilchInitializeType(ToggleIconButton);
  ZilchInitializeType(ListBox);
  ZilchInitializeType(ComboBox);
  ZilchInitializeType(StringComboBox);
  ZilchInitializeType(ToolTip);
  ZilchInitializeType(TextureView);
  ZilchInitializeType(EditText);
  ZilchInitializeType(TextBox);
  ZilchInitializeType(SearchView);
  ZilchInitializeType(SearchViewElement);
  ZilchInitializeType(Label);
  ZilchInitializeType(ProgressBar);
  ZilchInitializeType(Slider);
  ZilchInitializeType(SelectorButton);
  ZilchInitializeType(ImageWidget);
  ZilchInitializeType(CheckBox);
  ZilchInitializeType(TextCheckBox);
  ZilchInitializeType(WidgetManager);
  ZilchInitializeType(CommandExecuter);
  ZilchInitializeType(CommandManager);
  ZilchInitializeType(Spacer);
  ZilchInitializeType(Splitter);
  ZilchInitializeType(TabArea);
  ZilchInitializeType(Window);
  ZilchInitializeType(Viewport);
  ZilchInitializeType(MenuBarItem);
  ZilchInitializeType(MenuBar);
  ZilchInitializeType(MultiManager);
  ZilchInitializeType(Modal);
  ZilchInitializeType(Text);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void WidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  WidgetManager::Initialize();
  CommandManager::Initialize();
}

//**************************************************************************************************
void WidgetLibrary::Shutdown()
{
  CommandManager::Destroy();
  WidgetManager::Destroy();

  GetLibrary()->ClearComponents();
}

}//namespace Zero
