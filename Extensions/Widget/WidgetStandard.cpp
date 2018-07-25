///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(ContextMenuEntryChildren::range);

// Enums
ZilchDefineEnum(VerticalAlignment);
ZilchDefineEnum(HorizontalAlignment);
ZilchDefineEnum(IndicatorSide);
ZilchDefineEnum(ToolTipColorScheme);

//**************************************************************************************************
ZilchDefineStaticLibrary(WidgetLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  // Ranges
  ZilchInitializeRangeAs(ContextMenuEntryChildren::range, "ContextMenuEntryChildrenRange");

  // Enums
  ZilchInitializeEnum(VerticalAlignment);
  ZilchInitializeEnum(HorizontalAlignment);
  ZilchInitializeEnum(IndicatorSide);
  ZilchInitializeEnum(ToolTipColorScheme);

  // Events
  ZilchInitializeType(FocusEvent);
  ZilchInitializeType(MouseEvent);
  ZilchInitializeType(CommandEvent);
  ZilchInitializeType(CommandUpdateEvent);
  ZilchInitializeType(CommandCaptureContextEvent);
  ZilchInitializeType(HighlightBorderEvent);
  ZilchInitializeType(TabModifiedEvent);
  ZilchInitializeType(TabRenamedEvent);
  ZilchInitializeType(QueryModifiedSaveEvent);
  ZilchInitializeType(HandleableEvent);
  ZilchInitializeType(WindowTabEvent);
  ZilchInitializeType(MainWindowTransformEvent);
  ZilchInitializeType(MouseDragEvent);
  ZilchInitializeType(ModalConfirmEvent);
  ZilchInitializeType(ModalButtonEvent);
  ZilchInitializeType(SearchViewEvent);
  ZilchInitializeType(TagEvent);
  ZilchInitializeType(ContextMenuEvent);

  ZilchInitializeType(LayoutArea);
  ZilchInitializeType(Layout);
  ZilchInitializeType(FillLayout);
  ZilchInitializeType(StackLayout);
  ZilchInitializeType(EdgeDockLayout);
  ZilchInitializeType(DockLayout);
  ZilchInitializeType(RatioLayout);
  ZilchInitializeType(GridLayout);
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
  ZilchInitializeType(ContextMenuEntry);
  ZilchInitializeType(ContextMenuEntryDivider);
  ZilchInitializeType(ContextMenuEntryCommand);
  ZilchInitializeType(ContextMenuEntryMenu);
  ZilchInitializeType(MenuBarItem);
  ZilchInitializeType(MenuBar);
  ZilchInitializeType(MultiManager);
  ZilchInitializeType(Modal);
  ZilchInitializeType(Text);
  ZilchInitializeType(MultiLineText);

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
