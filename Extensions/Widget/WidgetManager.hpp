///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class WidgetManager : public ExplicitSingleton<WidgetManager, EventObject>
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  WidgetManager();
  ~WidgetManager();

  u64 IdCounter;
  HashMap<u64, Widget*> Widgets;
  Array<Widget*> RootWidgets;
  Array<Widget*> DestroyList;
  ActionSpace* mWidgetActionSpace;

  void CleanUp();
  void OnEngineUpdate(UpdateEvent* event);
  void OnShutdown(Event* event);
};

namespace Z
{
  extern WidgetManager* gWidgetManager;
}

}
