///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Engine/EngineStandard.hpp"
#include "Graphics/GraphicsStandard.hpp"

namespace Zero
{
// Forward declarations
class CheckBox;
class ColorBlock;
class ComboBox;
class Command;
class Composite;
class ContextMenu;
class DataEvent;
class DefinitionSet;
class DispatchAtParams;
class DisplayRender;
class GraphicsEngine;
class Gripper;
class GripZones;
class IconButton;
class KeyboardEvent;
class KeyboardTextEvent;
class Label;
class Label;
class LayoutInfo;
class ListBox;
class ListSource;
class MenuBar;
class MouseEvent;
class MultiLineText;
class MultiManager;
class PropertyEvent;
class PropertyView;
class RootWidget;
class ScrollArea;
class Slider;
class Spacer;
class Splitter;
class TagLabel;
class Text;
class Text;
class TextBox;
class TextButton;
class TextCheckBox;
class TextureView;
class Tool;
class ToolTip;
class TreeView;
class Widget;
class Window;

// Widget library
class ZeroNoImportExport WidgetLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(WidgetLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

#include "WidgetMath.hpp"
#include "Layout.hpp"
#include "Events.hpp"
#include "Widget.hpp"

#include "Composite.hpp"
#include "MultiDock.hpp"
#include "RootWidget.hpp"
#include "MainWindow.hpp"
#include "Constants.hpp"
#include "Manipulator.hpp"
#include "ScrollArea.hpp"
#include "PopUp.hpp"
#include "Toolbar.hpp"
#include "ToolTip.hpp"
#include "Button.hpp"
#include "ContextMenu.hpp"
#include "Modal.hpp"
#include "ListControls.hpp"
#include "Utility.hpp"
#include "WidgetTest.hpp"
#include "TextureView.hpp"
#include "EditText.hpp"
#include "TextBox.hpp"
#include "SearchView.hpp"
#include "TagWidgets.hpp"
#include "Text.hpp"
#include "Slider.hpp"
#include "SelectorButton.hpp"
#include "ImageWidget.hpp"
#include "CheckBox.hpp"
#include "SimpleAnimation.hpp"
#include "LayoutProxy.hpp"
#include "WidgetManager.hpp"
#include "Command.hpp"
#include "CommandBinding.hpp"
#include "Controls.hpp"
#include "Window.hpp"
#include "MultiManager.hpp"
#include "Viewport.hpp"
#include "LabeledTextBox.hpp"
