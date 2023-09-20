// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(ContextMenuEntryChildren::range);

// Enums
RaverieDefineEnum(VerticalAlignment);
RaverieDefineEnum(HorizontalAlignment);
RaverieDefineEnum(IndicatorSide);
RaverieDefineEnum(ToolTipColorScheme);

RaverieDefineStaticLibrary(WidgetLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRangeAs(ContextMenuEntryChildren::range, "ContextMenuEntryChildrenRange");

  // Enums
  RaverieInitializeEnum(VerticalAlignment);
  RaverieInitializeEnum(HorizontalAlignment);
  RaverieInitializeEnum(IndicatorSide);
  RaverieInitializeEnum(ToolTipColorScheme);

  // Events
  RaverieInitializeType(FocusEvent);
  RaverieInitializeType(MouseEvent);
  RaverieInitializeType(CommandEvent);
  RaverieInitializeType(CommandUpdateEvent);
  RaverieInitializeType(CommandCaptureContextEvent);
  RaverieInitializeType(HighlightBorderEvent);
  RaverieInitializeType(TabModifiedEvent);
  RaverieInitializeType(TabRenamedEvent);
  RaverieInitializeType(QueryModifiedSaveEvent);
  RaverieInitializeType(HandleableEvent);
  RaverieInitializeType(WindowTabEvent);
  RaverieInitializeType(MainWindowTransformEvent);
  RaverieInitializeType(MouseDragEvent);
  RaverieInitializeType(ModalConfirmEvent);
  RaverieInitializeType(ModalButtonEvent);
  RaverieInitializeType(SearchViewEvent);
  RaverieInitializeType(AlternateSearchCompletedEvent);
  RaverieInitializeType(TagEvent);
  RaverieInitializeType(ContextMenuEvent);

  RaverieInitializeType(LayoutArea);
  RaverieInitializeType(Layout);
  RaverieInitializeType(FillLayout);
  RaverieInitializeType(StackLayout);
  RaverieInitializeType(EdgeDockLayout);
  RaverieInitializeType(DockLayout);
  RaverieInitializeType(RatioLayout);
  RaverieInitializeType(GridLayout);
  RaverieInitializeType(SizePolicies);
  RaverieInitializeType(Widget);
  RaverieInitializeType(Composite);
  RaverieInitializeType(MultiDock);
  RaverieInitializeType(RootWidget);
  RaverieInitializeType(MainWindow);
  RaverieInitializeType(MouseManipulation);
  RaverieInitializeType(BaseScrollArea);
  RaverieInitializeType(ScrollArea);
  RaverieInitializeType(ButtonBase);
  RaverieInitializeType(TextButton);
  RaverieInitializeType(IconButton);
  RaverieInitializeType(ToggleIconButton);
  RaverieInitializeType(ListBox);
  RaverieInitializeType(ComboBox);
  RaverieInitializeType(StringComboBox);
  RaverieInitializeType(ToolTip);
  RaverieInitializeType(TextureView);
  RaverieInitializeType(EditText);
  RaverieInitializeType(TextBox);
  RaverieInitializeType(SearchView);
  RaverieInitializeType(SearchViewElement);
  RaverieInitializeType(Label);
  RaverieInitializeType(ProgressBar);
  RaverieInitializeType(Slider);
  RaverieInitializeType(SelectorButton);
  RaverieInitializeType(ImageWidget);
  RaverieInitializeType(CheckBox);
  RaverieInitializeType(TextCheckBox);
  RaverieInitializeType(WidgetManager);
  RaverieInitializeType(CommandExecuter);
  RaverieInitializeType(CommandManager);
  RaverieInitializeType(MetaScriptTagAttribute);
  RaverieInitializeType(MetaScriptShortcutAttribute);
  RaverieInitializeType(Spacer);
  RaverieInitializeType(Splitter);
  RaverieInitializeType(TabArea);
  RaverieInitializeType(Window);
  RaverieInitializeType(Viewport);
  RaverieInitializeType(ContextMenuEntry);
  RaverieInitializeType(ContextMenuEntryDivider);
  RaverieInitializeType(ContextMenuEntryCommand);
  RaverieInitializeType(ContextMenuEntryMenu);
  RaverieInitializeType(MenuBarItem);
  RaverieInitializeType(MenuBar);
  RaverieInitializeType(MultiManager);
  RaverieInitializeType(Modal);
  RaverieInitializeType(Text);
  RaverieInitializeType(MultiLineText);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void WidgetLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  RegisterClassAttributeType(ObjectAttributes::cTags, MetaScriptTagAttribute)->TypeMustBe(Component);
  RegisterClassAttributeType(ObjectAttributes::cShortcut, MetaScriptShortcutAttribute)->TypeMustBe(Component);

  WidgetManager::Initialize();
  CommandManager::Initialize();
}

void WidgetLibrary::Shutdown()
{
  CommandManager::Destroy();
  WidgetManager::Destroy();

  GetLibrary()->ClearComponents();
}

} // namespace Raverie
