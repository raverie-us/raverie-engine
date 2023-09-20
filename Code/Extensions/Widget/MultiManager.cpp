// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(MultiManager, builder, type)
{
}

MultiManager::MultiManager(RootWidget* rootWidget, MultiDock* mainDock)
{
  mMainRoot = rootWidget;
  mMainDock = mainDock;
  ConnectThisTo(OsWindow::sInstance, Events::OsKeyDown, OnWindowKeyDown);
}

void MultiManager::ManageWidget(Widget* widget)
{
  if (widget->mManager != this)
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

  if (widget->mHideOnClose)
    Info[info.Name] = info;

  if (info.Visible)
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

  forRange (Widget* child, ManagedWidgets.All())
    child->DispatchBubble(Events::TabFind, &event);

  return event.TabWidgetFound;
}

Widget* MultiManager::FindWidget(StringParam name)
{
  // Brute force search
  WindowTabEvent event;
  event.Name = name;

  forRange (Widget* child, ManagedWidgets.All())
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
  if (widget)
    Raverie::ShowWidget(widget);
  return widget;
}

Widget* MultiManager::ShowWidget(StringParam name)
{
  Widget* widget = InactiveWidgets.FindValue(name, nullptr);
  if (widget)
  {
    InternalActivateWidget(widget);
  }
  else
  {
    widget = this->FindWidget(name);
    if (widget)
      Raverie::ShowWidget(widget);
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
    Raverie::CloseTabContaining(widget);
  else
    return ShowWidget(name);
  return nullptr;
}

Widget* MultiManager::HideWidget(StringParam name)
{
  if (Widget* widget = FindWidget(name))
    Raverie::CloseTabContaining(widget);
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
  if (widget->mHideOnClose)
  {
    String name = widget->GetName();

    // Final clean up
    if (mMainDock->mDestroyed)
    {
      widget->Destroy();
      return;
    }

    LayoutInfo& info = Info[name];
    Window* window = GetWindowContaining(widget);

    if (window && window->mDocker)
    {
      info.Size = window->GetSize();
      info.Area = window->mDocker->GetDockArea();
    }
    else
    {
      info.Size = widget->GetSize();
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

void MultiManager::OnWindowKeyDown(KeyboardEvent* event)
{
  // When a key is pressed on any window it re dispatched
  // by the multi manager for global shortcuts
  this->DispatchEvent(Events::OsKeyDown, event);
}

} // namespace Raverie
