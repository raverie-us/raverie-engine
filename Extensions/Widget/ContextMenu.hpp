///////////////////////////////////////////////////////////////////////////////
///
/// \file ContextMenu.hpp
///
///
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ContextMenu;
class MenuBarItem;

namespace Events
{
  DeclareEvent(MenuItemSelected);
}

namespace MenuUi
{
  DeclareTweakable(Vec4, BackgroundColor);
  DeclareTweakable(Vec2, BorderPadding);
  DeclareTweakable(Vec4, GutterColor);
}

class ContextMenuDivider : public Composite
{
public:
  Element* mBackground;

  ContextMenuDivider(Composite* parent, Vec4 color)
    : Composite(parent)
  {
    mBackground = CreateAttached<Element>(cWhiteSquare);
    mBackground->SetColor(color);
  }

  Vec2 GetMinSize()
  {
    return Vec2(3,3);
  }

  void UpdateTransform() override
  {
    Vec2 lineSize = GetSize() - Vec2(2,2);
    float paddingWidth = Vec2(MenuUi::BorderPadding).x - Pixels(3);
    mBackground->SetSize(lineSize - Vec2(paddingWidth, 0));
    mBackground->SetTranslation(Pixels(1 + paddingWidth,1,0));
    Composite::UpdateTransform();
  }
};

///Item on a context Menu.
class ContextMenuItem : public Composite
{
public:
  typedef ContextMenuItem ZilchSelf;
  ContextMenuItem(ContextMenu* parent, StringParam menuName = String());

  void SetName(StringParam name, StringParam icon = String());
  void SetCommand(Command* command);
  void UpdateTransform() override;
  Vec2 GetMinSize() override;

  //Events
  void OnLeftMouseUp(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);

  //String Name;
  String ClientData;

  /// Whether or not the check is displayed.
  bool mActive;

  /// Whether or not the item is selectable.
  bool mEnabled;

private:
  Text* mText;
  Text* mShortcut;
  Element* mIcon;
  Element* mCheck;
  Element* mBackground;
  Element* mBorder;
  Command* mCommand;
};

///Content Menu PopUp
class ContextMenu : public PopUp
{
public:
  typedef ContextMenu ZilchSelf;
  ContextMenu(Widget* target);
  ~ContextMenu();

  void UpdateTransform() override;
  void SizeToContents() override;
  Vec2 GetMinSize() override;
  uint ItemCount();
  void LoadMenu(StringParam menuName);
  void CloseContextMenu();

  ContextMenuItem* AddCommand(Command* command);
  ContextMenuItem* AddCommandByName(StringParam commandName);
  ContextMenuItem* CreateContextItem(StringParam name);

  void AddDivider();

private:
  friend class ContextMenuItem;

  Array<ContextMenuItem*> mItems;
  Element* mBackground;
  Element* mGutter;
  Element* mBorder;
  String mMenuName;
};

class MenuBarItem : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef MenuBarItem ZilchSelf;
  MenuBarItem(Composite* widget);
  void UpdateTransform() override;
  Vec2 GetMinSize() override;
  void OnLeftMouseDown(MouseEvent* mouseEvent);
  void OnMouseEnter(MouseEvent* mouseEvent);
  void OpenContextMenu();

  MenuBar* GetMenuBar();
  void CloseContextMenu();
  void ClearOpenMenu(FocusEvent* event);
  
  HandleOf<MenuBar> mMenuBar;
  ContextMenu* mContextMenu;
  Text* mText;
  Element* mBackground;
  String mMenuName;
};

class MenuBar : public Composite
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef MenuBar ZilchSelf;
  MenuBar(Composite* widget);
  
  void LoadMenu(StringParam name);
  MenuBarItem* GetOpenMenuBarItem();

  HandleOf<MenuBarItem> mOpenMenuBarItem;
  Array<MenuBarItem*> Entry;
  String mMenuName;
};

#define ConnectMenu(menuName, optionName, function)                    \
  { ContextMenuItem* item = new ContextMenuItem(menuName, optionName); \
   ConnectThisTo(item, Zero::Events::MenuItemSelected, function); } 

}
