///////////////////////////////////////////////////////////////////////////////
///
/// \file WidgetTest.hpp
/// WidgetTests
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class WidgetTest : public ColorBlock
{
public:
  typedef WidgetTest ZilchSelf;
  WidgetTest(Composite* parent, StringParam name);
  void OnMouseEvent(MouseEvent* event);
  void OnKeyboardEvent(KeyboardEvent* event);
  void OnFocusEvent(FocusEvent* event);
  String Name;
};

class ParentWidgetTest : public Composite
{
public:
  typedef ParentWidgetTest ZilchSelf;
  ParentWidgetTest(Composite* parent, StringParam name);
  void OnFocusEvent(FocusEvent* event);
  void OnMouseEvent(MouseEvent* event);
  String Name;
};

void FlexMinSizeLayoutTest(Composite* testWindow);
void StandardControlsLayoutTest(Composite* testWindow);
void OpenSeperateWindow(OsWindow* mainWindow);
void OpenTestWidgets(Composite* owner);

}
