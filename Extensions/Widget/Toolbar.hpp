///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolBar.hpp
/// Declaration of the Toolbar classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
DeclareEnum2(ToolBarMode, Dockable, Fixed);

//------------------------------------------------------------- Command Sub Item
class ToolBarGroupPopUp;
class CommandSubItem : public Composite
{
public:
  typedef CommandSubItem ZilchSelf;
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

//--------------------------------------------------------- Tool Bar Group Popup
class ToolBarGroup;
class ToolBarGroupPopUp : public PopUp
{
public:
  ToolBarGroupPopUp(Composite* parent, ToolBarGroup* group);

  Array<CommandSubItem*> mCommandWidgets;
};

//--------------------------------------------------------------- Tool Bar Group
class CommandSubItem;
class ToolBarGroup : public Composite
{
public:
  typedef ToolBarGroup ZilchSelf;
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

//---------------------------------------------------------------- Tool Bar Area
class ToolBarArea : public Composite
{
public:
  ToolBarArea(Composite* parent);
  void UpdateTransform() override;
};

//--------------------------------------------------------------------- Tool Bar
class ToolBar : public Composite
{
public:
  typedef ToolBar ZilchSelf;

  ToolBar(Composite* parent);
  ~ToolBar();

  void DoLayout() override;
  void UpdateTransform() override;

  void SetMode(ToolBarMode::Type mode){mToolBarMode = mode;}
  void OnMouseDown(MouseEvent* event);

  void AddCommand(Command* command);
  ToolBarGroup* AddGroup(StringParam icon);
  void LoadMenu(StringParam menuName);

  void SetIconSize(Vec2 size);

private:
  Element* mBackground;
  Vec2 mIconSize;
  ToolBarMode::Type mToolBarMode;
};

}// namespace Zero
