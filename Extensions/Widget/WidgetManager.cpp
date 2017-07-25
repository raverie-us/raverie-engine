///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(WidgetManager, builder, type)
{
}

WidgetManager::WidgetManager()
{
  Z::gWidgetManager = this;
  IdCounter = 0;
  mWidgetActionSpace = new ActionSpace();
  ConnectThisTo(Z::gEngine, Events::EngineUpdate, OnEngineUpdate);
  ConnectThisTo(Z::gEngine, Events::EngineShutdown, OnShutdown);
}

WidgetManager::~WidgetManager()
{
  SafeDelete(mWidgetActionSpace);
}

void WidgetManager::OnEngineUpdate(UpdateEvent* event)
{
  mWidgetActionSpace->UpdateActions(event, ActionExecuteMode::FrameUpdate);
  mWidgetActionSpace->UpdateActions(event, ActionExecuteMode::LogicUpdate);
  DispatchEvent(Events::WidgetUpdate, event);
  CleanUp();
}

void WidgetManager::OnShutdown(Event* event)
{
  forRange(Widget* widget, Widgets.Values())
  {
    if(widget->mParent == NULL)
      widget->Destroy();
  }
  CleanUp();
}

void WidgetManager::CleanUp()
{
  while(!DestroyList.Empty())
  {
    // Use temporary list so that widgets
    // that delete widget in their destructor
    // will not create problems
    Array<Widget*> templist;
    templist.Swap(DestroyList);

    forRange(Widget* widget, templist.All())
    {
      delete widget;
    }

    templist.Clear();

    // Put it back not
    if(DestroyList.Empty())
      templist.Swap(DestroyList);
  }
}

namespace Z
{
WidgetManager* gWidgetManager = nullptr;
}

}
