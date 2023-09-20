// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
DeclareEnum2(ToolBarMode, Dockable, Fixed);

class ToolBarGroupPopUp;
class CommandSubItem : public Composite
{
public:
  typedef CommandSubItem RaverieSelf;
  CommandSubItem(ToolBarGroupPopUp* parent, Command* command);

  float MeasureWidth();
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);
  void OnLeftClick(MouseEvent* e);

  Text* mText;
  Element* mIcon;
  Element* mBackground;
  Command* mCommand;
  ToolBarGroupPopUp* mPopUp;
};

class ToolBarGroup;
class ToolBarGroupPopUp : public PopUp
{
public:
  ToolBarGroupPopUp(Composite* parent, ToolBarGroup* group);

  Array<CommandSubItem*> mCommandWidgets;
};

class CommandSubItem;
class ToolBarGroup : public Composite
{
public:
  typedef ToolBarGroup RaverieSelf;
  ToolBarGroup(Composite* parent, StringParam name);

  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  void AddCommand(Command* command);
  void LoadMenu(StringParam menuName);

  void OnButtonPressed(Event* e);

  IconButton* mButton;
  Element* mExpandIcon;
  Array<Command*> mCommands;
  HandleOf<ToolBarGroupPopUp> mPopUp;
};

class ToolBarArea : public Composite
{
public:
  ToolBarArea(Composite* parent);
  void UpdateTransform() override;
};

class ToolBar : public Composite
{
public:
  typedef ToolBar RaverieSelf;

  ToolBar(Composite* parent);
  ~ToolBar();

  void DoLayout() override;
  void UpdateTransform() override;

  void SetMode(ToolBarMode::Type mode)
  {
    mToolBarMode = mode;
  }
  void OnMouseDown(MouseEvent* event);

  void AddCommand(Command* command, Command* secondaryCommand);
  ToolBarGroup* AddGroup(StringParam icon);
  void LoadMenu(StringParam menuName);

  void SetIconSize(Vec2 size);

private:
  Element* mBackground;
  Vec2 mIconSize;
  ToolBarMode::Type mToolBarMode;
};

} // namespace Raverie
