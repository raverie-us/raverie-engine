// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace MenuUi
{
const cstr cLocation = "EditorUi/Controls/Menu";
Tweakable(Vec4, BackgroundColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec2, BorderPadding, Vec2(1, 1), cLocation);

Tweakable(Vec4, ItemTextColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ItemBackgroundColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ItemBorderColor, Vec4(1, 1, 1, 1), cLocation);

Tweakable(Vec4, ItemSelectedTextColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ItemSelectedBorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, ItemSelectedBackgroundColor, Vec4(1, 1, 1, 1), cLocation);

Tweakable(Vec4, ItemDisabledTextColor, Vec4(1, 1, 1, 1), cLocation);

Tweakable(Vec4, MenuBarItemPadding, Vec4(4, 4, 4, 4), cLocation);
Tweakable(Vec4, GutterColor, Vec4(1, 1, 1, 1), cLocation);
} // namespace MenuUi

namespace Events
{
DefineEvent(MenuDestroy);
DefineEvent(MenuItemSelected);
DefineEvent(MenuItemHover);
DefineEvent(MouseHoverSibling);
DefineEvent(MenuEntryModified);
DefineEvent(ContextMenuCreated);
} // namespace Events

// Context
void Context::Add(HandleParam object)
{
  mContextMap[object.StoredType->Name] = object;
}

void Context::Add(HandleParam object, BoundType* overrideType)
{
  Add(object, overrideType->Name);
}

void Context::Add(HandleParam object, StringParam overrideName)
{
  mContextMap[overrideName] = object;
}

void Context::Add(const Context& context)
{
  forRange (ContextMap::value_type entry, context.mContextMap.All())
    Add(entry.second, entry.first);
}

void Context::Remove(BoundType* boundType)
{
  Remove(boundType->Name);
}

void Context::Remove(StringParam typeName)
{
  mContextMap.Erase(typeName);
}

Handle Context::Get(BoundType* boundType)
{
  return Get(boundType->Name);
}

Handle Context::Get(StringParam typeName)
{
  return mContextMap.FindValue(typeName, Handle());
}

void Context::Clear()
{
  mContextMap.Clear();
}

RaverieDefineType(ContextMenuEvent, builder, type)
{
}

ContextMenuEvent::ContextMenuEvent(ContextMenuEntry* rootEntry)
{
  RootEntry = rootEntry;
}

ContextMenuEvent::ContextMenuEvent(ContextMenuEntry* rootEntry, Handle source)
{
  RootEntry = rootEntry;
  Source = source;
}

RaverieDefineType(ContextMenuEntry, builder, type)
{
  RaverieBindDocumented();
  RaverieBindConstructor();

  RaverieBindOverloadedMethodAs(AddEntry, RaverieInstanceOverload(ContextMenuEntry*, StringParam, bool), "AddEntry");
  RaverieBindMethod(AddDivider);
  RaverieBindMethod(AddCommandByName);
  RaverieBindOverloadedMethodAs(RemoveEntry, RaverieInstanceOverload(void, StringParam), "RemoveEntry");

  RaverieBindMethod(GetEntry);
  RaverieBindMethodAs(GetEntries, "Entries");

  RaverieBindFieldProperty(mName);
  RaverieBindFieldProperty(mIcon);

  RaverieBindEvent(Events::ContextMenuCreated, ContextMenuEvent);
}

ContextMenuEntry::ContextMenuEntry(StringParam name, StringParam icon, bool readOnly) :
    mName(name),
    mIcon(icon),
    mEnabled(true),
    mParent(nullptr),
    mReadOnly(readOnly)
{
  ConnectThisTo(this, Events::MenuItemSelected, OnItemSelected);
  ConnectThisTo(this, Events::MenuItemHover, OnItemHover);
}

ContextMenuEntry::~ContextMenuEntry()
{
  Clear();
}

void ContextMenuEntry::OnItemSelected(ObjectEvent* e)
{
  ContextMenuItem* menuItem = (ContextMenuItem*)e->Source;

  if (!mChildren.Empty())
    CreateSubMenu(menuItem);
}

void ContextMenuEntry::OnItemHover(ObjectEvent* e)
{
  ContextMenuItem* menuItem = (ContextMenuItem*)e->Source;

  if (!mChildren.Empty())
    CreateSubMenu(menuItem);
}

void ContextMenuEntry::OnChildMenuClose(ObjectEvent* e)
{
  // A sub menu has closed so re-enable closing the parent menu based on mouse
  // distance
  ContextMenu* menuClosed = (ContextMenu*)e->Source;
  // If the menu being closed has any children menu close them also
  if (menuClosed->mSubMenu)
    menuClosed->mSubMenu->FadeOut();

  menuClosed->mParentMenu->mCloseMode = PopUpCloseMode::MouseDistance;
  menuClosed->mParentMenu->mSubMenu = nullptr;
}

void ContextMenuEntry::SetEnabled(bool state, String disabledText)
{
  mEnabled = state;
  mDisabledText = disabledText;
}

void ContextMenuEntry::AddEntry(ContextMenuEntry* entry)
{
  entry->mParent = this;
  mChildren.PushBack(entry);
  Event event;
  this->DispatchEvent(Events::MenuEntryModified, &event);
}

ContextMenuEntry* ContextMenuEntry::AddEntry(StringParam name, bool readOnly)
{
  ContextMenuEntry* entry = new ContextMenuEntry(name, String(), readOnly);
  AddEntry(entry);
  return entry;
}

ContextMenuEntry* ContextMenuEntry::AddDivider()
{
  ContextMenuEntry* entry = new ContextMenuEntryDivider();
  AddEntry(entry);
  return entry;
}

ContextMenuEntry* ContextMenuEntry::AddCommand(Command* command)
{
  ContextMenuEntry* entry = new ContextMenuEntryCommand(command);
  AddEntry(entry);
  return entry;
}

ContextMenuEntry* ContextMenuEntry::AddCommandByName(StringParam commandName)
{
  ContextMenuEntry* entry = new ContextMenuEntryCommand(commandName);
  AddEntry(entry);
  return entry;
}

void ContextMenuEntry::RemoveEntry(StringParam name)
{
  size_t numEntries = mChildren.Size();
  for (size_t i = 0; i < numEntries; ++i)
  {
    if (mChildren[i]->mName == name)
    {
      delete mChildren[i];
      mChildren.EraseAt(i);
      break;
    }
  }
}

void ContextMenuEntry::RemoveEntry(ContextMenuEntry* entry)
{
  if (mChildren.EraseValue(entry))
    delete entry;
}

void ContextMenuEntry::Clear()
{
  size_t numEntries = mChildren.Size();
  for (size_t i = 0; i < numEntries; ++i)
    delete mChildren[i];

  mChildren.Clear();
}

ContextMenuEntry* ContextMenuEntry::GetEntry(StringParam name)
{
  forRange (ContextMenuEntry* entry, mChildren)
  {
    if (entry->mName == name)
      return entry;
  }
  return nullptr;
}

Array<ContextMenuEntry*>::range ContextMenuEntry::GetEntries()
{
  return mChildren.All();
}

size_t ContextMenuEntry::EntryCount()
{
  return mChildren.Size();
}

Widget* ContextMenuEntry::Create(ContextMenu* parent)
{
  ContextMenuItem* item = new ContextMenuItem(parent, this);
  parent->mItems.PushBack(item);
  item->SetName(mName);
  return item;
}

void ContextMenuEntry::CreateSubMenu(ContextMenuItem* menuItem)
{
  if (!menuItem)
    return;

  ContextMenu* parent = (ContextMenu*)menuItem->GetParentMenu();
  if (parent->mSubMenu)
  {
    // If there is already a sub menu on the context menu check to see if this
    // entry is the root entry and don't return as this menu is already open
    if (parent->mSubMenu->mRootEntry == this)
      return;

    // Otherwise close the current sub menu
    parent->mSubMenu->FadeOut();
  }

  ContextMenu* menu = new ContextMenu(parent, this);
  // When creating a sub menu get the local offset of the item that spawned it
  menu->mSubMenuOffset = menuItem->GetTranslation();
  ConnectThisTo(menu, Events::PopUpClosed, OnChildMenuClose);
  // Set the new sub menu and parent menu as each others parent and child menu
  // respectively
  parent->mSubMenu = menu;
  menu->mParentMenu = parent;
  menu->mDirty = true;

  // When opening a sub menu disable the parent from closing based on mouse
  // distance so the sub menu doesn't close when we stray too far from the
  // parent menu
  parent->mCloseMode = PopUpCloseMode::DisableClose;

  // After creating the sub menu update the parent immediately to properly
  // position and display the new sub menu
  parent->UpdateTransform();
}

// ContextMenuEntryDivider
RaverieDefineType(ContextMenuEntryDivider, builder, type)
{
  RaverieBindConstructor();
}

Widget* ContextMenuEntryDivider::Create(ContextMenu* parent)
{
  Widget* item = new ContextMenuDivider(parent, MenuUi::GutterColor);
  parent->mItems.PushBack(item);
  return item;
}

// ContextMenuEntryCommand
RaverieDefineType(ContextMenuEntryCommand, builder, type)
{
  RaverieFullBindConstructor(builder, type, ContextMenuEntryCommand, "commandName", StringParam);
  RaverieBindFieldProperty(mCommandName);
}

ContextMenuEntryCommand::ContextMenuEntryCommand(Command* command) : mCommand(command)
{
}

ContextMenuEntryCommand::ContextMenuEntryCommand(StringParam commandName) : mCommand(nullptr), mCommandName(commandName)
{
}

Widget* ContextMenuEntryCommand::Create(ContextMenu* parent)
{
  Command* command;
  if (mCommand)
  {
    command = mCommand;
  }
  else
  {
    CommandManager* commandManager = CommandManager::GetInstance();
    command = commandManager->GetCommand(mCommandName);
  }

  ContextMenuItem* item = new ContextMenuItem(parent, this);
  parent->mItems.PushBack(item);
  item->SetCommand(command);
  return item;
}

// ContextMenuEntryMenu
RaverieDefineType(ContextMenuEntryMenu, builder, type)
{
  RaverieFullBindConstructor(builder, type, ContextMenuEntryMenu, "menuName", StringParam);
  RaverieBindFieldProperty(mMenuName);
}

ContextMenuEntryMenu::ContextMenuEntryMenu(StringParam menuName) : mMenuName(menuName)
{
}

Widget* ContextMenuEntryMenu::Create(ContextMenu* parent)
{
  DeveloperConfig* devConfig = Z::gEngine->GetConfigCog()->has(DeveloperConfig);
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(mMenuName, nullptr);
  ReturnIf(menuDef == nullptr, nullptr, "Could not find menu definition '%s'", mMenuName.c_str());

  forRange (String& name, menuDef->Entries.All())
  {
    // Divider
    if (name == Divider)
    {
      // create the divider UI
      Widget* item = new ContextMenuDivider(parent, MenuUi::GutterColor);
      parent->mItems.PushBack(item);
      continue;
    }

    // Command
    Command* command = commandManager->GetCommand(name);
    if (command)
    {
      // If it's a dev only command and there's no dev config, don't add it
      if (command->DevOnly && devConfig == nullptr)
        continue;

      ContextMenuItem* item = new ContextMenuItem(parent, this);
      parent->mItems.PushBack(item);
      item->SetCommand(command);
      continue;
    }

    // Sub Menu
    MenuDefinition* menuDef = commandManager->mMenus.FindValue(name, nullptr);
    if (menuDef != nullptr)
    {
      ErrorIf(true, "Todo sub menus");
      continue;
    }

    ErrorIf(true, "Invalid menu entry '%s'", name.c_str());
  }

  return nullptr;
}

ContextMenuItem::ContextMenuItem(Composite* parent, ContextMenuEntry* entry) : Composite(parent), mEntry(entry)
{
  mName = entry->mName;
  mParentMenu = (ContextMenu*)parent;
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquare);

  Thickness thickness(MenuUi::BorderPadding);
  thickness.Right = Pixels(2);
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2(0, 0), thickness));

  mCheck = CreateAttached<Element>("CheckIcon");

  mText = new Text(this, cText);
  mText->SetSizing(SizeAxis::X, SizePolicy::Flex, 20);
  mText->SetText(entry->mName);
  mText->SizeToContents();

  Spacer* spacer = new Spacer(this);
  spacer->SetSize(Vec2(20, 10));

  mShortcut = new Text(this, cText);
  mIcon = nullptr;

  mCommand = nullptr;
  mEnabled = mEntry->mEnabled;
  mReadOnly = mEntry->mReadOnly;
  mActive = false;

  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::MouseHover, OnMouseHover);
  ConnectThisTo(this, Events::MouseHoverSibling, OnSiblingHover);

  parent->SizeToContents();
}

void ContextMenuItem::OnMouseEnter(MouseEvent* event)
{
  mEntry->DispatchEvent(Events::MouseEnter, event);
  MarkAsNeedsUpdate();
}

void ContextMenuItem::OnMouseExit(MouseEvent* event)
{
  mEntry->DispatchEvent(Events::MouseExit, event);
  MarkAsNeedsUpdate();
}

void ContextMenuItem::OnMouseHover(MouseEvent* event)
{
  ObjectEvent objectEvent(this);
  mEntry->DispatchEvent(Events::MenuItemHover, &objectEvent);
}

void ContextMenuItem::OnSiblingHover(ObjectEvent* event)
{
  ContextMenuItem* item = (ContextMenuItem*)event->Source;
  // Don't do anything if we were the item selected
  if (item == this)
    return;
}

ContextMenu* ContextMenuItem::GetParentMenu()
{
  if (mParentMenu.IsNotNull())
    return (ContextMenu*)mParentMenu;

  return nullptr;
}

Vec2 ContextMenuItem::GetMinSize()
{
  return Composite::GetMinSize();
}

void ContextMenuItem::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  mCheck->SetVisible(mActive);
  Vec3 checkPos(Pixels(1), mSize.y * 0.5f - mCheck->GetSize().y * 0.5f, 0);
  mCheck->SetTranslation(checkPos);

  mBackground->SetColor(MenuUi::ItemBackgroundColor);
  mBorder->SetColor(MenuUi::ItemBackgroundColor);

  if (!mEnabled)
  {
    mText->SetColor(MenuUi::ItemDisabledTextColor);
    mShortcut->SetColor(MenuUi::ItemDisabledTextColor);

    // If a reason was provided for why this item was disabled create a tooltip
    // displaying it
    if (mToolTip.IsNull() && !mEntry->mDisabledText.Empty())
    {
      ToolTip* toolTip = new ToolTip(this);
      toolTip->SetTextAndPlace(mEntry->mDisabledText, GetScreenRect());
    }
  }
  else if (IsMouseOver())
  {
    mBackground->SetColor(MenuUi::ItemSelectedBackgroundColor);
    mBorder->SetColor(MenuUi::ItemSelectedBackgroundColor);
  }

  if (mIcon)
  {
    Vec3 rightSide = Vec3(mSize.x, 0, 0);
    rightSide.x -= mIcon->GetSize().x;
    mIcon->SetTranslation(rightSide + Pixels(0, 2, 0));
  }

  if (!mEntry->mChildren.Empty() && !mIcon)
  {
    mIcon = CreateAttached<Element>("PropArrowRight");
  }
  else if (mEntry->mChildren.Empty() && mIcon)
  {
    mIcon->Destroy();
    mIcon = nullptr;
  }

  Composite::UpdateTransform();
}

void ContextMenuItem::SetName(StringParam name, StringParam icon)
{
  mName = name;
  mText->SetText(name);
  if (!icon.Empty())
    mIcon = CreateAttached<Element>(icon);
  MarkAsNeedsUpdate();
}

void ContextMenuItem::SetCommand(Command* command)
{
  mReadOnly = command->ReadOnly;
  SetName(command->GetDisplayName());
  mShortcut->SetText(command->Shortcut);
  mCommand = command;
  mEnabled = command->IsEnabled();
  mActive = command->IsActive();
}

void ContextMenuItem::OnLeftClick(MouseEvent* event)
{
  if (Z::gEngine->IsReadOnly() && !mReadOnly)
  {
    DoNotifyWarning("Context Menu",
                    BuildString("Cannot execute menu item ", mName, " because we are in read-only mode"));
    return;
  }

  if (!mEnabled)
    return;

  ObjectEvent eventToSend(this);
  mEntry->DispatchEvent(Events::MenuItemSelected, &eventToSend);

  if (mCommand)
  {
    Context& commandContext = CommandManager::GetInstance()->mContext;

    Context recover = commandContext;

    // Add all menu contexts
    ContextMenuEntry* current = mEntry;
    while (current)
    {
      commandContext.Add(current->mContext);
      current = current->mParent;
    }

    mCommand->ExecuteCommand();

    commandContext = recover;
  }

  // If this item isn't a sub context menu close the context menu
  if (mEntry->mChildren.Empty())
    this->GetParent()->Destroy();
}

ContextMenu::ContextMenu(Widget* target, ContextMenuEntry* rootEntry) :
    PopUp(target, PopUpCloseMode::MouseDistance, cPopUpNormal),
    mParentMenu(nullptr),
    mSubMenu(nullptr),
    mSubMenuOffset(Vec3::cZero)
{
  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mGutter = CreateAttached<Element>(cWhiteSquare);
  if (rootEntry == nullptr)
    mRootEntry = new ContextMenuEntry("RootEntry");
  else
    mRootEntry = rootEntry;

  mDirty = false;

  ConnectThisTo(mRootEntry, Events::MenuEntryModified, OnMenuEntriesModified);
  Thickness thickness(Pixels(2, 2));
  SetLayout(CreateStackLayout(LayoutDirection::TopToBottom, Vec2(0, 0), thickness));
  SizeToContents();
}

ContextMenu::~ContextMenu()
{
  // when the context menu loses focus and deletes itself we need
  // to clear the currently open menu references so returning the menu bar
  // requires you to click an item to open it again
  if (Widget* target = mTarget.Get<Widget*>())
  {
    FocusEvent event(nullptr, target);
    target->DispatchEvent(Events::FocusLost, &event);
  }
}

void ContextMenu::UpdateTransform()
{
  if (mDirty)
    RebuildUi();

  mBackground->SetColor(MenuUi::BackgroundColor);
  mBorder->SetColor(MenuUi::BorderColor);
  mBackground->SetSize(mSize);
  mBorder->SetSize(mSize);

  mGutter->SetColor(MenuUi::GutterColor);
  mGutter->SetSize(Vec2(Pixels(1), mSize.y - Pixels(3)));
  mGutter->SetTranslation(Vec3(Vec2(MenuUi::BorderPadding).x - Pixels(4), Vec2(MenuUi::BorderPadding).y, 0));

  if (mSubMenu)
  {
    // Position the sub menu
    mSubMenu->UpdateTransform();
    Vec3 subMenuPos = Vec3(mSize.x, 0, 0) + GetScreenPosition();
    mSubMenu->FitSubMenuOnScreen(subMenuPos, mSize);
  }

  PopUp::UpdateTransform();
}

void ContextMenu::SizeToContents()
{
  SetSize(GetMinSize());
  // Check
  Composite::UpdateTransform();
}

Vec2 ContextMenu::GetMinSize()
{
  return Composite::GetMinSize() + Vec2(MenuUi::BorderPadding);
}

void ContextMenu::OnDestroy()
{
  ObjectEvent e(this);
  DispatchEvent(Events::MenuDestroy, &e);

  Composite::OnDestroy();
}

void ContextMenu::RebuildUi()
{
  // Destroy any ContentMenuItems currently on the ContextMenu and rebuild the
  // UI
  forRange (Widget* widget, mItems.All())
  {
    widget->Destroy();
  }
  mItems.Clear();

  // Create the menu UI from our entry list
  forRange (ContextMenuEntry* entry, mRootEntry->mChildren.All())
  {
    entry->Create(this);
  }
  // Resize since new ContextMenuItems were just created
  SizeToContents();

  mDirty = false;
}

uint ContextMenu::IsEmpty()
{
  return mRootEntry->EntryCount() == 0;
}

void ContextMenu::CloseContextMenu()
{
  FadeOut(0.05f);
}

// Similar to shift onto screen, but takes the ContextMenuItem's position and
// the hierarchy's size and position into account to shift the menu to an
// appropriate position if there is not enough space
void ContextMenu::FitSubMenuOnScreen(Vec3 position, Vec2 parentSize)
{
  Vec2 screenSize = this->GetParent()->GetSize();
  Vec2 thisSize = this->GetSize();

  position += mSubMenuOffset;

  if (position.y + thisSize.y > screenSize.y)
    position.y -= (position.y + thisSize.y) - screenSize.y;

  if (position.x + thisSize.x > screenSize.x)
  {
    // Adding 1 pixel shifts the menu so the submenu doesn't overlap the parent
    // menu
    position.x -= (parentSize.x + thisSize.x) + Pixels(1);
    // When a sub menu is placed on the left side of a parent menu
    // the drop shadow overlaps the parents so just make it clear
    mDropShadow->SetColor(Vec4::cZAxis);
  }
  else
  {
    // Shift the sub menu over so that it doesn't overlap with its parents
    position.x -= Pixels(3);
  }

  // This is a sub menu and needs to calculate its local position for placement
  // relative to its parent
  this->SetTranslation(position);
}

void ContextMenu::OnMenuEntriesModified(Event* event)
{
  mDirty = true;
  MarkAsNeedsUpdate();
}

Raverie::ContextMenuEntry* ContextMenu::GetRootEntry()
{
  return mRootEntry;
}

ContextMenuEntry* ContextMenu::AddEntry(StringParam name, bool readOnly)
{
  return mRootEntry->AddEntry(name, readOnly);
}

void ContextMenu::AddDivider()
{
  mRootEntry->AddEntry(new ContextMenuEntryDivider());
}

void ContextMenu::AddContextMenu(StringParam menuName)
{
  mRootEntry->AddEntry(new ContextMenuEntryMenu(menuName));
}

void ContextMenu::AddCommand(Command* command)
{
  mRootEntry->AddEntry(new ContextMenuEntryCommand(command));
}

void ContextMenu::AddCommandByName(StringParam commandName)
{
  mRootEntry->AddEntry(new ContextMenuEntryCommand(commandName));
}

void ContextMenu::ShiftOntoScreen(Vec3 offset)
{
  // If the context menu was just created and hasn't been built yet
  // or if the menu has been altered in some way it needs to be rebuilt
  // so that shift screen operates on the correct dimensions
  if (mDirty)
    UpdateTransform();

  PopUp::ShiftOntoScreen(offset);
}

void ContextMenu::OnMouseDown(MouseEvent* event)
{
  if (!mMoved)
    return;

  // If the mouse down out side the pop up close it
  if (!mSubMenu && !mParentMenu && !this->Contains(event->Position))
    FadeOut();

  // If this menu has a child context menu they need to be taken into account
  // for closing this menu
  if (!IsPositionInHierarchy(event->Position))
    FadeOut();
}

void ContextMenu::OnAnyGained(FocusEvent* event)
{
  // Did focus move outside the popup?
  if (!mSubMenu && !mParentMenu && !this->IsAncestorOf(event->ReceivedFocus))
    FadeOut();

  // Did the focus switch to a child context menu? Then don't fade out.
  if (!IsFocusOnHierarchy(event->ReceivedFocus))
    FadeOut();
}

void ContextMenu::OnFocusOut(FocusEvent* event)
{
  if (!mSubMenu && !mParentMenu)
    FadeOut();
}

bool ContextMenu::IsPositionInHierarchy(Vec2Param screenPosition)
{
  return GetRootContextMenu()->IsPositionInSubMenuRecursive(screenPosition);
}

bool ContextMenu::IsFocusOnHierarchy(Widget* focusObject)
{
  return GetRootContextMenu()->IsFocusInSubMenuRecursive(focusObject);
}

ContextMenu* ContextMenu::GetRootContextMenu()
{
  ContextMenu* rootMenu = this;
  while (rootMenu->mParentMenu)
    rootMenu = rootMenu->mParentMenu;

  return rootMenu;
}

bool ContextMenu::IsPositionInSubMenuRecursive(Vec2Param screenPosition)
{
  if (this->Contains(screenPosition))
    return true;

  if (mSubMenu && mSubMenu->IsPositionInSubMenuRecursive(screenPosition))
    return true;

  return false;
}

bool ContextMenu::IsFocusInSubMenuRecursive(Widget* focusObject)
{
  if (this->IsAncestorOf(focusObject))
    return true;

  if (mSubMenu && mSubMenu->IsFocusInSubMenuRecursive(focusObject))
    return true;

  return false;
}

RaverieDefineType(MenuBarItem, builder, type)
{
}

MenuBarItem::MenuBarItem(Composite* parent) : Composite(parent), mContextMenu(nullptr)
{
  mMenuBar = (MenuBar*)parent;
  mBackground = CreateAttached<Element>(cHighlight);
  mBackground->SetVisible(false);
  mText = new Text(this, cText);
  ConnectThisTo(this, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::FocusLost, ClearOpenMenu);
}

void MenuBarItem::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mText->SetSize(mSize);
  WidgetRect rect = RemoveThicknessRect(Thickness(MenuUi::MenuBarItemPadding), mSize);
  PlaceWithRect(rect, mText);
  Composite::UpdateTransform();
}

Vec2 MenuBarItem::GetMinSize()
{
  return ExpandSizeByThickness(Thickness(MenuUi::MenuBarItemPadding), mText->GetMinSize());
}

void MenuBarItem::OnLeftMouseDown(MouseEvent* mouseEvent)
{
  if (GetMenuBar()->GetOpenMenuBarItem())
    CloseContextMenu();
  else
    OpenContextMenu();
}

void MenuBarItem::OnMouseEnter(MouseEvent* mouseEvent)
{
  MenuBarItem* openMenuBarItem = GetMenuBar()->GetOpenMenuBarItem();
  if (openMenuBarItem && openMenuBarItem != this)
  {
    openMenuBarItem->CloseContextMenu();
    OpenContextMenu();
  }
}

void MenuBarItem::OpenContextMenu()
{
  ContextMenu* contextMenu = new ContextMenu(this);
  contextMenu->SetTranslation(this->GetScreenPosition() + Pixels(0, mSize.y, 0));
  contextMenu->AddContextMenu(mName);
  contextMenu->SizeToContents();
  mContextMenu = contextMenu;
  GetMenuBar()->mOpenMenuBarItem = this;
}

MenuBar* MenuBarItem::GetMenuBar()
{
  return mMenuBar;
}

void MenuBarItem::CloseContextMenu()
{
  // check if we have an open context menu attached our menu bar item
  mContextMenu->CloseContextMenu();
}

void MenuBarItem::ClearOpenMenu(FocusEvent* event)
{
  // if this is the open menu bar we need to clear the menu bars reference to us
  // this check handles both closing an open menu to update to a new open menu
  // and when focus is lost on the context menu and it deletes itself
  MenuBar* menuBar = GetMenuBar();
  if (!menuBar)
    return;

  MenuBarItem* openMenuBar = menuBar->mOpenMenuBarItem;
  if ((MenuBarItem*)GetMenuBar()->mOpenMenuBarItem == this)
  {
    // null out handles for closed items
    mContextMenu = nullptr;
    GetMenuBar()->mOpenMenuBarItem = nullptr;
  }
}

RaverieDefineType(MenuBar, builder, type)
{
}

MenuBar::MenuBar(Composite* parent) : Composite(parent), mOpenMenuBarItem(nullptr)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Vec2(9.0f, 0), Thickness(0, 0)));
}

void MenuBar::LoadMenu(StringParam menuName)
{
  CommandManager* commandManager = CommandManager::GetInstance();
  MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, nullptr);
  ReturnIf(menuDef == nullptr, , "Could not find menu definition '%s'", menuName.c_str());

  forRange (String& menuName, menuDef->Entries.All())
  {
    MenuDefinition* menuDef = commandManager->mMenus.FindValue(menuName, nullptr);
    ErrorIf(menuDef == nullptr, "Could not find menu definition '%s'", menuName.c_str());
    if (menuDef)
    {
      MenuBarItem* entry = new MenuBarItem(this);
      entry->mText->SetText(menuName);
      entry->mName = menuName;
    }
  }
}

MenuBarItem* MenuBar::GetOpenMenuBarItem()
{
  return mOpenMenuBarItem;
}

} // namespace Raverie
