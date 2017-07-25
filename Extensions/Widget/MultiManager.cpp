///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiManager.cpp
///
/// Authors: Chris Peters
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(MultiManager, builder, type)
{
}

MultiManager::MultiManager(RootWidget* rootWidget, MultiDock* mainDock)
{
  mMainRoot = rootWidget;
  mMainDock = mainDock;
  ConnectThisTo(rootWidget->GetOsWindow(), Events::OsKeyDown, OnWindowKeyDown);
}

void MultiManager::ManageWidget(Widget* widget)
{
  if(widget->mManager != this)
  {
    widget->mManager = this;
    ManagedWidgets.Insert(widget);
  }
}

Window* MultiManager::AddManagedWidget(Widget* widget, DockArea::Enum dockArea, bool visible)
{
  LayoutInfo info;
  info.Area = dockArea;
  info.Size = widget->GetSize();
  info.Name = widget->GetName();
  info.Visible = visible;
  info.ActiveWidget = widget;
  ErrorIf(widget->mName.Empty(), "Must have a name");
  return AddManagedWidget(widget, info);
}

Window* MultiManager::AddManagedWidget(Widget* widget, LayoutInfo& info)
{
  ManageWidget(widget);

  if(widget->mHideOnClose)
    Info[info.Name] = info;

  if(info.Visible)
  {
    return mMainDock->AddWidget(widget, info);
  }
  else
  {
    widget->SetActive(false);
    InactiveWidgets[info.Name] = widget;
    return nullptr;
  }
}

Widget* MultiManager::FindWidgetWith(HandleParam searchObject)
{
  WindowTabEvent event;
  event.SearchObject = searchObject;

  forRange(Widget* child, ManagedWidgets.All())
    child->DispatchBubble(Events::TabFind, &event);

  return event.TabWidgetFound;
}

Widget* MultiManager::FindWidget(StringParam name)
{
  // Brute force search
  WindowTabEvent event;
  event.Name = name;

  forRange(Widget* child, ManagedWidgets.All())
    child->DispatchBubble(Events::TabFind, &event);

  return event.TabWidgetFound;
}

Widget* MultiManager::InternalActivateWidget(Widget* widget)
{
  String name = widget->GetName();

  // Widget was in the non active 
  InactiveWidgets.Erase(name);

  LayoutInfo& location = Info[name];

  Window* window = mMainDock->AddWidget(widget, location);

  return widget;
}

Widget* MultiManager::ShowWidgetWith(HandleParam searchObject)
{
  Widget* widget = FindWidgetWith(searchObject);
  if(widget)
    Zero::ShowWidget(widget);
  return widget;
}

Widget* MultiManager::ShowWidget(StringParam name)
{
  Widget* widget = InactiveWidgets.FindValue(name, nullptr);
  if(widget)
  {
    InternalActivateWidget(widget);
  }
  else
  {
    widget = this->FindWidget(name);
    if(widget)
      Zero::ShowWidget(widget);
  }

  // Let the widget know its being shown
  if (widget)
  {
    Event toSend;
    widget->DispatchEvent(Events::WidgetShown, &toSend);
  }
  return widget;
}

Widget* MultiManager::ToggleWidget(StringParam name)
{
  if (Widget* widget = FindWidget(name))
    Zero::CloseTabContaining(widget);
  else
    return ShowWidget(name);
  return nullptr;
}

void MultiManager::Destroyed(Widget* widget)
{
  ManagedWidgets.Erase(widget);
}

void MultiManager::Closed(Widget* widget)
{
  // A widget on a tabbed window is being closed
  // return it to the hidden state if hide on close 
  // is set
  if(widget->mHideOnClose)
  {
    String name = widget->GetName();

    // Final clean up
    if(mMainDock->mDestroyed)
    {
      widget->Destroy();
      return;
    }

    LayoutInfo& info = Info[name];
    info.Size = widget->GetSize();

    Window* window = GetWindowContaining(widget);
    if(window && window->mDocker)
    {
      info.Area = window->mDocker->GetDockArea();
    }

    // Parent back to the main window
    mMainDock->AttachChildWidget(widget, AttachType::Normal);
    widget->SetActive(false);
    InactiveWidgets[name] = widget;

  }
  else
  {
    widget->Destroy();
  }
}

void MultiManager::Transfer(TabWidget* tabWidget, Widget* widget)
{
  // Is this widget on the main widget this is
  // true if they have the same root
  RootWidget* targetRoot = widget->GetRootWidget();
  RootWidget* mainRoot = mMainDock->GetRootWidget();

  if(targetRoot == mainRoot)
  {
    // Create a new Os window and attach the tab to it
    OsWindow* currentOsWindow = mainRoot->GetOsWindow();
    OsShell* shell = Z::gEngine->has(OsShell);
    Window* currentWindow = tabWidget->mTabArea->mParentWindow;

    // Open window on current location
    IntVec2 windowPos = ToIntVec2(ToVector2(currentWindow->GetScreenPosition()));
    windowPos += currentOsWindow->GetPosition();
    IntVec2 windowSize = Math::ToIntVec2( currentWindow->GetSize() );
    String name = tabWidget->mTitle->GetText();

    // Create a new top level window
    WindowStyleFlags::Enum windowFlags = (WindowStyleFlags::Enum)(WindowStyleFlags::Resizable | WindowStyleFlags::OnTaskBar | WindowStyleFlags::ClientOnly);
    OsWindow* newOsWindow = shell->CreateOsWindow(name, windowSize, windowPos, currentOsWindow, windowFlags);
    newOsWindow->SetMinSize(IntVec2(500, 500));
    MainWindow* rootWidget = new MainWindow(newOsWindow);
    rootWidget->SetTitle(name);

    ConnectThisTo(newOsWindow, Events::OsKeyDown, OnWindowKeyDown);

    // Attach a window to it
    Window* window = new Window(rootWidget);
    ManageWidget(window);
    newOsWindow->SetMinSize( ToIntVec2(currentWindow->GetMinSize()));
    window->mDocker = new OsDocker();
    window->AttachAsTab(widget);

    // Remove this tab
    tabWidget->SetOwnedWidget(nullptr);
    tabWidget->mTabArea->CloseTab(tabWidget);

  }
  else
  {
    // Move to the main widget
    AddManagedWidget(widget, DockArea::Floating, true);
    // Remove this tab
    tabWidget->SetOwnedWidget(nullptr);
    tabWidget->mTabArea->CloseTab(tabWidget);

    // Close the old window
    targetRoot->GetOsWindow()->Close();
  }

}

void MultiManager::OnWindowKeyDown(KeyboardEvent* event)
{
  // When a key is pressed on any window it re dispatched
  // by the multi manager for global shortcuts
  this->DispatchEvent(Events::OsKeyDown, event);
}

}
