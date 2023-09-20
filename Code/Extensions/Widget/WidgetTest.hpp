// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class WidgetTest : public ColorBlock
{
public:
  typedef WidgetTest RaverieSelf;
  WidgetTest(Composite* parent, StringParam name);
  void OnMouseEvent(MouseEvent* event);
  void OnKeyboardEvent(KeyboardEvent* event);
  void OnFocusEvent(FocusEvent* event);
  String Name;
};

class ParentWidgetTest : public Composite
{
public:
  typedef ParentWidgetTest RaverieSelf;
  ParentWidgetTest(Composite* parent, StringParam name);
  void OnFocusEvent(FocusEvent* event);
  void OnMouseEvent(MouseEvent* event);
  String Name;
};

void FlexMinSizeLayoutTest(Composite* testWindow);
void StandardControlsLayoutTest(Composite* testWindow);
void OpenTestWidgets(Composite* owner);

} // namespace Raverie
