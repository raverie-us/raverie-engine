///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiManager.hpp
///
/// Authors: Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class LayoutInfo
{
public:

  LayoutInfo()
  {
    Size = Vec2(0,0);
    Area = DockArea::Center;
    Visible = true;
  }

  // Name of the widget
  String Name;

  // Is this visible right now?
  bool Visible;

  HandleOf<Widget> ActiveWidget;

  // Size of the widget
  Vec2 Size;

  // Only valid if in a Multi dock
  DockArea::Enum Area;
};

class MultiManager : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  MultiManager(RootWidget* rootWidget, MultiDock* mainDock);

  void Closed(Widget* widget);
  void Destroyed(Widget* widget);

  // The primary window of the application
  RootWidget* mMainRoot;
  MultiDock* mMainDock;
  HashSet<Widget*> ManagedWidgets;
  HashMap<String, Widget*> InactiveWidgets;
  HashMap<String, LayoutInfo> Info;

  // Add a widget to be managed for tracking and searching
  Window* AddManagedWidget(Widget* widget, DockArea::Enum dockArea, bool isVisible);
  Window* AddManagedWidget(Widget* widget, LayoutInfo& info);

  // Show a widget forcing it to the front
  Widget* ShowWidget(StringParam name);
  Widget* ShowWidgetWith(HandleParam searchObject);

  Widget* ToggleWidget(StringParam name);

  // Find a managed widget
  Widget* FindWidget(StringParam name);
  Widget* FindWidgetWith(HandleParam searchObject);

  // For moving windows between OS windows
  void Transfer(TabWidget* tabWidget, Widget* widget);

//Internals
  void ManageWidget(Widget* widget);
  Widget* InternalActivateWidget(Widget* widget);

  void OnWindowKeyDown(KeyboardEvent* event);
};

}
