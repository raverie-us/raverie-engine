// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
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
} // namespace Events

namespace MenuUi
{
DeclareTweakable(Vec4, BackgroundColor);
DeclareTweakable(Vec2, BorderPadding);
DeclareTweakable(Vec4, GutterColor);
} // namespace MenuUi

// Context
class Context
{
public:
  void Add(HandleParam object);
  void Add(HandleParam object, BoundType* overrideType);
  void Add(HandleParam object, StringParam name);
  void Add(const Context& context);

  void Remove(BoundType* boundType);
  void Remove(StringParam name);

  Handle Get(BoundType* boundType);
  Handle Get(StringParam typeName);

  template <typename ContextType>
  ContextType* Get();

  /// Clears all context.
  void Clear();

  typedef HashMap<String, Handle> ContextMap;
  ContextMap mContextMap;
};

typedef Context& ContextRef;
typedef const Context& ContextParam;

// Context Menu Event
class ContextMenuEvent : public Event
{
  RaverieDeclareType(ContextMenuEvent, TypeCopyMode::ReferenceType);

  ContextMenuEvent(ContextMenuEntry* rootEntry);
  ContextMenuEvent(ContextMenuEntry* rootEntry, Handle source);

  ContextMenuEntry* RootEntry;
  Handle Source;
};

typedef Array<ContextMenuEntry*> ContextMenuEntryChildren;

class ContextMenuEntry : public SafeId32EventObject
{
public:
  RaverieDeclareType(ContextMenuEntry, TypeCopyMode::ReferenceType);

  ContextMenuEntry(StringParam name = String(), StringParam icon = String(), bool readOnly = false);
  virtual ~ContextMenuEntry();

  // Context Menu Entry Interface (for use in raverie and C++)
  /// Adds the provided entry to the this entries children
  void AddEntry(ContextMenuEntry* entry);
  /// Adds a new entry with the provided name with an icon if one is provided to
  /// this menu entries children
  ContextMenuEntry* AddEntry(StringParam name = String(), bool readOnly = false);
  ContextMenuEntry* AddDivider();
  ContextMenuEntry* AddCommand(Command* command);
  ContextMenuEntry* AddCommandByName(StringParam commandName);

  /// Remove the entry with the provided name from this menu entries children
  void RemoveEntry(StringParam name);
  /// Remove the provided entry from this menu entries children
  void RemoveEntry(ContextMenuEntry* entry);
  /// Removes all child entries
  void Clear();

  /// Returns the children entry with the provided name if it exists and null
  /// otherwise
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

  ContextMenuEntry* mParent;
  ContextMenuEntryChildren mChildren;

  // Used to store any specific context information for use by selecting a menu
  // item
  Context mContext;

  // Used to disable a menu item from being selectable and greys out the item
  // text
  bool mEnabled;
  // If provided a tooltip can display the reason why the item is disabled
  String mDisabledText;
  /// Whether or not the item is read only (doesn't modify state).
  bool mReadOnly;
};

class ContextMenuEntryDivider : public ContextMenuEntry
{
public:
  RaverieDeclareType(ContextMenuEntryDivider, TypeCopyMode::ReferenceType);
  Widget* Create(ContextMenu* parent) override;
};

class ContextMenuEntryCommand : public ContextMenuEntry
{
public:
  RaverieDeclareType(ContextMenuEntryCommand, TypeCopyMode::ReferenceType);
  ContextMenuEntryCommand(Command* command);
  ContextMenuEntryCommand(StringParam commandName);

  Widget* Create(ContextMenu* parent) override;

  Command* mCommand;
  String mCommandName;
};

class ContextMenuEntryMenu : public ContextMenuEntry
{
public:
  RaverieDeclareType(ContextMenuEntryMenu, TypeCopyMode::ReferenceType);
  ContextMenuEntryMenu(StringParam menuName);

  Widget* Create(ContextMenu* parent) override;

  String mMenuName;
};

/// Item on a context Menu.
class ContextMenuItem : public Composite
{
public:
  typedef ContextMenuItem RaverieSelf;
  ContextMenuItem(Composite* parent, ContextMenuEntry* entry);

  void SetName(StringParam name, StringParam icon = String());
  void SetCommand(Command* command);
  virtual void UpdateTransform() override;
  Vec2 GetMinSize() override;

  // Events
  void OnLeftClick(MouseEvent* event);
  void OnMouseEnter(MouseEvent* event);
  void OnMouseExit(MouseEvent* event);
  void OnMouseHover(MouseEvent* event);
  void OnSiblingHover(ObjectEvent* event);

  // String Name;
  String ClientData;

  /// Whether or not the check is displayed.
  bool mActive;

  /// Whether or not the item is selectable.
  bool mEnabled;

  /// Whether or not the item is read only (doesn't modify state).
  bool mReadOnly;

  ContextMenu* GetParentMenu();

  // The menu entry that defined this menu item
  ContextMenuEntry* mEntry;

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
};

class ContextMenuDivider : public Composite
{
public:
  Element* mBackground;

  ContextMenuDivider(Composite* parent, Vec4 color) : Composite(parent)
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

/// Content Menu PopUp
class ContextMenu : public PopUp
{
public:
  typedef ContextMenu RaverieSelf;
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
  ContextMenuEntry* AddEntry(StringParam name, bool readOnly = false);
  void AddDivider();
  void AddContextMenu(StringParam menuName);
  void AddCommand(Command* command);
  void AddCommandByName(StringParam commandName);

  // Popup Interface
  void ShiftOntoScreen(Vec3 offset) override;
  void OnMouseDown(MouseEvent* event) override;
  void OnAnyGained(FocusEvent* event) override;
  void OnFocusOut(FocusEvent* event) override;

  bool IsPositionInHierarchy(Vec2Param screenPosition);
  bool IsFocusOnHierarchy(Widget* focusObject);

  ContextMenu* mParentMenu;
  ContextMenu* mSubMenu;
  Vec3 mSubMenuOffset;
  ContextMenuEntry* mRootEntry;
  // Flag for tracking when the context menu has been altered and needs to be
  // rebuilt
  bool mDirty;

  // For internal use only
  Array<Widget*> mItems;

private:
  friend class ContextMenuItem;

  ContextMenu* GetRootContextMenu();
  // These functions only check from the current menu an down through its sub
  // menus
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
  RaverieDeclareType(MenuBarItem, TypeCopyMode::ReferenceType);

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
  RaverieDeclareType(MenuBar, TypeCopyMode::ReferenceType);

  MenuBar(Composite* widget);

  void LoadMenu(StringParam name);
  MenuBarItem* GetOpenMenuBarItem();

  HandleOf<MenuBarItem> mOpenMenuBarItem;
  Array<MenuBarItem*> Entry;
  String mMenuName;
};

#define ConnectMenu(menu, optionName, function, readOnly)                                                                                                                                              \
  {                                                                                                                                                                                                    \
    ContextMenuEntry* entry = menu->AddEntry(String(optionName), readOnly);                                                                                                                            \
    ConnectThisTo(entry, Raverie::Events::MenuItemSelected, function);                                                                                                                                 \
  }

template <typename ContextType>
ContextType* Context::Get()
{
  BoundType* type = RaverieTypeId(ContextType);
  return Get(type).Get<ContextType*>();
}

} // namespace Raverie
