///////////////////////////////////////////////////////////////////////////////
///
/// \file ContextMenu.hpp
///
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
class ContextMenuEntry;
class ContextMenuItem;
class ContextMenu;
class MenuBarItem;

namespace Events
{
  DeclareEvent(MenuDestroy);
  DeclareEvent(MenuItemSelected);
  DeclareEvent(MenuItemHover);
  DeclareEvent(MouseHoverSibling);
  DeclareEvent(MenuEntryModified);
  DeclareEvent(ContextMenuCreated);
}

namespace MenuUi
{
  DeclareTweakable(Vec4, BackgroundColor);
  DeclareTweakable(Vec2, BorderPadding);
  DeclareTweakable(Vec4, GutterColor);
}

class ContextMenuEvent : public Event
{
  ZilchDeclareType(ContextMenuEvent, TypeCopyMode::ReferenceType);

  ContextMenuEvent(ContextMenuEntry* rootEntry);
  ContextMenuEvent(ContextMenuEntry* rootEntry, Handle source);

  ContextMenuEntry* RootEntry;
  Handle Source;
};

typedef Array<ContextMenuEntry*> ContextMenuEntryChildren;

class ContextMenuEntry : public SafeId32EventObject
{
public:
  ZilchDeclareType(ContextMenuEntry, TypeCopyMode::ReferenceType);

  ContextMenuEntry(StringParam name = String(), StringParam icon = String());
  virtual ~ContextMenuEntry();

  // Context Menu Entry Interface (for use in zilch and C++)
  /// Adds the provided entry to the this entries children
  void AddEntry(ContextMenuEntry* entry);
  /// Adds a new entry with the provided name with an icon if one is provided to this menu entries children
  ContextMenuEntry* AddEntry(StringParam name = String());
  ContextMenuEntry* AddDivider();
  ContextMenuEntry* AddCommand(Command* command);
  ContextMenuEntry* AddCommandByName(StringParam commandName);

  /// Remove the entry with the provided name from this menu entries children
  void RemoveEntry(StringParam name);
  /// Remove the provided entry from this menu entries children
  void RemoveEntry(ContextMenuEntry* entry);
  /// Removes all child entries
  void Clear();
  
  /// Returns the children entry with the provided name if it exists and null otherwise
  ContextMenuEntry* GetEntry(StringParam name);
  /// Returns a range with all this menu entries children
  ContextMenuEntryChildren::range GetEntries();
  /// Returns the number of entries on this entry
  size_t EntryCount();

  // Context Menu Interface (for use in C++ only)
  virtual Widget* Create(ContextMenu* parent);
  void CreateSubMenu(ContextMenuItem* menuItem);

  // Event Handlers
  void OnItemSelected(ObjectEvent* e);
  void OnItemHover(ObjectEvent* e);
  void OnChildMenuClose(ObjectEvent* e);

  void SetEnabled(bool state, String disabledText = String());

  String mName;
  String mIcon;
  ContextMenuEntryChildren mChildren;

  // Used to store any specific context information for use by selecting a menu item
  Any mContextData;
  
  // Used to disable a menu item from being selectable and greys out the item text
  bool mEnabled;
  // If provided a tooltip can display the reason why the item is disabled
  String mDisabledText;
};

class ContextMenuEntryDivider : public ContextMenuEntry
{
public:
  ZilchDeclareType(ContextMenuEntryDivider, TypeCopyMode::ReferenceType);
  Widget* Create(ContextMenu* parent) override;
};

class ContextMenuEntryCommand : public ContextMenuEntry
{
public:
  ZilchDeclareType(ContextMenuEntryCommand, TypeCopyMode::ReferenceType);
  ContextMenuEntryCommand(Command* command);
  ContextMenuEntryCommand(StringParam commandName);

  Widget* Create(ContextMenu* parent) override;

  Command* mCommand;
  String mCommandName;
};

class ContextMenuEntryMenu : public ContextMenuEntry
{
public:
  ZilchDeclareType(ContextMenuEntryMenu, TypeCopyMode::ReferenceType);
  ContextMenuEntryMenu(StringParam menuName);

  Widget* Create(ContextMenu* parent) override;

  String mMenuName;
};

///Item on a context Menu.
class ContextMenuItem : public Composite
{
public:
  typedef ContextMenuItem ZilchSelf;
  ContextMenuItem(Composite* parent, ContextMenuEntry* entry);

  void SetName(StringParam name, StringParam icon = String());
  void SetCommand(Command* command);
  virtual void UpdateTransform() override;
  Vec2 GetMinSize() override;

  //Events
  void OnLeftClick(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseHover(MouseEvent* event);
  void OnSiblingHover(ObjectEvent* event);

  //String Name;
  String ClientData;

  /// Whether or not the check is displayed.
  bool mActive;

  /// Whether or not the item is selectable.
  bool mEnabled;

  ContextMenu* GetParentMenu();

private:
  Text* mText;
  Text* mShortcut;
  Element* mIcon;
  Element* mCheck;
  Element* mBackground;
  Element* mBorder;
  Command* mCommand;
  HandleOf<ToolTip> mToolTip;

  HandleOf<ContextMenu> mParentMenu;
  ContextMenuEntry* mEntry;
};

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
    return Vec2(3, 3);
  }

  void UpdateTransform() override
  {
    Vec2 lineSize = GetSize() - Vec2(2, 2);
    float paddingWidth = Vec2(MenuUi::BorderPadding).x - Pixels(3);
    mBackground->SetSize(lineSize - Vec2(paddingWidth, 0));
    mBackground->SetTranslation(Pixels(1 + paddingWidth, 1, 0));
    Composite::UpdateTransform();
  }
};

///Content Menu PopUp
class ContextMenu : public PopUp
{
public:
  typedef ContextMenu ZilchSelf;
  ContextMenu(Widget* target, ContextMenuEntry* rootEntry = nullptr);
  ~ContextMenu() override;

  void UpdateTransform() override;
  void SizeToContents() override;
  Vec2 GetMinSize() override;
  void OnDestroy() override;
  void RebuildUi();

  uint IsEmpty();
  void CloseContextMenu();
  void FitSubMenuOnScreen(Vec3 position, Vec2 parentSize);

  void OnMenuEntriesModified(Event* event);

  // ContextMenuEntry builder functions
  ContextMenuEntry* GetRootEntry();
  ContextMenuEntry* AddEntry(StringParam name);
  void AddDivider();
  void AddZeroContextMenu(StringParam menuName);
  void AddCommand(Command* command);
  void AddCommandByName(StringParam commandName);

  // Popup Interface
  void OnMouseDown(MouseEvent* event) override;
  void OnAnyGained(FocusEvent* event) override;
  void OnFocusOut(FocusEvent* event)  override;
  
  bool IsPositionInHierarchy(Vec2Param screenPosition);
  bool IsFocusOnHierarchy(Widget* focusObject);
  
  ContextMenu* mParentMenu;
  ContextMenu* mSubMenu;
  Vec3 mSubMenuOffset;
  ContextMenuEntry* mRootEntry;
  // Flag for tracking when the context menu has been altered and needs to be rebuilt
  bool mDirty;

  // For internal use only
  Array<Widget*> mItems;
private:
  friend class ContextMenuItem;

  ContextMenu* GetRootContextMenu();
  // These functions only check from the current menu an down through its sub menus
  bool IsPositionInSubMenuRecursive(Vec2Param screenPosition);
  bool IsFocusInSubMenuRecursive(Widget* focusObject);

  Element* mBackground;
  Element* mGutter;
  Element* mBorder;
  String mMenuName;
};

class MenuBarItem : public Composite
{
public:
  ZilchDeclareType(MenuBarItem, TypeCopyMode::ReferenceType);

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
  ZilchDeclareType(MenuBar, TypeCopyMode::ReferenceType);

  MenuBar(Composite* widget);
  
  void LoadMenu(StringParam name);
  MenuBarItem* GetOpenMenuBarItem();

  HandleOf<MenuBarItem> mOpenMenuBarItem;
  Array<MenuBarItem*> Entry;
  String mMenuName;
};

#define ConnectMenu(menu, optionName, function)                    \
  { ContextMenuEntry* entry = menu->AddEntry(String(optionName)); \
   ConnectThisTo(entry, Zero::Events::MenuItemSelected, function); } 
}
